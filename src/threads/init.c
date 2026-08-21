/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   data.c.                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */
#include "../../include/codexion.h"

void	create_join(t_data *data)
{
	int	i;

	i = -1;
    pthread_create(&data->monitor, NULL, monitoring, data);
	while (++i < data->number_of_coders)
		pthread_create(&data->coders[i].thread, NULL, run, &data->coders[i]);
	pthread_mutex_lock(&data->gate_mutex);
	data->ready = true;
	pthread_cond_broadcast(&data->gate_cond);
	pthread_mutex_unlock(&data->gate_mutex);
	i = -1;
	while (++i < data->number_of_coders)
		pthread_join(data->coders[i].thread, NULL);
	pthread_join(data->monitor, NULL);
}

bool	init_dongle(t_data *data)
{
	int	i;
	int	n;

	i = -1;
	n = data->number_of_coders;
	data->dongles = malloc(data->number_of_coders * sizeof(t_dongle));
	if (!data->dongles)
		return (false);
	while (++i < data->number_of_coders)
	{
		data->dongles[i].id = i;
		data->dongles[i].last_release = 0;
		data->dongles[i].heap = data->heap;
		data->dongles[i].taken = false;
		pthread_cond_init(&data->dongles[i].cond, NULL);
		pthread_mutex_init(&data->dongles[i].mutex, NULL);
		data->dongles[i].left = &data->coders[i];
		data->dongles[i].right = &data->coders[(i - 1 + n) % n];
	}
	return (true);
}

bool	init_coders(t_data *data)
{
	int		i;
	t_coder	*coder;

	i = -1;
	data->coders = malloc(data->number_of_coders * sizeof(t_coder));
	if (!data->coders)
		return (false);
	while (++i < data->number_of_coders)
	{
		coder = &data->coders[i];
		coder->id = i + 1;
		coder->heap_index = i;
		coder->compile_count = 0;
		coder->last_compile_time = get_time(MILLISECOND);
		coder->data_all = data;
		coder->status = INIT;
		coder->waiting_for = NULL;
		coder->finished = false;
		pthread_cond_init(&coder->cond, NULL);
		pthread_mutex_init(&coder->mutex, NULL);
		attribute_dongle(coder, i);
	}
}

bool	init_simulation(t_data *data)
{
	data->simulation_start = get_time(MILLISECOND);
	data->ready = false;
	data->end_of_simulation = false;
	data->coders_finished = 0;
	data->coders = malloc(data->number_of_coders * sizeof(t_coder));
	data->dongles = malloc(data->number_of_coders * sizeof(t_dongle));
	if (!data->coders || !data->dongles)
	{
		if (data->coders)
			free(data->coders);
		if (data->dongles)
			free(data->dongles);
		return (false);
	}
	init_coders(data);
	build_heap(data);
	init_dongle(data);
	pthread_mutex_init(&data->stdout_mutex, NULL);
	pthread_mutex_init(&data->end_mutex, NULL);
	pthread_mutex_init(&data->gate_mutex, NULL);
	pthread_mutex_init(&data->finished_mutex, NULL);
	pthread_cond_init(&data->gate_cond, NULL);
	pthread_cond_init(&data->monitor_cond, NULL);
	create_join(data);
	return (true);
}

void	init_data(t_data *data, char **argv)
{
	data->number_of_coders = atoi(argv[1]);
	data->time_to_burnout = atoi(argv[2]);
	data->time_to_compile = atoi(argv[3]);
	data->time_to_debug = atoi(argv[4]);
	data->time_to_refactor = atoi(argv[5]);
	data->number_of_compiles_required = atoi(argv[6]);
	data->dongle_cooldown = atoi(argv[7]);
	if (!strcmp(argv[8], "edf"))
		data->scheduler = EDF;
	else
		data->scheduler = FIFO;
}
