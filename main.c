#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

bool _vec_when(size_t idx, const void *data);

void _vec_print(size_t idx, const void *data);

int main(int argc, char const *argv[])
{
    vector_t _vec, *vec = &_vec;
    if (!vector_init(&_vec, sizeof(int), 10))
    {
        return 1;
    }

    _vec.m_flags |= VFLAG_FIXED_CAP;

    int data[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };

    vector_APPEND(vec, data);

    float fdata[] = {
        1.0f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f
    };

    vector_APPEND(vec, fdata);

    vector_print(vec);
    vector_print_if(vec, _vec_when, _vec_print);

    return 0;
}

bool _vec_when(size_t idx, const void *data)
{
    return *(const int*)data != 0;
}

void _vec_print(size_t idx, const void *data)
{
    int x = *(int*)data;
    printf("%zu: val = %d\n", idx, x);
}
