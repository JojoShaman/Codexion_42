#include "../../include/codexion.h"

void    init_dongle(t_data *data)
{
    int     i;

    i = -1;
    while (++i < data->number_of_coders)
    {
        data->dongles[i].id = i;
        pthread_cond_init(&data->dongles[i].cond, NULL);
        pthread_mutex_init(&data->dongles[i].mutex, NULL);
    }
}

void    attribute_dongle(t_coder *coder, int position)
{
    t_data  *data;

    data = coder->data_all;
    if (position % 2)
    {
        coder->first_dongle = &data->dongles[(position + 1) %
            data->number_of_coders];
        coder->second_dongle = &data->dongles[position];
    }
    else
    {
        coder->first_dongle = &data->dongles[position];
        coder->second_dongle = &data->dongles[(position + 1) %
            data->number_of_coders];
    }
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
        coder->compile_count = 0;
        coder->last_compile_time = get_time(MILLISECOND);
        pthread_cond_init(&coder->cond, NULL);
        coder->data_all = data;
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
    data->simulation_start = 0;
    data->coders_ready = false;
    data->end_of_simulation = false;
    data->coders = malloc(data->number_of_coders * sizeof(*data->coders));
    if (!data->coders)
        return 0;
    data->dongles = malloc(data->number_of_coders * sizeof((*data->dongles)));
    if (!data->dongles)
        return 0;
    init_coders(data);
    pthread_mutex_init(&data->write_mutex, NULL);
    pthread_mutex_init(&data->end_mutex, NULL);
    return 1;
}

