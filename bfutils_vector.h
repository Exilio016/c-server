/* bfutils_vector.h

DESCRIPTION: 

    This is a single-header-file library that provides dynamic-arrays (vector) for C.

USAGE:
    
    In one source file put:
        #define BFUTILS_VECTOR_IMPLEMENTATION
        #include "bfutils_vector.h"
    
    Other source files should contain only the import line.

    Functions (macros):

        vector_header: 
            BFUtilsVectorHeader *vector_header(T*); Returns the header object.

        vector_capacity:
            size_t vector_capacity(T*); Returns the vector capacity.

        vector_length:
            size_t vector_length(T*); Returns the vector length.

        vector_ensure_capacity:
            void vector_ensure_capacity(T*, size_t); If current capacity is less that provided, grows the vector to provided capacity.

        vector_push:
            void vector_push(T*, T); Insert element to end of the vector. Grows the vector if required.

        vector_pop:
            T vector_pop(T*); Removes and return the last element in the vector.

        vector_free:
            void vector_free(T*); Frees the vector.

        string_push:
            void string_push(char *, const char*); Appends to the end of a char* vector another char* vector.
            It inserts a NULL byte at the end without incrementing the length.

        string_push_cstr:
            void string_push_cstr(char *, const char*); Appends to the end of a char* vector a null terminated string. 
            It inserts a NULL byte at the end without incrementing the length.

        string_split:
            char **string_split(const char *, const char*); Splits a NULL terminated string by a delimiter. Returns a vector of NULL terminated strings.
            The returned vector needs to be free by calling vector_free on each element and itself.

        string_format:
            char *string_format(const char*, ...); Returns the formatted string. It needs to be free by calling vector_free. 
    
    Compile-time options:
        
        #define BFUTILS_VECTOR_NO_SHORT_NAME
        
            This flag needs to be set globally.
            By default this file exposes functions without bfutils_ prefix.
            By defining this flag, this library will expose only functions prefixed with bfutils_

        #define BFUTILS_REALLOC another_realloc
        #define BFUTILS_FREE another_free

            These flags needs to be set only in the file containing #define BFUTILS_VECTOR_IMPLEMENTATION
            If you don't want to use 'stdlib.h' realloc and free function you can define this flag with a custom function.

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

#ifndef BFUTILS_VECTOR_H
#define BFUTILS_VECTOR_H

#ifndef BFUTILS_VECTOR_NO_SHORT_NAME

#define vector_header bfutils_vector_header
#define vector_capacity bfutils_vector_capacity
#define vector_length bfutils_vector_length
#define vector_ensure_capacity bfutils_vector_ensure_capacity
#define vector_push bfutils_vector_push
#define vector_pop bfutils_vector_pop
#define vector_free bfutils_vector_free
#define string_push bfutils_string_push_str
#define string_push_cstr bfutils_string_push_cstr
#define string_split bfutils_string_split
#define string_format bfutils_string_format

#endif //BFUTILS_VECTOR_NO_SHORT_NAME

#if (!defined(BFUTILS_REALLOC) && defined(BFUTILS_FREE)) || (defined(BFUTILS_REALLOC) && !defined(BFUTILS_FREE))
#error "You must define both BFUTILS_REALLOC and BFUTILS_FREE or neither."
#endif

#ifndef BFUTILS_REALLOC
#include <stdlib.h>
#define BFUTILS_REALLOC realloc
#define BFUTILS_FREE free
#endif //BFUTILS_REALLOC

#include <stddef.h>

typedef struct {
    size_t length;
    size_t capacity;
} BFUtilsVectorHeader;

#define bfutils_vector_header(v) ((v) ? (BFUtilsVectorHeader *) (v) - 1 : NULL)
#define bfutils_vector_capacity(v) ((v) ? bfutils_vector_header((v))->capacity : 0)
#define bfutils_vector_length(v) ((v) ? bfutils_vector_header((v))->length : 0)
#define bfutils_vector_new_capacity(v) ((v) ? (bfutils_vector_capacity((v)) * 1.5) : 128)
#define bfutils_vector_push(v, e) ((v) = bfutils_vector_grow((v), sizeof(*(v)), bfutils_vector_length((v)) + 1),\
    (v)[bfutils_vector_header((v))->length++] = e)
#define bfutils_vector_pop(v) ((v)[--bfutils_vector_header((v))->length])
#define bfutils_vector_free(v) (BFUTILS_FREE(bfutils_vector_header((v))), (v) = NULL)
#define bfutils_vector_ensure_capacity(v, c) ((v) = bfutils_vector_capacity_grow((v), sizeof(*(v)), (c)))
#define bfutils_string_push_cstr(s, a) ((s) = bfutils_string_push_cstr_f((s), (a)))
#define bfutils_string_push_str(s, a) ((s) = bfutils_string_push_str_f((s), (a)))

extern void *bfutils_vector_grow(void *vector, size_t element_size, size_t length);
extern void *bfutils_vector_capacity_grow(void *vector, size_t element_size, size_t capacity);
extern char* bfutils_string_push_cstr_f(char *str, const char *cstr);
extern char* bfutils_string_push_str_f(char *str, const char *s);
extern char** bfutils_string_split(const char *cstr, const char *delim);
extern char* bfutils_string_format(const char *format, ...);

#endif // VECTOR_H
#ifdef BFUTILS_VECTOR_IMPLEMENTATION
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void *bfutils_vector_grow(void *vector, size_t element_size, size_t length) {
    if (bfutils_vector_capacity(vector) < length) {
        size_t length = bfutils_vector_length(vector);
        size_t capacity = bfutils_vector_new_capacity(vector);
        BFUtilsVectorHeader *header = BFUTILS_REALLOC(bfutils_vector_header(vector), sizeof(BFUtilsVectorHeader) + (element_size * capacity));
        header->capacity = capacity;
        header->length = length;
        vector = (void*)(header + 1);
    }
    return vector;
}

void *bfutils_vector_capacity_grow(void *vector, size_t element_size, size_t capacity) {
    if (bfutils_vector_capacity(vector) < capacity) {
        size_t length = bfutils_vector_length(vector);
        BFUtilsVectorHeader *header = BFUTILS_REALLOC(bfutils_vector_header(vector), sizeof(BFUtilsVectorHeader) + (element_size * capacity));
        header->capacity = capacity;
        header->length = length;
        vector = (void*)(header + 1);
    }
    return vector;
}

char *bfutils_string_push_cstr_f(char *str, const char *cstr) {
    if (cstr == NULL) 
        return str;
    bfutils_vector_ensure_capacity(str, vector_length(str) + strlen(cstr) + 1);
    for (int i = 0; i < strlen(cstr); i++) {
        bfutils_vector_push(str, cstr[i]);
    }
    str[bfutils_vector_length(str)] = '\0'; //Inserts \0 without incrementing length
    return str;
}

char *bfutils_string_push_str_f(char *str, const char *s) {
    bfutils_vector_ensure_capacity(str, vector_length(str) + vector_length(s) + 1);
    for (int i = 0; i < bfutils_vector_length(s); i++) {
        bfutils_vector_push(str, s[i]);
    }
    str[bfutils_vector_length(str)] = '\0'; //Inserts \0 without incrementing length
    return str;
}

char **bfutils_string_split(const char *cstr, const char *delim) {
    char **list = NULL;
    char *saveptr = NULL;
    
    char *new_str = NULL;
    bfutils_string_push_cstr(new_str, cstr);

    char *res = strtok_r(new_str, delim, &saveptr);
    while (res != NULL) {
        char *t = NULL;
        bfutils_string_push_cstr(t, res);
        vector_push(list, t);
        res = strtok_r(NULL, delim, &saveptr);
    }

    vector_free(new_str);
    return list;
}

char* bfutils_string_format(const char *format, ...) {
    char *res = NULL;
    va_list list;
    va_start(list, format);
    int l = vsnprintf(res, 0, format, list);
    vector_ensure_capacity(res, l + 1);
    va_end(list);

    va_start(list, format);
    vsnprintf(res, l+1, format, list);
    bfutils_vector_header(res)->length = l;
    va_end(list);
    return res;
}
#endif //BFUTILS_VECTOR_IMPLEMENTATION
