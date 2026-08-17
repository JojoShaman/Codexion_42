#include "../../include/codexion.h"

bool    init_heap(t_data *data)
{
    t_heap *heap;

    heap = malloc(data->number_of_coders * sizeof(*heap));
    if (!heap)
        return (false);
    return (true);
}


void    heapify(t_heap *arr, int i)
{
    t_heap_node     *tmp;
    int     parent;
    int     child1;
    int     child2;
    int     size;

    tmp = NULL;
    parent = i;
    child1 = 2 * i + 1;
    child2 = 2 * i + 2;
    size = arr->size;

    if (child1 < size && arr->node[child1].deadline < arr->node[parent].deadline)
        parent = child1;
    if (child2 < size && arr->node[child2].deadline < arr->node[parent].deadline)
        parent = child2;
    if (parent != i)
    {
        tmp = &arr->node[i];
        arr->node[i] = arr->node[parent];
        arr->node[parent] = *tmp;
        heapify(arr, parent);
    }
}

void    build_heap(t_data *data)
{
    int     start;
    int     i;

    start = (data->number_of_coders / 2) - 1;
    i = 0;
    data->heap->size = data->number_of_coders;
    while (++i < data->number_of_coders)
        data->heap->node[i].coder = &data->coders[i];
    i = start + 1;
    while (--i >= 0)
        heapify(data->heap, i);
}