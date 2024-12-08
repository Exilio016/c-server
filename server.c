#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <limits.h>
#include <signal.h>
#include <ucontext.h>
#include <poll.h>
#define BFUTILS_VECTOR_IMPLEMENTATION
#include "bfutils_vector.h"
#define BFUTILS_HASHMAP_IMPLEMENTATION
#include "bfutils_hash.h"
#define BFUTILS_PROCESS_IMPLEMENTATION
#include "bfutils_process.h"

#define defer_return(r) { ret = (r); goto defer; }

typedef struct {
    char *key;
    char *value;
} HttpHeader;

typedef struct {
    char *protocol;
    char *path;
    HttpHeader *headers;
    char *body;
} HttpReq;

typedef struct {
    int status_code;
    HttpHeader *headers;
    char *body;
} HttpRes;

void http_header_free(void *obj) {
    HttpHeader *header = (HttpHeader*) obj;
    vector_free(header->key);
    vector_free(header->value);
}

HttpReq parse_http_request(const char *request) {
    HttpReq req = {0};
    req.headers = hashmap(http_header_free);
    char **lines = string_split(request, "\r\n");
    if(vector_length(lines) > 0) {
        char **status_line = string_split(lines[0], " ");
        if (vector_length(status_line) > 2) {
            req.protocol = status_line[0];
            req.path = status_line[1];
            for (int i = 2; i < vector_length(status_line); i++) {
                vector_free(status_line[i]);
            }
            vector_free(status_line);
            vector_free(lines[0]);

            int is_body = 0;
            char *body = NULL;
            for (int i = 1; i < vector_length(lines); i++) {
                if (0 == strcmp(lines[i], "")) {
                    is_body = 1;
                    vector_free(lines[i]);
                    continue;
                }
                if (!is_body) {
                    char *saveptr = NULL;
                    char *key = NULL;
                    char *value = NULL;
                    char *v = strtok_r(lines[i], ": ", &saveptr);
                    string_push_cstr(key, v);
                    v = strtok_r(NULL, "", &saveptr);
                    string_push_cstr(value, v);
                    string_hashmap_push(req.headers, key, value);
                }
                else {
                    string_push(body, lines[i]);
                }
                vector_free(lines[i]);
            }
            req.body = body;
        }
    }
    vector_free(lines);
    return req;
}

void print_http_request(HttpReq *req) {
    printf("Protocol: %s\nPath: %s\nHeaders:\n", req->protocol, req->path);
    HashmapIterator it = hashmap_iterator(req->headers);
    while(hashmap_iterator_has_next(&it)) {
        HttpHeader header = hashmap_iterator_next(req->headers, &it);
        printf("\t%s:%s\n", header.key, header.value);
    }
    printf("Body:\n%s\n", req->body); 
}

char *http_response_to_bytes(HttpRes *res) {
    char *response = NULL;
    char *status_line = string_format("HTTP/1.1 %d OK\r\n", res->status_code);
    string_push(response, status_line);
    vector_free(status_line);

    char *content_length = string_format("%d", vector_length(res->body));
    string_hashmap_push(res->headers, string_format("Content-Length"), content_length);

    HashmapIterator it = hashmap_iterator(res->headers);
    while(hashmap_iterator_has_next(&it)) {
        HttpHeader header = hashmap_iterator_next(res->headers, &it);
        string_push(response, header.key);
        string_push_cstr(response, ": ");
        string_push(response, header.value);
        string_push_cstr(response, "\r\n");
    }
    string_push_cstr(response, "\r\n");
    string_push(response, res->body);
    return response;
}

char *read_entire_file(const char *path){
    struct stat file_stat;
    if (stat(path, &file_stat) < 0) {
        return NULL;
    }
    size_t size = file_stat.st_size;
    int is_file = file_stat.st_mode & (S_IFREG | S_IFLNK);

    if (!is_file) {
        return NULL;
    }

    FILE *fp = fopen(path, "r");
    char *bytes = NULL;

    if (fp == NULL) {
        return NULL;
    };

    vector_ensure_capacity(bytes, size);

    long read = 0;
    do {
        int l = fread(bytes, sizeof(char), size, fp);
        if (l < 0) break;
        read += l;
    } while(read < size);
    vector_header(bytes)->length = read;
    fclose(fp);
    return bytes;
}

char *not_found_body(const char *path) {
    char *body = string_format("<html><head><title>Page not found</title></head><body><h1>Page not found</h1><p>Page %s not found</p></body></html>", path);
    return body;
}

char *get_file_mime_type(const char *path) {
    char *out;
    char absolute_path[PATH_MAX];
    path = realpath(path, absolute_path);
    int status = process_sync((char *[]) {"file", "-i", (char*) path, NULL}, NULL, &out, NULL);

    char *mime = NULL;
    for (int i = 0; i < strlen(out); i++) {
        if (out[i] == ':'){
            out[strlen(out) - 1] = '\0';
            string_push_cstr(mime, out + i + 2);
            break;
        }
    }
    free(out);
    return mime;
}

HttpRes handle_request(HttpReq *req, char *folder) {
    HttpRes res = {.status_code = 200};
    res.headers = hashmap(http_header_free);
    char *path = NULL;
    char *body = NULL;
    string_push_cstr(path, folder);
    if (0 == strcmp(req->path, "/")) {
        string_push_cstr(path, "/index.html");
        body = read_entire_file(path);
    }
    else {
        string_push(path, req->path);
        body = read_entire_file(path);
    }
    if (body == NULL) {
        res.status_code = 404;
        res.body = not_found_body(req->path);
    }
    else {
        string_hashmap_push(res.headers, string_format("Content-Type"), get_file_mime_type(path));
        res.body = body;
    }
    string_hashmap_push(res.headers, string_format("Connection"), string_format("close"));
    vector_free(path);
    return res;
}

void http_request_free(HttpReq *req) {
    vector_free(req->protocol);
    vector_free(req->body);
    vector_free(req->path);
    hashmap_free(req->headers);
}

void http_response_free(HttpRes *res) {
    vector_free(res->body);
    hashmap_free(res->headers);
}


static int sock;
void sighandler(int signal) {
    shutdown(sock, SHUT_RDWR);
    printf("Exiting the program...\n");
}

void close_safe(int fd) {
    shutdown(fd, SHUT_WR);
    char buf[1024];
    int r;
    do {
        r = read(fd, buf, 1024);
    } while(r > 0);
    close(fd);
}

int main (int argc, char *argv[]) {
    int ret = 0;
    struct option *options = NULL;
    struct option opt =  {.name = "port", .val = 'p', .flag = NULL, .has_arg = 1};
    vector_push(options, opt);
    opt = (struct option) {.name = "help", .val = 'h', .flag = NULL, .has_arg = 0 };
    vector_push(options, opt);
    opt = (struct option) {.name = "files", .val = 'f', .flag = NULL, .has_arg = 1 };
    vector_push(options, opt);
    opt = (struct option) {0};
    vector_push(options, opt);

    struct sigaction act = {.sa_handler = sighandler, .sa_flags = SA_RESTART | SA_NOCLDSTOP};
    sigaction(SIGINT, &act, NULL);

    long port = 8080;
    char *files = NULL;
    char *end = NULL;
    char o;
    while ((o = getopt_long(argc, argv, "hp:f:", options, NULL)) > 0) {
        switch (o) {
            case 'p':
                port = strtol(argv[optind - 1], &end, 10);
                if (port <= 0 || port > SHRT_MAX || *end != '\0') {
                    fprintf(stderr, "Invalid port: %s\n", argv[optind - 1]);
                    fprintf(stderr, "Usage: %s [-h] [-p PORT] -f PATH\n", argv[0]);
                    defer_return(1);
                }
                break;
            case 'f':
                files = argv[optind - 1];
                break;
            case 'h':
                printf("Usage: %s [-h] [-p PORT] -f PATH\n", argv[0]);
                printf("Options:\n");
                printf("\t-h\t--help      \tShow this help menu\n");
                printf("\t-p\t--port=PORT \tSpecify the port to be used. Defaults to 8080\n");
                printf("\t-f\t--files=PATH\tSpecify the folder containing the static files to be exposed by the server\n");
                break;
            default:
                fprintf(stderr, "Usage: %s [-h] [-p PORT] -f PATH\n", argv[0]);
                defer_return(1);
        }
    }
    if (files == NULL) {
        fprintf(stderr, "Usage: %s [-h] [-p PORT] -f PATH\n", argv[0]);
        defer_return(1);
    }

    sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons((short) port), .sin_addr = {.s_addr = htonl(INADDR_ANY)}};
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
        
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        defer_return(1);
    }
    if (listen(sock, 100) < 0) {
        perror("listen");
        defer_return(1);
    }
    printf("Listening to port %d\n", (int) port);

    int fd;
    while ((fd = accept(sock, (struct sockaddr*) &client_addr, &client_addr_len)) > 0) {
        char *msg = NULL;
        int l;
        struct pollfd fds = {.fd = fd, .events = POLLIN | POLLHUP };
        do {
            char buffer[1024] = {0};
            l = recv(fd, buffer, 1024, 0);
            string_push_cstr(msg, buffer);
            if (poll(&fds, 1, 100) < 1) {
                break;
            }
        } while(1);
        HttpReq req = parse_http_request(msg);
        HttpRes res = handle_request(&req, files);
        char *res_bytes = http_response_to_bytes(&res);
        send(fd, res_bytes, vector_length(res_bytes), 0);
        
        http_request_free(&req);
        http_response_free(&res);
        vector_free(res_bytes);
        vector_free(msg);
        close_safe(fd);
    }

defer:
    if (sock > 0) {
        if (close(sock) < 0) {
            perror("close");
        }
    }
    vector_free(options);
    return ret;
}
