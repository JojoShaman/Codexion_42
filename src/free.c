#include "../include/codexion.h"

void    cleanup(t_data *data)
{
    int     i;

    i = -1;
    while (++i < data->number_of_coders)
    {
        pthread_mutex_destroy(&data->coders[i].mutex);
        pthread_cond_destroy(&data->coders[i].cond);
        pthread_mutex_destroy(&data->coders[i].status_mutex);
        pthread_mutex_destroy(&data->coders[i].read_long_mutex);
        pthread_mutex_destroy(&data->dongles[i].mutex);
        pthread_cond_destroy(&data->dongles[i].cond);
    }
    if (data->coders)
        free(data->coders);
    if (data->dongles)
        free(data->dongles);
    if (data->heap)
    {
        if (data->heap->node)
            free(data->heap->node);
        free(data->heap);
    }
    pthread_mutex_destroy(&data->ready_mutex);
    pthread_mutex_destroy(&data->another_mutex);
    pthread_mutex_destroy(&data->end_mutex);
    pthread_mutex_destroy(&data->write_mutex);
    pthread_cond_destroy(&data->monitor_cond);
    pthread_cond_destroy(&data->ready_cond);
    if (data)
        free(data);
}