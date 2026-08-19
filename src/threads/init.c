#include "../../include/codexion.h"
#include <pthread.h>

void    init_dongle(t_data *data)
{
    int     i;

    i = -1;
    while (++i < data->number_of_coders)
    {
        data->dongles[i].id = i;
        data->dongles[i].last_release = 0;
        data->dongles[i].heap = data->heap;
        data->dongles[i].taken = false;
        pthread_cond_init(&data->dongles[i].cond, NULL);
        pthread_mutex_init(&data->dongles[i].mutex, NULL);
    }
}

void    attribute_dongle(t_coder *coder, int position)
{
    t_data  *data;
    int     n;

    data = coder->data_all;
    n = data->number_of_coders;
    if (position % 2) // if odd pos, left dongle first
    {
        coder->first_dongle = &data->dongles[(position + 1) % n];
        coder->second_dongle = &data->dongles[position];
    }
    else // if even pos, right dongle first
    {
        coder->first_dongle = &data->dongles[position];
        coder->second_dongle = &data->dongles[(position + 1) % n];
    }
    data->dongles[position].left = &data->coders[position];
    data->dongles[position].right = &data->coders[(position - 1 + n) % n];
}

void    init_coders(t_data *data)
{
    int     i;
    t_coder *coder;

    i = -1;
    while (++i < data->number_of_coders)
    {
        coder = &data->coders[i];
        coder->id = i + 1;
        coder->heap_index = i;
        coder->compile_count = 0;
        coder->last_compile_time = get_time(MILLISECOND);
        pthread_cond_init(&coder->cond, NULL);
        pthread_mutex_init(&coder->mutex, NULL);
        pthread_mutex_init(&coder->status_mutex, NULL);
        pthread_mutex_init(&coder->read_long_mutex, NULL);
        coder->data_all = data;
        coder->status = WAITING_DONGLE;
        coder->waiting_for = NULL;
        coder->finished = false;
        attribute_dongle(coder, i);
    }
}

bool    init_data(t_data *data, char **argv)
{
    data->number_of_coders = atoi(argv[1]);
    data->time_to_burnout = atoi(argv[2]);
    data->time_to_compile = atoi(argv[3]);
    data->time_to_debug = atoi(argv[4]);
    data->time_to_refactor = atoi(argv[5]);
    data->number_of_compiles_required = atoi(argv[6]);
    data->dongle_cooldown = atoi(argv[7]);
    if (!strcmp(argv[8], "fifo"))
        data->scheduler = FIFO;
    else
        data->scheduler = EDF;
    data->simulation_start = get_time(MILLISECOND);
    data->ready = false;
    data->end_of_simulation = false;
    data->coders_finished = 0;
    data->coders = malloc(data->number_of_coders * sizeof(*data->coders));
    if (!data->coders)
        return 0;
    data->dongles = malloc(data->number_of_coders * sizeof((*data->dongles)));
    if (!data->dongles)
    {
        free(data->coders);
        return 0;
    }
    init_coders(data);
    build_heap(data);
    init_dongle(data);
    pthread_mutex_init(&data->write_mutex, NULL);
    pthread_mutex_init(&data->end_mutex, NULL);
    pthread_mutex_init(&data->ready_mutex, NULL);
    pthread_mutex_init(&data->another_mutex, NULL);
    pthread_cond_init(&data->ready_cond, NULL);
    pthread_cond_init(&data->monitor_cond, NULL);
    return 1;
}

