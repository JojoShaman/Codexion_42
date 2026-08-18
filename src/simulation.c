#include "../include/codexion.h"
#include <pthread.h>
#include <stdbool.h>

t_coder *first_arrived(t_dongle *dongle)
{
    t_coder *left;
    t_coder *right;

    left = dongle->left;
    right = dongle->right;
    if (right->status == WAITING_DONGLE
            && left->status == WAITING_DONGLE)
    {
        if (left->arrival_time > right->arrival_time)
                return (right);
        else
                return (left);
    }
    else if (left->status == WAITING_DONGLE)
        return (left);
    return (right);
}

void    dongle_acquire(t_dongle *dongle, t_coder *coder)
{
    pthread_mutex_lock(&dongle->mutex);
    coder->status = WAITING_DONGLE;
    while (dongle->taken)
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
    while (get_time(MILLISECOND) < dongle->last_release +
            coder->data_all->dongle_cooldown)
        dongle_cooldown(dongle, coder->data_all);
    while (coder != first_arrived(dongle))
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
    coder->status = TAKING_DONGLE;
    dongle->taken = true;
    pthread_mutex_unlock(&dongle->mutex);
}

void    update_deadline(t_coder *coder)
{
    int         time_to_burnout;
    long long   last_compile;
    long long   deadline_before;
    long long   deadline_after;
    t_heap      *heap;

    heap = NULL;
    last_compile = coder->last_compile_time;
    time_to_burnout = coder->data_all->time_to_burnout;
    deadline_before = 0;
    deadline_after = 0;
    pthread_mutex_lock(&coder->data_all->heap->mutex);
    heap = coder->data_all->heap;
    deadline_before = heap->node[0].deadline;
    heap->node[coder->heap_index].deadline = last_compile + time_to_burnout;
    heapify(heap, coder->heap_index);
    deadline_after = heap->node[0].deadline;
    if (deadline_after != deadline_before)
        pthread_cond_signal(&coder->data_all->monitor_cond);
    pthread_mutex_unlock(&coder->data_all->heap->mutex);
}

void    dongle_release(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);
    dongle->taken = false;
    dongle->last_release = get_time(MILLISECOND);
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}

void    set_status(t_coder *coder, t_status log)
{
    coder->status = log;
    output(coder, log);
}

void    compile(t_coder *coder)
{
    coder->arrival_time = get_time(MILLISECOND);
    dongle_acquire(coder->first_dongle, coder);
    set_status(coder, TAKING_DONGLE);
    dongle_acquire(coder->second_dongle, coder);
    set_status(coder, TAKING_DONGLE);
    coder->last_compile_time = get_time(MILLISECOND);
    update_deadline(coder);
    set_status(coder, COMPILING);
    coder->compile_count += 1;
    ft_usleep(coder->data_all->time_to_compile, coder);
    dongle_release(coder->first_dongle);
    dongle_release(coder->second_dongle);
}

void    debug(t_coder *coder)
{
    set_status(coder, DEBUGGING);
    ft_usleep(coder->data_all->time_to_debug, coder);
}

void    refacor(t_coder *coder)
{
    set_status(coder, REFACTORING);
    ft_usleep(coder->data_all->time_to_refactor, coder);
}

bool    is_end(t_data *data)
{
    bool    is_end;

    is_end = false;
    pthread_mutex_lock(&data->end_mutex);
    if (data->end_of_simulation)
        is_end = true;
    pthread_mutex_unlock(&data->end_mutex);
    return (is_end);
}


void    *simulate(void *data)
{
    t_coder *coder;

    coder = (t_coder *)data;

    pthread_mutex_lock(&coder->data_all->ready_mutex);
    while (!coder->data_all->ready)
        pthread_cond_wait(&coder->data_all->ready_cond, &coder->data_all->ready_mutex);
    pthread_mutex_unlock(&coder->data_all->ready_mutex);
    while (!is_end(coder->data_all))
    {
        compile(coder);
        debug(coder);
        refacor(coder);
        if (coder->compile_count == coder->data_all->number_of_compiles_required)
            break;
    }
    return NULL;
}

void    coders_ready(t_data *data)
{
    pthread_mutex_lock(&data->ready_mutex);
    data->ready = true;
    pthread_cond_broadcast(&data->ready_cond);
    pthread_mutex_unlock(&data->ready_mutex);
}

void    *monitoring(void *all_data)
{
    t_heap          *heap;
    t_data          *data;
    struct timespec ts;
    int             rt;

    heap = NULL;
    data = (t_data *)all_data;
    heap = data->heap;
    while(!data->end_of_simulation)
    {
        pthread_mutex_lock(&data->heap->mutex);
        ts.tv_sec = heap->node[0].deadline / 1000;
        ts.tv_nsec = (heap->node[0].deadline % 1000) * 1000000;
        rt = pthread_cond_timedwait(&data->monitor_cond, &data->heap->mutex, &ts);
        if (rt == ETIMEDOUT)
        {
            pthread_mutex_lock(&data->end_mutex);
            data->end_of_simulation = true;
            output(heap->node[0].coder, BURNED_OUT);
            pthread_mutex_unlock(&data->end_mutex);
        }
        pthread_mutex_unlock(&data->heap->mutex);
    }
    return NULL;
}

void    run_thread(t_data *data)
{
    int     i;

    i = -1;
    pthread_create(&data->monitor, NULL, monitoring, &data);
    while (++i < data->number_of_coders)
        pthread_create(&data->coders[i].thread, NULL, simulate, &data->coders[i]);
    coders_ready(data);
    i = -1;
    while (++i < data->number_of_coders)
        pthread_join(data->coders[i].thread, NULL);
    pthread_join(data->monitor, NULL);
}
