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

static void	init_data_mutex(t_data *data)
{
	pthread_mutex_init(&data->write_mutex, NULL);
	pthread_mutex_init(&data->end_mutex, NULL);
	pthread_mutex_init(&data->ready_mutex, NULL);
	pthread_mutex_init(&data->another_mutex, NULL);
	pthread_cond_init(&data->ready_cond, NULL);
	pthread_cond_init(&data->monitor_cond, NULL);
}

static void	init_constant(t_data *data, char **argv)
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
	data->scheduler = FIFO;
}

bool	init_data(t_data *data, char **argv)
{
	init_constant(data, argv);
	data->simulation_start = get_time(MILLISECOND);
	data->ready = false;
	data->end_of_simulation = false;
	data->coders_finished = 0;
	data->coders = malloc(data->number_of_coders * sizeof(*data->coders));
	if (!data->coders)
		return (0);
	data->dongles = malloc(data->number_of_coders * sizeof((*data->dongles)));
	if (!data->dongles)
	{
		free(data->coders);
		return (0);
	}
	init_coders(data);
	build_heap(data);
	init_dongle(data);
	init_data_mutex(data);
	return (1);
}
