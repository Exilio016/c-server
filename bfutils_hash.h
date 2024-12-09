/* bfutils_hash.h

DESCRIPTION: 

    This is a single-header-file library that provides a hashmap for C.

USAGE:
    
    In one source file put:
        #define BFUTILS_HASHMAP_IMPLEMENTATION
        #include "bfutils_hash.h"
    
    Other source files should contain only the import line.

    To use bellow functions you need to have a type T containing an a TK key and a TV value.
    Then just declare: T *hashmap = NULL

    If the key is a string, use the string_hashmap functions.

    Functions (macros):

        hashmap:
            T *hashmap(void (*)(void*)); Initializes a hashmap.
            This function needs to be used only when the key or value needs to be free.
            Otherwise you can simply initialize a hashmap with NULL.
            The function passed will be called for each element inserted in the hashmap when hashmap_free is called, and it will receive a pointer to the element.

        hashmap_header:
            BFUtilsHashmapHeader *hashmap_header(T*); Return a pointer to the hashmap header.

        hashmap_push:
            void hashmap_push(T*, TK, TV); Inserts an element to the hashmap.

        hashmap_get:
            TV hashmap_get(T*, TK); Returns an element value from the hashmap.

        hashmap_remove:
            TV hashmap_remove(T*, TK); Removes and returns an element from the hashmap.

        hashmap_contains:
            int hashmap_contains(T*, TK); Returns a non-zero value if the hashmap contains the TK key.

        string_hashmap_push:
            void string_hashmap_push(T*, const char*, TV); Inserts an element to the hashmap.
            If the key already exists in the hashmap, its value will be replaced.
            And if the hashmap was initialized with hashmap function, the element_free function will be called for the replaced element.

        string_hashmap_get: 
            TV string_hashmap_get(T*, const char *); Returns an element value from the hashmap.

        string_hashmap_remove: 
            TV string_hashmap_remove(T*, const char*); Removes and returns an element from the hashmap.

        string_hashmap_contains:
            int hashmap_contains(T*, const char*); Returns a non-zero value if the hashmap contains the char* key.

        hashmap_free:
            void hashmap_free(T*); Frees the hashmap.
            If the hashmap was initialized with hashmap funtion. The element_free function provided during initialization will be called for each element.
        
        hashmap_iterator:
            HashmapIterator hashmap_iterator(T*); Returns an iterator for the hashmap.

        hashmap_iterator_reverse:
            HashmapIterator hashmap_iterator_reverse(T*); Returns an iterator for the hashmap in reverse order.
        
        hashmap_iterator_has_next:
            int hashmap_iterator_has_next(HashmapIterator *); Returns a non-zero value if a next position exits in the hashmap.
        
        hashmap_iterator_has_previous:
            int hashmap_iterator_has_previous(HashmapIterator *); Returns a non-zero value if a previous position exits in the hashmap.
        
        hashmap_iterator_next:
            T hashmap_iterator_next(T*, HashmapIterator*); Returns the next position on the hashmap, it modifies the iterator.

        hashmap_iterator_previous:
            T hashmap_iterator_previous(T*, HashmapIterator*); Returns the previous position on the hashmap, it modifies the iterator.

    Compile-time options:
        
        #define BFUTILS_HASHMAP_NO_SHORT_NAME
        
            This flag needs to be set globally.
            By default this file exposes functions without bfutils_ prefix.
            By defining this flag, this library will expose only functions prefixed with bfutils_

        #define BFUTILS_HASHMAP_MALLOC another_malloc
        #define BFUTILS_HASHMAP_CALLOC another_calloc
        #define BFUTILS_HASHMAP_REALLOC another_realloc
        #define BFUTILS_HASHMAP_FREE another_free

            These flags needs to be set only in the file containing #define BFUTILS_HASHMAP_IMPLEMENTATION
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

#ifndef BFUTILS_HASHMAP_H
#define BFUTILS_HASHMAP_H

#include <stddef.h>

typedef struct {
    size_t insert_count;
    size_t length;
    unsigned char *slots;
    unsigned char *removed;
    void (*element_free)(void*);
} BFUtilsHashmapHeader;

typedef struct {
    BFUtilsHashmapHeader *h;
    size_t current;
    size_t first;
    size_t last;
    int started;
} BFUtilsHashmapIterator;


#ifndef BFUTILS_HASHMAP_NO_SHORT_NAME

#define hashmap_header bfutils_hashmap_header
#define hashmap_push bfutils_hashmap_push
#define hashmap_get bfutils_hashmap_get
#define hashmap_remove bfutils_hashmap_remove
#define hashmap_contains bfutils_hashmap_contains
#define string_hashmap_push bfutils_string_hashmap_push
#define string_hashmap_get bfutils_string_hashmap_get
#define string_hashmap_remove bfutils_string_hashmap_remove
#define string_hashmap_contains bfutils_string_hashmap_contains
#define hashmap_free bfutils_hashmap_free
#define hashmap_iterator bfutils_hashmap_iterator
#define hashmap_iterator_reverse bfutils_hashmap_iterator_reverse
#define hashmap_iterator_next bfutils_hashmap_iterator_next
#define hashmap_iterator_previous bfutils_hashmap_iterator_previous
#define hashmap_iterator_has_next bfutils_hashmap_iterator_has_next
#define hashmap_iterator_has_previous bfutils_hashmap_iterator_has_previous
#define hashmap bfutils_hashmap

typedef BFUtilsHashmapHeader HashmapHeader; 
typedef BFUtilsHashmapIterator HashmapIterator; 

#endif //BFUTILS_HASHMAP_NO_SHORT_NAME

#if ((defined(BFUTILS_HASHMAP_REALLOC) && (!defined(BFUTILS_HASHMAP_MALLOC) || !defined(BFUTILS_HASHMAP_CALLOC) || !defined(BFUTILS_HASHMAP_FREE))) \
    || (defined(BFUTILS_HASHMAP_MALLOC) && (!defined(BFUTILS_HASHMAP_REALLOC) || !defined(BFUTILS_HASHMAP_CALLOC) || !defined(BFUTILS_HASHMAP_FREE))) \
    || (defined(BFUTILS_HASHMAP_CALLOC) && (!defined(BFUTILS_HASHMAP_MALLOC) || !defined(BFUTILS_HASHMAP_REALLOC) || !defined(BFUTILS_HASHMAP_FREE))) \
    || (defined(BFUTILS_HASHMAP_FREE) && (!defined(BFUTILS_HASHMAP_MALLOC) || !defined(BFUTILS_HASHMAP_REALLOC) || !defined(BFUTILS_HASHMAP_CALLOC))))
#error "You must define all BFUTILS_HASHMAP_REALLOC, BFUTILS_HASHMAP_CALLOC, BFUTILS_HASHMAP_MALLOC, BFUTILS_HASHMAP_FREE or neither."
#endif

#ifndef BFUTILS_HASHMAP_REALLOC
#include <stdlib.h>
#define BFUTILS_HASHMAP_REALLOC realloc
#define BFUTILS_HASHMAP_CALLOC calloc
#define BFUTILS_HASHMAP_MALLOC malloc
#define BFUTILS_HASHMAP_FREE free
#endif //BFUTILS_HASHMAP_REALLOC

#define BFUTILS_HASHMAP_ADDRESSOF(v) ((__typeof__(v)[1]){v})

#define bfutils_hashmap_header(h) ((h) ? (BFUtilsHashmapHeader *)(h) - 1 : NULL)
#define bfutils_hashmap_insert_count(h) ((h) ? bfutils_hashmap_header((h))->insert_count : 0)
#define bfutils_hashmap_length(h) ((h) ? bfutils_hashmap_header((h))->length : 0)
#define bfutils_hashmap_slots(h) ((h) ? bfutils_hashmap_header((h))->slots : NULL)
#define bfutils_hashmap_removed(h) ((h) ? bfutils_hashmap_header((h))->removed : NULL)
#define bfutils_hashmap_push(h, k, v) { \
    (h) = bfutils_hashmap_resize((h), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 0); \
    __typeof__((h)->key) __key = (k); \
    size_t __pos = bfutils_hashmap_insert_position((h), BFUTILS_HASHMAP_ADDRESSOF(__key), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 0); \
    (h)[__pos].key = __key; \
    (h)[__pos].value = (v); \
}
#define bfutils_hashmap_get(h, k) ((h)[bfutils_hashmap_get_position((h), BFUTILS_HASHMAP_ADDRESSOF(k), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 0)].value)
#define bfutils_hashmap_contains(h, k) (bfutils_hashmap_get_position((h), BFUTILS_HASHMAP_ADDRESSOF(k), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 0) > 0)
#define bfutils_hashmap_remove(h, k) ((h) = bfutils_hashmap_resize((h), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 0),\
    (h)[bfutils_hashmap_remove_key((h), BFUTILS_HASHMAP_ADDRESSOF(k), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 0)].value)
#define bfutils_string_hashmap_push(h, k, v) { \
    (h) = bfutils_hashmap_resize((h), sizeof(*(h)), offsetof(typeof(*(h)), key), sizeof((h)->key), 1); \
    __typeof__((h)->key) __key = (k); \
    size_t __pos = bfutils_hashmap_insert_position((h), __key, sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 1); \
    (h)[__pos].key = __key; \
    (h)[__pos].value = (v); \
}
#define bfutils_string_hashmap_get(h, k) ((h)[bfutils_hashmap_get_position((h), (k), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 1)].value)
#define bfutils_string_hashmap_contains(h, k) (bfutils_hashmap_get_position((h), (k), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 1) > 0)
#define bfutils_string_hashmap_remove(h, k) ((h) = bfutils_hashmap_resize((h), sizeof(*(h)), offsetof(typeof(*(h)), key), sizeof((h)->key), 1) ,\
    (h)[bfutils_hashmap_remove_key((h), (k), sizeof(*(h)), offsetof(__typeof__(*(h)), key), sizeof((h)->key), 1)].value)
#define bfutils_hashmap_free(h) (bfutils_hashmap_free_f((h), sizeof(*(h))), (h) = NULL)

#define bfutils_hashmap_iterator_next(h, i) ((h)[bfutils_hashmap_iterator_next_position(i)])
#define bfutils_hashmap_iterator_previous(h, i) ((h)[bfutils_hashmap_iterator_previous_position(i)])
#define bfutils_hashmap(element_free) (bfutils_hashmap_with_free(element_free))

extern void *bfutils_hashmap_resize(void *hm, size_t element_size, size_t key_offset, size_t key_size, int is_string);
extern size_t bfutils_hashmap_insert_position(void *hm, const void *key, size_t element_size, size_t key_offset, size_t key_size, int is_string);
extern size_t bfutils_hashmap_function(const void* key, size_t key_size);
extern long bfutils_hashmap_get_position(void *hm, const void *key, size_t element_size, size_t key_offset, size_t key_size, int is_string);
extern long bfutils_hashmap_remove_key(void *hm, const void *key, size_t element_size, size_t key_offset, size_t key_size, int is_string);
extern void bfutils_hashmap_free_f(void *hm, size_t element_size);

extern BFUtilsHashmapIterator bfutils_hashmap_iterator(void *hm);
extern BFUtilsHashmapIterator bfutils_hashmap_iterator_reverse(void *hm);
extern int bfutils_hashmap_iterator_has_next(BFUtilsHashmapIterator *it);
extern int bfutils_hashmap_iterator_has_previous(BFUtilsHashmapIterator *it);
extern size_t bfutils_hashmap_iterator_next_position(BFUtilsHashmapIterator *it);
extern size_t bfutils_hashmap_iterator_previous_position(BFUtilsHashmapIterator *it);
extern void *bfutils_hashmap_with_free(void (*element_free)(void*));

#endif // HASHMAP_H
#ifdef BFUTILS_HASHMAP_IMPLEMENTATION
#include <string.h>

void *bfutils_hashmap_with_free(void (*element_free)(void*)) {
    BFUtilsHashmapHeader *header = (BFUtilsHashmapHeader*) BFUTILS_HASHMAP_REALLOC(NULL, sizeof(BFUtilsHashmapHeader));
    header->length = 0;
    header->insert_count = 0;
    header->element_free = element_free;
    header->slots = NULL;
    header->removed = NULL;
    return (void*) (header + 1);
}

void *bfutils_hashmap_resize(void *hm, size_t element_size, size_t key_offset, size_t key_size, int is_string) {
    int need_to_grow = bfutils_hashmap_length(hm) == 0 || bfutils_hashmap_insert_count(hm) / (double) bfutils_hashmap_length(hm) > 0.5;
    int need_to_shrink = bfutils_hashmap_length(hm) > 32 && bfutils_hashmap_insert_count(hm) / (double) bfutils_hashmap_length(hm) < 0.25;
    if (!need_to_grow && !need_to_shrink) {
        return hm;
    }
    size_t old_length = bfutils_hashmap_length(hm);
    size_t length = bfutils_hashmap_length(hm) > 0 ? bfutils_hashmap_length(hm) * 2 : 32;
    if (need_to_shrink) {
        length = bfutils_hashmap_length(hm) / 2;
    }
    unsigned char *slots = bfutils_hashmap_slots(hm);
    unsigned char *removed = bfutils_hashmap_removed(hm);
    unsigned char *old_data = NULL;
    if (old_length > 0) {
        old_data = (unsigned char*) BFUTILS_HASHMAP_MALLOC(element_size * old_length);
        memcpy(old_data, hm, element_size * old_length);
    }
    void (*element_free)(void*) = bfutils_hashmap_header(hm) ? bfutils_hashmap_header(hm)->element_free : NULL;
    BFUtilsHashmapHeader *header = (BFUtilsHashmapHeader*) BFUTILS_HASHMAP_REALLOC(bfutils_hashmap_header(hm), sizeof(BFUtilsHashmapHeader) + (element_size * length));
    header->slots = (unsigned char*) BFUTILS_HASHMAP_CALLOC(1, length / 8 + 1);
    header->removed = (unsigned char*) BFUTILS_HASHMAP_CALLOC(1, length / 8 + 1);
    header->length = length;
    header->element_free = element_free;
    header->insert_count = 0;
    hm = (void*) (header + 1);

    if (old_length > 0) {
        for(int i = 0; i <= old_length / 8; i++) {
            for (int j = 0; j < 8; j++) {
                int is_slot_occupied = slots[i] & (1 << j);
                int is_removed = removed[i] & (1 << j);
                if (is_slot_occupied && !is_removed) {
                    void * key = old_data + ((i*8+j) * element_size) + key_offset;
                    size_t pos = bfutils_hashmap_insert_position(hm, key, element_size, key_offset, key_size, is_string);
                    void *element = (unsigned char*)hm + (pos * element_size);
                    void *source = old_data + ((i*8+j) * element_size);
                    memcpy(element, source, element_size);
                }
            }
        }
    }

    if(slots) {
        BFUTILS_HASHMAP_FREE(slots);
    }
    if(old_data) {
        BFUTILS_HASHMAP_FREE(old_data);
    }
    if(removed) {
        BFUTILS_HASHMAP_FREE(removed);
    }
    return hm;
}

int keycmp(const void *keya, const void *keyb, size_t key_size, int is_string) {
    if (is_string) {
        return strcmp(keya, *((char**) keyb));
    }
    return memcmp(keya, keyb, key_size);
}

size_t bfutils_hashmap_insert_position(void *hm, const void *key, size_t element_size, size_t key_offset, size_t key_size, int is_string) {
    size_t hash = is_string ? bfutils_hashmap_function(key, strlen(key)) : bfutils_hashmap_function(key, key_size);
    size_t index = hash % bfutils_hashmap_length(hm);
    size_t slot_index = index % 8;
    size_t slot_array_index = index / 8;
    size_t last_array_index = bfutils_hashmap_length(hm) / 8;

    int is_slot_occupied = bfutils_hashmap_slots(hm)[slot_array_index] & (1 << slot_index);
    int found_removed_slot = 0;
    size_t removed_slot_index = 0;
    while (is_slot_occupied) {
        int is_slot_removed = bfutils_hashmap_removed(hm)[slot_array_index] & (1 << slot_index);
        if (is_slot_removed && !found_removed_slot) {
            found_removed_slot = 1;
            removed_slot_index = slot_array_index * 8 + slot_index;
            continue;
        }
        void *src = (unsigned char*) hm + (index * element_size) + key_offset;
        if (0 == keycmp(key, src, key_size, is_string) && !is_slot_removed) {
            size_t pos = slot_array_index * 8 + slot_index;
            if (bfutils_hashmap_header(hm)->element_free != NULL) {
                bfutils_hashmap_header(hm)->element_free((unsigned char*) hm + (element_size * pos));
            }
            return pos;
        }

        slot_index++;
        if (slot_index == 8){
            slot_index = 0;
            slot_array_index = (slot_array_index + 1) % (last_array_index);
        }
        is_slot_occupied = bfutils_hashmap_slots(hm)[slot_array_index] & (1 << slot_index);
    }
    if (found_removed_slot) {
        bfutils_hashmap_removed(hm)[removed_slot_index / 8] &= ~(1 << (removed_slot_index % 8));
        bfutils_hashmap_slots(hm)[removed_slot_index / 8] |= (1 << (removed_slot_index % 8));
    }
    else {
        bfutils_hashmap_slots(hm)[slot_array_index] |= (1 << slot_index);
    }
    bfutils_hashmap_header(hm)->insert_count++;
    return slot_array_index * 8 + slot_index;
}

long bfutils_hashmap_get_position(void *hm, const void *key, size_t element_size, size_t key_offset, size_t key_size, int is_string) {
    size_t hash = is_string ? bfutils_hashmap_function(key, strlen(key)) : bfutils_hashmap_function(key, key_size);
    size_t index = hash % bfutils_hashmap_length(hm);
    size_t slot_index = index % 8;
    size_t slot_array_index = index / 8;
    size_t last_array_index = bfutils_hashmap_length(hm) / 8;

    int is_slot_occupied = bfutils_hashmap_slots(hm)[slot_array_index] & (1 << slot_index);
    while (is_slot_occupied) {
        index = slot_array_index * 8 + slot_index;
        int is_slot_removed = bfutils_hashmap_removed(hm)[slot_array_index] & (1 << slot_index);
        if (!is_slot_removed) {
            void *src = (unsigned char*) hm + (index * element_size) + key_offset;
            if (0 == keycmp(key, src, key_size, is_string)) {
                return index;
            }
        }

        slot_index++;
        if (slot_index == 8){
            slot_index = 0;
            slot_array_index = (slot_array_index + 1) % last_array_index;
        }
        is_slot_occupied = bfutils_hashmap_slots(hm)[slot_array_index] & (1 << slot_index);
    }
    return -1;
}

void bfutils_hashmap_free_f(void *hm, size_t element_size) {
    if (hm == NULL) return;
    if (bfutils_hashmap_header(hm)->element_free != NULL) {
        BFUtilsHashmapIterator it = bfutils_hashmap_iterator(hm);
        while(bfutils_hashmap_iterator_has_next(&it)) {
            size_t pos = bfutils_hashmap_iterator_next_position(&it);
            bfutils_hashmap_header(hm)->element_free((unsigned char*) hm + (pos * element_size));
        }
    }

    BFUTILS_HASHMAP_FREE(bfutils_hashmap_header(hm)->slots);
    BFUTILS_HASHMAP_FREE(bfutils_hashmap_header(hm)->removed);
    BFUTILS_HASHMAP_FREE(bfutils_hashmap_header(hm));
}

long bfutils_hashmap_remove_key(void *hm, const void *key, size_t element_size, size_t key_offset, size_t key_size, int is_string) {
    long index = bfutils_hashmap_get_position(hm, key, element_size, key_offset, key_size, is_string);
    if (index > 0) {
        size_t slot_index = index % 8;
        size_t slot_array_index = index / 8;
        bfutils_hashmap_removed(hm)[slot_array_index] |= (1 << slot_index);
        bfutils_hashmap_header(hm)->insert_count--;
    }
    return index;
}

size_t bfutils_hashmap_function(const void* key, size_t key_size) { //SDBM hash function
    unsigned char *str = (unsigned char *) key;
    size_t hash = 0;

    for (size_t i = 0; i < key_size; ++str, ++i) {
        hash = (*str) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

BFUtilsHashmapIterator bfutils_hashmap_iterator(void *hm) {
    unsigned char *slots = bfutils_hashmap_slots(hm);
    unsigned char *removed = bfutils_hashmap_removed(hm);
    size_t first = 0;
    size_t last = 0;
    int first_set = 0;
    for(int i = 0; i <= bfutils_hashmap_length(hm) / 8; i++) {
        for (int j = 0; j < 8; j++) {
            int is_slot_occupied = slots[i] & (1 << j);
            int is_removed = removed[i] & (1 << j);
            if (is_slot_occupied && !is_removed) {
                if (!first_set) {
                    first = i*8 + j;
                    first_set = 1;
                }
                last = i*8 + j;
            }
        }
    }
    return (BFUtilsHashmapIterator) {
        .h = bfutils_hashmap_header(hm),
        .current = first,
        .first = first,
        .last = last,
    };
}

BFUtilsHashmapIterator bfutils_hashmap_iterator_reverse(void *hm) {
    BFUtilsHashmapIterator it = bfutils_hashmap_iterator(hm);
    it.started = 1;
    return it;
}

int bfutils_hashmap_iterator_has_next(BFUtilsHashmapIterator *it) {
    if (it->started == 0) {
        return it->h->insert_count > 0;
    }
    return it->current < it->last;
}
int bfutils_hashmap_iterator_has_previous(BFUtilsHashmapIterator *it) {
    if (it->started == 1) {
        return it->h->insert_count > 0;
    }
    return it->current > it->first;
}

size_t bfutils_hashmap_iterator_next_position(BFUtilsHashmapIterator *it) {
    size_t index = it->started == 0 ? it->first : it->current + 1;
    size_t slot_index = index % 8;
    size_t slot_array_index = index / 8;

    unsigned char *slots = it->h->slots;
    unsigned char *removed = it->h->removed;
    while (index < it->last) {
        int is_slot_occupied = slots[slot_array_index] & (1 << slot_index);
        int is_slot_removed = removed[slot_array_index] & (1 << slot_index);
        if (is_slot_occupied && !is_slot_removed) {
            it->started = 2;
            it->current = index;
            return it->current;
        }

        slot_index++;
        if (slot_index == 8){
            slot_index = 0;
            slot_array_index = (slot_array_index + 1) % (it->h->length / 8);
        }
        index = slot_array_index * 8 + slot_index;
    }
    it->started = 2;
    it->current = it->last;
    return it->current;
}

size_t bfutils_hashmap_iterator_previous_position(BFUtilsHashmapIterator *it) {
    size_t index = it->started == 1 ? it->last : it->current - 1;
    size_t slot_index = index % 8;
    size_t slot_array_index = index / 8;

    unsigned char *slots = it->h->slots;
    unsigned char *removed = it->h->removed;
    while (index > it->first) {
        int is_slot_occupied = slots[slot_array_index] & (1 << slot_index);
        int is_slot_removed = removed[slot_array_index] & (1 << slot_index);
        if (is_slot_occupied && !is_slot_removed) {
            it->started = 2;
            it->current = index;
            return it->current;
        }

        if (slot_index == 0){
            slot_index = 7;
            slot_array_index = (slot_array_index - 1) % (it->h->length / 8);
        } else {
            slot_index--;
        }
        index = slot_array_index * 8 + slot_index;
    }
    it->started = 2;
    it->current = it->first;
    return it->current;
}
#endif //BFUTILS_HASHMAP_IMPLEMENTATION
