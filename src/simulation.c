#include "../include/codexion.h"
#include <pthread.h>

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

long long   get_long(t_mtx *mutex, long long *value)
{
    long long   ret;

    pthread_mutex_lock(mutex);
    ret = *value;
    pthread_mutex_unlock(mutex);
    return (ret);
}

void    set_long(t_mtx *mutex, long long *dir, long long value)
{
    pthread_mutex_lock(mutex);
    *dir = value;
    pthread_mutex_unlock(mutex);
}

t_coder *first_arrived(t_dongle *dongle)
{
    bool    left_wait;
    bool    right_wait;
    long long   left_arrival;
    long long   right_arrival;
    t_coder     *first;

    left_wait = (dongle->left->waiting_for == dongle);
    right_wait = (dongle->right->waiting_for == dongle);
    left_arrival = get_long(
        &dongle->left->read_long_mutex, &dongle->left->arrival_time);
    right_arrival = get_long(
        &dongle->right->read_long_mutex, &dongle->right->arrival_time);
    if (left_wait && right_wait)
    {
        if (left_arrival > right_arrival)
                first = dongle->right;
        else
                first = dongle->left;
    }
    else if (left_wait)
        first = dongle->left;
    else
        first = dongle->right;
    return (first);
}

void    set_status(t_coder *coder, t_status log, bool display)
{
    pthread_mutex_lock(&coder->status_mutex);
    coder->status = log;
    if (display)
        output(coder, log);
    pthread_mutex_unlock(&coder->status_mutex);
}

bool    dongle_acquire(t_dongle *dongle, t_coder *coder)
{
    pthread_mutex_lock(&dongle->mutex);
    coder->waiting_for = dongle;
    set_status(coder, WAITING_DONGLE, false);
    while (dongle->taken && !is_end(coder->data_all))
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
    if (is_end(coder->data_all))
    {
        coder->waiting_for = NULL;
        pthread_mutex_unlock(&dongle->mutex);
        return (false);
    }
    if (get_time(MILLISECOND) < dongle->last_release +
            coder->data_all->dongle_cooldown)
        dongle_cooldown(dongle, coder->data_all);
    while (coder != first_arrived(dongle) && !is_end(coder->data_all))
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
    if (is_end(coder->data_all))
    {
        coder->waiting_for = NULL;
        pthread_mutex_unlock(&dongle->mutex);
        return (false);
    }
    dongle->taken = true;
    coder->waiting_for = NULL;
    pthread_mutex_unlock(&dongle->mutex);
    return (true);
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

void    compile(t_coder *coder)
{
    if (is_end(coder->data_all))
        return ;
    set_long(
        &coder->read_long_mutex,
        &coder->arrival_time,
        get_time(MILLISECOND));
    if (!dongle_acquire(coder->first_dongle, coder))
        return;
    set_status(coder, TOOK_FIRST, true);
    if (!dongle_acquire(coder->second_dongle, coder))
        return ;
    set_status(coder, TOOK_SECOND, true);
    coder->last_compile_time = get_time(MILLISECOND);
    update_deadline(coder);
    set_status(coder, COMPILING, true);
    coder->compile_count += 1;
    ft_usleep(coder->data_all->time_to_compile, coder);
    dongle_release(coder->first_dongle);
    dongle_release(coder->second_dongle);
}

void    debug(t_coder *coder)
{
    if (is_end(coder->data_all))
        return ;
    set_status(coder, DEBUGGING, true);
    ft_usleep(coder->data_all->time_to_debug, coder);
}

void    refacor(t_coder *coder)
{
    if (is_end(coder->data_all))
        return ;
    set_status(coder, REFACTORING, true);
    ft_usleep(coder->data_all->time_to_refactor, coder);
}

void    set_end(t_data *data)
{
    int     i;

    i = -1;
    pthread_mutex_lock(&data->end_mutex);
    data->end_of_simulation = true;
    pthread_mutex_unlock(&data->end_mutex);
    while (++i < data->number_of_coders)
    {
        pthread_cond_broadcast(&data->coders[i].cond);
        pthread_mutex_lock(&data->dongles[i].mutex);
        pthread_cond_broadcast(&data->dongles[i].cond);
        pthread_mutex_unlock(&data->dongles[i].mutex);
    }
}

void    coder_finished(t_data *data)
{
    bool    all_finished;

    all_finished = false;
    pthread_mutex_lock(&data->another_mutex);
    data->coders_finished++;
    if (data->coders_finished == data->number_of_coders)
        all_finished = true;
    pthread_mutex_unlock(&data->another_mutex);
    if (all_finished)
    {
        set_end(data);
        pthread_mutex_lock(&data->heap->mutex);
        pthread_cond_signal(&data->monitor_cond);
        pthread_mutex_unlock(&data->heap->mutex);
    }
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
        {
            coder->finished = true;
            coder_finished(coder->data_all);
            break;
        }
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
    while(!is_end(data))
    {
        pthread_mutex_lock(&data->heap->mutex);
        ts.tv_sec = heap->node[0].deadline / 1000;
        ts.tv_nsec = (heap->node[0].deadline % 1000) * 1000000;
        rt = pthread_cond_timedwait(&data->monitor_cond, &data->heap->mutex, &ts);
        if (rt == ETIMEDOUT && !is_end(data) && !heap->node[0].coder->finished)
        {
            set_end(data);
            output(heap->node[0].coder, BURNED_OUT);
        }
        pthread_mutex_unlock(&data->heap->mutex);
    }
    return NULL;
}

void    run_thread(t_data *data)
{
    int     i;

    i = -1;
    pthread_create(&data->monitor, NULL, monitoring, data);
    while (++i < data->number_of_coders)
        pthread_create(&data->coders[i].thread, NULL, simulate, &data->coders[i]);
    coders_ready(data);
    i = -1;
    while (++i < data->number_of_coders)
        pthread_join(data->coders[i].thread, NULL);
    pthread_join(data->monitor, NULL);
}
