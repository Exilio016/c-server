/* bfutils_process.h

DESCRIPTION: 

    This is a single-header-file library that provides utility funtions to work with process.

USAGE:
    
    In one source file put:
        #define BFUTILS_PROCESS_IMPLEMENTATION
        #include "bfutils_process.h"
    
    Other source files should contain only the import line.

    Functions (macros):
    
        process_sync:
        int process_sync(char *const *cmd, char *in, char **out, char **err); Starts and waits for the process execution.
            "cmd" needs to be a null-terminated array containing the process and its arguments.
            if "in" is not NULL, its content will be send to the process STDIN. 
            if "out", or "err" is not NULL, a null-terminated string will be placed at *out or *err with the contents of STDOUT and STDERR respectively.
            The caller needs to free *out and *err.
            The return value is the process exit status.
        
        process_async:
        Process process_async(char *const *cmd); Starts a new process and return imediatelly.
            "cmd" needs to be a null-terminated array containing the process and its arguments.
            It returns a handle to the process.
            The caller needs to call process_close to close all opened file descriptors.

        process_write_stdin:
        void process_write_stdin(Process *p, const char *in); It writes the contents of in to the process stdin.

        process_read_stdout:
        char *process_read_stdout(Process *p); It returns the contents of the process stdout as a null-terminated string.
            The caller needs to free the returned string.

        process_read_stderr:
        char *process_read_stderr(Process *p); It returns the contents of the process stderr as a null-terminated string.
            The caller needs to free the returned string.

        process_wait:
        int process_wait(Process *p); It waits for the end of the process execution and returns its exit status.
        
        process_is_running:
        int process_is_running(Process *p, int *status); It returns a non-zero value if process is running.
            If an error occurs on the waitpid internal call this function returns -1.
            If "status" is not NULL and this function returned 0 (process has exited), *status will be populated with the process exit status.

        process_close:
        process_close(Process *p); It closes all opened file descriptors.
    
    Compile-time options:
        
        #define BFUTILS_PROCESS_NO_SHORT_NAME
        
            This flag needs to be set globally.
            By default this file exposes functions without bfutils_ prefix.
            By defining this flag, this library will expose only functions prefixed with bfutils_

        #define BFUTILS_PROCESS_MALLOC another_malloc
        #define BFUTILS_PROCESS_CALLOC another_calloc
        #define BFUTILS_PROCESS_REALLOC another_realloc
        #define BFUTILS_PROCESS_FREE another_free

            These flags needs to be set only in the file containing #define BFUTILS_PROCESS_IMPLEMENTATION
            If you don't want to use 'stdlib.h' memory functions you can define these flags with custom functions.

LICENSE:

    MIT License
    
    Copyright (c) 2024 Bruno Flávio Ferreira 
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#ifndef BFUTILS_PROCESS_H
#define BFUTILS_PROCESS_H

#include <sys/types.h>

typedef struct {
    pid_t pid;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
} BFUtilsProcess;

#ifndef BFUTILS_PROCESS_NO_SHORT_NAME

#define process_sync bfutils_process_sync
#define process_async bfutils_process_async
#define process_write_stdin bfutils_process_write_stdin
#define process_read_stdout bfutils_process_read_stdout
#define process_read_stderr bfutils_process_read_stderr
#define process_wait bfutils_process_wait
#define process_is_running bfutils_process_is_running
#define process_close bfutils_process_close

typedef BFUtilsProcess Process;

#endif //BFUTILS_PROCESS_NO_SHORT_NAME

#if ((defined(BFUTILS_PROCESS_REALLOC) && (!defined(BFUTILS_PROCESS_MALLOC) || !defined(BFUTILS_PROCESS_CALLOC) || !defined(BFUTILS_PROCESS_FREE))) \
    || (defined(BFUTILS_PROCESS_MALLOC) && (!defined(BFUTILS_PROCESS_REALLOC) || !defined(BFUTILS_PROCESS_CALLOC) || !defined(BFUTILS_PROCESS_FREE))) \
    || (defined(BFUTILS_PROCESS_CALLOC) && (!defined(BFUTILS_PROCESS_MALLOC) || !defined(BFUTILS_PROCESS_REALLOC) || !defined(BFUTILS_PROCESS_FREE))) \
    || (defined(BFUTILS_PROCESS_FREE) && (!defined(BFUTILS_PROCESS_MALLOC) || !defined(BFUTILS_PROCESS_REALLOC) || !defined(BFUTILS_PROCESS_CALLOC))))
#error "You must define all BFUTILS_PROCESS_REALLOC, BFUTILS_PROCESS_CALLOC, BFUTILS_PROCESS_MALLOC, BFUTILS_PROCESS_FREE or neither."
#endif

#ifndef BFUTILS_PROCESS_REALLOC
#include <stdlib.h>
#define BFUTILS_PROCESS_REALLOC realloc
#define BFUTILS_PROCESS_CALLOC calloc
#define BFUTILS_PROCESS_MALLOC malloc
#define BFUTILS_PROCESS_FREE free
#endif //BFUTILS_PROCESS_REALLOC

extern int bfutils_process_sync(char *const *cmd, const char *in, char **out, char **err);
extern BFUtilsProcess bfutils_process_async(char *const *cmd);
extern void bfutils_process_write_stdin(BFUtilsProcess *p, const char *in);
extern char *bfutils_process_read_stdout(BFUtilsProcess *p);
extern char *bfutils_process_read_stderr(BFUtilsProcess *p);
extern int bfutils_process_wait(BFUtilsProcess *p);
extern int bfutils_process_is_running(BFUtilsProcess *p, int *status);
extern void bfutils_process_close(BFUtilsProcess *p);

#endif // PROCESS_H
#ifdef BFUTILS_PROCESS_IMPLEMENTATION
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

void read_fd(int fd, char **res) {
    char buffer[1024];
    int count = 0;
    int length; 
    *res = NULL;
    do {
        length = read(fd, buffer, 1024);
        if (length < 0){
            length = 0;
        }
        *res = (char*) BFUTILS_PROCESS_REALLOC(*res, count + length + 1);
        if (length > 0)
            strncpy(*res + count, buffer, length);
        count += length;
    } while(length > 0);
    (*res)[count] = '\0';
}

void close_pair(int *fd) {
    close(fd[0]);
    close(fd[1]);
}

int set_fds_nonblock(int *stdout_fd, int *stderr_fd){
    if (fcntl(stdout_fd[0], F_SETFL, fcntl(stdout_fd[0], F_GETFL) | O_NONBLOCK) == -1){
        return 0;
    }
    if (fcntl(stdout_fd[1], F_SETFL, fcntl(stdout_fd[1], F_GETFL) | O_NONBLOCK) == -1){
        return 0;
    }
    if (fcntl(stderr_fd[0], F_SETFL, fcntl(stderr_fd[0], F_GETFL) | O_NONBLOCK) == -1){
        return 0;
    }
    if (fcntl(stderr_fd[1], F_SETFL, fcntl(stderr_fd[1], F_GETFL) | O_NONBLOCK) == -1){
        return 0;
    }
    return 1;
}

BFUtilsProcess bfutils_process_async(char *const *cmd) {
    BFUtilsProcess process = {.pid = -1, .stdin_fd = -1, .stdout_fd = -1, .stderr_fd = -1};
    if(cmd == NULL || *cmd == NULL) {
        return process;
    }
    int stdin_fd[2];
    int stdout_fd[2];
    int stderr_fd[2];

    if (pipe(stdin_fd) < 0) {
        return process;
    }
    if (pipe(stdout_fd) < 0) {
        close_pair(stdin_fd);
        return process;
    }
    if (pipe(stderr_fd) < 0) {
        close_pair(stdin_fd);
        close_pair(stdout_fd);
        return process;
    }

    if (!set_fds_nonblock(stdout_fd, stderr_fd)){
        close_pair(stdin_fd);
        close_pair(stdout_fd);
        close_pair(stderr_fd);
        return process;
    }

    pid_t pid;
    pid = fork();
    switch (pid) {
        case -1:
            return process;
        case 0:
            if (dup2(stdin_fd[0], STDIN_FILENO) < 0) {
                perror("dup2");
                exit(1);
            }
            close_pair(stdin_fd);

            if (dup2(stdout_fd[1], STDOUT_FILENO) < 0) {
                perror("dup2");
                exit(1);
            }
            close_pair(stdout_fd);

            if (dup2(stderr_fd[1], STDERR_FILENO) < 0) {
                perror("dup2");
                exit(1);
            }
            close_pair(stderr_fd);

            execvp(cmd[0], cmd);
            break;
        default:
            process.pid = pid;
            process.stdin_fd = stdin_fd[1];
            process.stdout_fd = stdout_fd[0];
            process.stderr_fd = stderr_fd[0];
            close(stdin_fd[0]);
            close(stdout_fd[1]);
            close(stderr_fd[1]);
    }
    return process;
}

int bfutils_process_sync(char *const *cmd, const char *in, char **out, char **err) {
    BFUtilsProcess process = bfutils_process_async(cmd);
    if (process.pid < 0) {
        return -1;
    }

    bfutils_process_write_stdin(&process, in);
    int status = bfutils_process_wait(&process);
    
    if (out != NULL) {
        *out = bfutils_process_read_stdout(&process);
    }

    if (err != NULL) {
        *err = bfutils_process_read_stderr(&process);
    }
    bfutils_process_close(&process);

    return status;
}

void bfutils_process_write_stdin(BFUtilsProcess *p, const char *in) {
    if (in != NULL) {
        int wrote = 0;
        int len = strlen(in);
        do {
            wrote += write(p->stdin_fd, in, len);
        } while (wrote < len);
    }
}

char *bfutils_process_read_stdout(BFUtilsProcess *p) {
    char *out;
    read_fd(p->stdout_fd, &out);
    return out;
}

char *bfutils_process_read_stderr(BFUtilsProcess *p) {
    char *out;
    read_fd(p->stderr_fd, &out);
    return out;
}

int bfutils_process_wait(BFUtilsProcess *p) {
    int status;
    close(p->stdin_fd);
    int wpid = waitpid(p->pid, &status, 0);
    if (wpid < 0) {
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status)) {
        return WTERMSIG(status);
    }
    else if (WIFSTOPPED(status)) {
        return WSTOPSIG(status);
    }
    return status;
}

int bfutils_process_is_running(BFUtilsProcess *p, int *s) {
    int status;
    close(p->stdin_fd);
    int wpid = waitpid(p->pid, &status, WNOHANG);
    if (wpid < 0) {
        return -1;
    }
    if (wpid != 0 && s != NULL) {
        if (WIFEXITED(status)) {
            *s = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
            *s = WTERMSIG(status);
        }
        else if (WIFSTOPPED(status)) {
            *s = WSTOPSIG(status);
        }
    }
    return wpid == 0;
}

void bfutils_process_close(BFUtilsProcess *p) {
    close(p->stdin_fd);
    close(p->stdout_fd);
    close(p->stderr_fd);
}
#endif //BFUTILS_PROCESS_IMPLEMENTATION
