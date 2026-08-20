/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   utils.c                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */
#include "../../include/codexion.h"

long get_time(t_time time)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    if (time == SECOND)
        return (tv.tv_sec);
    else if (time == MILLISECOND)
        return (tv.tv_sec * 1e3 + tv.tv_usec / 1e3);
    else if (time == MICROSECOND)
        return (tv.tv_usec);
    return 0;
}

void    ft_usleep(int to_sleep, t_coder *coder)
{
    struct timeval tv;
    struct timespec ts;
    int     rt;

    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + ((tv.tv_usec + (to_sleep * 1000)) / 1000000);
    ts.tv_nsec = ((tv.tv_usec + (to_sleep * 1000)) % 1000000) * 1000;
    pthread_mutex_lock(&coder->data_all->end_mutex);
    while (!coder->data_all->end_of_simulation)
    {
        rt = pthread_cond_timedwait(&coder->cond,
            &coder->data_all->end_mutex, &ts);
        if (rt == ETIMEDOUT)
            break;
    }
    pthread_mutex_unlock(&coder->data_all->end_mutex);
}

void    dongle_cooldown(t_dongle *dongle, t_data *data)
{
    struct timeval tv;
    struct timespec ts;
    int     rt;
    int     cooldown;

    gettimeofday(&tv, NULL);
    cooldown = data->dongle_cooldown - (
        get_time(MILLISECOND) - dongle->last_release);
    ts.tv_sec = tv.tv_sec + (
        (tv.tv_usec + (cooldown * 1000)) / 1000000);
    ts.tv_nsec = (
        (tv.tv_usec + (cooldown * 1000)) % 1000000) * 1000;
    while (
        get_time(MILLISECOND) < dongle->last_release + data->dongle_cooldown)
    {
        rt = pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
        if (rt == ETIMEDOUT)
            break;
    }
}