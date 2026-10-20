#include "vector.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ALLIGNMENT 32

// rounds up to closest power of 2
size_t uppow2(size_t x)
{
    if (x <= 1) return 1;

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

#if SIZE_MAX > UINT32_MAX
    x |= x >> 32;
#endif

    return x + 1;
}

size_t _cap_align(size_t cap_bytes)
{
    return (cap_bytes + (ALLIGNMENT - 1)) & ~(size_t)(ALLIGNMENT - 1);
}

// data needs to be null for clear alloc
static bool _vec_alloc_data(vector_t *vec, size_t cap, size_t elem)
{
    if (!vec->m_data && vec->m_flags & VFLAG_FIXED_CAP)
    {
        return false;
    }

    size_t new_cap = _cap_align(cap * elem) / elem;
    
    void *data = realloc(vec->m_data, new_cap * elem);
    if (!data)
    {
        return false;
    }

    vec->m_data = data;
    vec->m_cap = new_cap;
    vec->m_elem = (unsigned)elem;

    return true;
}

static void inline _vec_set(vector_t *vec, size_t index, size_t count, void *data)
{
    memcpy((char*)vec->m_data + index * vec->m_elem, data, count * vec->m_elem);
}

static void inline _vec_get(vector_t *vec, size_t index, size_t count, void *data)
{
    memcpy(data, (char*)vec->m_data + index * vec->m_elem, count * vec->m_elem);
}

// returns ptr to data's index element
static void * _vec_elem(vector_t *vec, size_t index)
{
    if (!vec->m_data) return NULL;
    return (char*)vec->m_data + index * vec->m_elem;
}

bool vector_init(vector_t *vec, size_t element_size, size_t capacity)
{
    assert(vec != NULL);
    assert(capacity > 0);
    assert(element_size > 0);

    vec->m_data = NULL;

    if (!_vec_alloc_data(vec, capacity, element_size))
    {
        return false;
    }

    vec->m_size = 0;
    vec->m_flags = 0;

    return true;
}

vector_t * vector_new(size_t element_size, size_t capacity)
{
    assert(capacity > 0);
    assert(element_size > 0);
    
    vector_t *vec = malloc(sizeof(vector_t));
    if (!vec)
    {
        return NULL;
    }

    vec->m_data = NULL;

    if (!_vec_alloc_data(vec, capacity, element_size))
    {
        return false;
    }

    vec->m_size = 0;
    vec->m_flags = VFLAG_HEAP_STRUCT;

    return vec;
}

void vector_destry(vector_t *vec)
{
    if (vec)
    {
        if (vec->m_data)
        {
            free(vec->m_data);
            vec->m_data = NULL;
        }

        if (vec->m_flags & VFLAG_HEAP_STRUCT)
        {
            free(vec);
        }
    }
}

void * vector_steal_data(vector_t* vec, size_t *out_capacity)
{
    assert(vec != NULL);

    void *data = vec->m_data;
    *out_capacity = vec->m_cap;

    vec->m_data = NULL;
    vec->m_size = 0;
    vec->m_cap = 0;
    vec->m_elem = 0;
    vec->m_flags &= ~VFLAG_CLEAR_ON_STEAL;

    return data;
}

bool vector_set_fixed_capacity(vector_t *vec, size_t capacity)
{
    assert(vec != NULL);
    assert(capacity > 0);

    vec->m_flags &= ~ VFLAG_FIXED_CAP;

    if (!_vec_alloc_data(vec, capacity, vec->m_elem))
    {
        return false;
    }

    vec->m_cap = capacity;
    vec->m_cap |= VFLAG_FIXED_CAP;

    return true;
}

void * vector_front(vector_t *vec)
{
    assert(vec != NULL);

    return vec->m_size > 0 && vec->m_data ? (void*)vec->m_data : NULL;
}

void * vector_back(vector_t *vec)
{
    assert(vec != NULL);

    return vec->m_size > 0 && vec->m_data ? (char*)vec->m_data + (vec->m_size - 1) * vec->m_elem : NULL;
}

bool vector_insert(vector_t *vec, size_t idx, void *data)
{
    assert(vec != NULL);
    assert(data != NULL);

    if (idx > vec->m_size) return false;

    if (
        vec->m_size >= vec->m_cap &&
        !_vec_alloc_data(vec, vec->m_cap * 2, vec->m_elem)
    )
    {
        return false;
    }

    if (idx == vec->m_size)
    {
        return vector_push(vec, data);
    }

    memmove(
        (char*)vec->m_data + (idx + 1) * vec->m_elem,
        (char*)vec->m_data +  idx * vec->m_elem,
        (vec->m_size - idx) * vec->m_elem
    );
    _vec_set(vec, idx, 1, data);
    vec->m_size++;

    return true;
}

bool vector_delete(vector_t *vec, size_t idx)
{
    assert(vec != NULL);

    if (idx >= vec->m_size) return false;

    memmove(
        _vec_elem(vec, idx),
        _vec_elem(vec, idx + 1),
        (vec->m_size - idx) * vec->m_elem
    );

    vec->m_size--;

    return true;
}

bool vector_get(vector_t *vec, size_t idx, void *out_obj);
bool vector_set(vector_t *vec, size_t idx, void *obj);

bool vector_append(vector_t *vec, void *data, size_t count)
{
    assert(vec != NULL);
    assert(data != NULL);
    assert(count > 0);

    size_t desired_size = vec->m_size + count;
    size_t multiplier = 1;

    while (desired_size > vec->m_cap * multiplier)
    {
        multiplier <<= 1; // *= 2;
    }
    if (
        multiplier > 1 &&
        _vec_alloc_data(vec, vec->m_cap * multiplier, vec->m_elem)
    )
    {
        return false;
    }

    memcpy(
        (char*)vec->m_data + vec->m_size * vec->m_elem,
        data,
        count * vec->m_elem
    );

    vec->m_size += count;

    return true;
}

bool vector_cut(vector_t *vec, void* out_data, size_t count);

bool vector_push(vector_t *vec, void *obj)
{
    assert(vec != NULL);
    assert(obj != NULL);

    if (
        vec->m_size >= vec->m_cap &&
        !_vec_alloc_data(vec, vec->m_cap * 2, vec->m_elem)
    )
    {
       return false;
    }

    _vec_set(vec, vec->m_size, 1, obj);
    vec->m_size++;

    return true;
}

bool vector_pop(vector_t *vec, void *out)
{
    assert(vec != NULL);
    assert(out != NULL);

    _vec_get(vec, vec->m_size - 1, 1, out);
    vec->m_size--;

    return true;
}

void vector_resize(vector_t *vec, size_t capacity)
{
    assert(vec != NULL);

    if (vec->m_cap < capacity)
    {
    }
}

void vector_clear(vector_t *vec)
{
    assert(vec != NULL);

    vec->m_size = 0;
}

void vector_print(vector_t *vec)
{
    printf("vector::data = %p\n", vec->m_data);
    printf("vector::size = %zu\n", vec->m_size);
    printf("vector::capacity = %zu\n", vec->m_cap);
    printf("vector::element_size = %u\n", vec->m_elem);
    printf("vector::flags = %x\n", vec->m_flags);
}

void vector_print_if(vector_t *vec, bool (*when)(size_t, const void*), void (*print)(size_t, const void*))
{
    assert(vec != NULL);
    assert(when != NULL);
    assert(print != NULL);

    for (size_t i = 0; i < vec->m_size; i++)
    {
        const void *current = (char*)vec->m_data + i * vec->m_elem;
        if (when(i, current))
        {
            print(i, current);
        }
    }
}