/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   routine.c                                         :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

static bool	compile(t_coder *coder);
static void	debug_and_refactor(t_coder *coder);
static void	coder_is_done(t_data *data);

void	*run(void *data)
{
	t_coder	*coder;

	coder = (t_coder *)data;
	pthread_mutex_lock(&coder->data->gate_mutex);
	while (!coder->data->ready)
		pthread_cond_wait(&coder->data->gate_cond,
			&coder->data->gate_mutex);
	pthread_mutex_unlock(&coder->data->gate_mutex);
	while (!is_end(coder->data))
	{
		if (coder->compile_count == coder->data->number_of_compiles_required)
		{
			pthread_mutex_lock(&coder->mutex);
			coder->finished = true;
			pthread_mutex_unlock(&coder->mutex);
			remove_node(coder);
			coder_is_done(coder->data);
			break ;
		}
		if (!compile(coder))
			break ;
		if (!is_end(coder->data))
			debug_and_refactor(coder);
	}
	return (NULL);
}

static bool	compile(t_coder *coder)
{
	if (is_end(coder->data))
		return (false);
	pthread_mutex_lock(&coder->mutex);
	coder->arrival_time = get_time(MILLISECOND);
	pthread_mutex_unlock(&coder->mutex);
	if (!dongle_acquire(coder->first_dongle, coder))
		return (false);
	set_status(coder, TOOK_FIRST, true);
	if (!dongle_acquire(coder->second_dongle, coder))
	{
		dongle_release(coder->first_dongle);
		return (false);
	}
	set_status(coder, TOOK_SECOND, true);
	coder->last_compile_time = get_time(MILLISECOND);
	update_deadline(coder);
	set_status(coder, COMPILING, true);
	coder->compile_count += 1;
	ft_usleep(coder->data->time_to_compile, coder);
	dongle_release(coder->first_dongle);
	dongle_release(coder->second_dongle);
	return (true);
}

static void	debug_and_refactor(t_coder *coder)
{
	if (is_end(coder->data))
		return ;
	set_status(coder, DEBUGGING, true);
	ft_usleep(coder->data->time_to_debug, coder);
	if (is_end(coder->data))
		return ;
	set_status(coder, REFACTORING, true);
	ft_usleep(coder->data->time_to_refactor, coder);
}

static void	coder_is_done(t_data *data)
{
	bool	all_finished;

	all_finished = false;
	pthread_mutex_lock(&data->finished_mutex);
	data->coders_finished++;
	if (data->coders_finished == data->number_of_coders)
		all_finished = true;
	pthread_mutex_unlock(&data->finished_mutex);
	if (all_finished)
	{
		set_end(data);
		pthread_mutex_lock(&data->heap->mutex);
		pthread_cond_signal(&data->monitor_cond);
		pthread_mutex_unlock(&data->heap->mutex);
	}
}
