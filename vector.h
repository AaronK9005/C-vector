#ifndef MY_VECTOR_DATA_STRUCTURE_H
#define MY_VECTOR_DATA_STRUCTURE_H

#include <stddef.h>
#include <stdbool.h>

// #warning Not ready yet! Needs essential foundations

typedef struct _vector vector_t;
typedef struct _extended_vector extVector_t;

#define VFLAG_HEAP_STRUCT 0x01 // vector struct is allocated
#define VFLAG_FIXED_CAP 0x04 // can not resize
#define VFLAG_EXTENDED 0x08 // vector is extended (has fptrs for printing and stuff)
#define VFLAG_NONHEAP_DATA 0x10 // data was not allocatedd

#define VFLAG_CLEAR_ON_STEAL (VFLAG_FIXED_CAP | VFLAG_NONHEAP_DATA)

struct _vector {
    void *m_data;
    size_t m_size;
    size_t m_cap;
    unsigned m_elem; // size of one element
    unsigned m_flags;
};

struct _extended_vector {
    vector_t base;
    int (*compare)(const void*, const void*);
    bool (*when)(size_t, const void*);
    void (*print)(size_t, const void*);
};

bool vector_init(vector_t *vec, size_t element_size, size_t capacity);
vector_t * vector_new(size_t element_size, size_t capacity);
void vector_destry(vector_t *vec);

void * vector_steal_data(vector_t* vec, size_t *out_capacity);

bool vector_set_fixed_capacity(vector_t *vec, size_t capacity);

void * vector_front(vector_t *vec);
void * vector_back(vector_t *vec);

bool vector_push(vector_t *vec, void *obj);
bool vector_pop(vector_t *vec, void *out);
bool vector_insert(vector_t *vec, size_t idx, void *data);
bool vector_delete(vector_t *vec, size_t idx);

bool vector_get(vector_t *vec, size_t idx, void *out_obj);
bool vector_set(vector_t *vec, size_t idx, void *obj);

#define vector_APPEND(vec, array) vector_append((vec), (array), (sizeof(array) / sizeof(array[0])))
bool vector_append(vector_t *vec, void *data, size_t count);
bool vector_cut(vector_t *vec, void* out_data, size_t count); //? reneame

void vector_print(vector_t *vec);
void vector_print_if(vector_t *vec, bool (*when)(size_t, const void*), void (*print)(size_t, const void*));

void vector_resize(vector_t *vec, size_t capacity);
void vector_clear(vector_t *vec);

#endif