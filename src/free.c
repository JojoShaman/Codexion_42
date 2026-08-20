/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   free.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */
#include "../include/codexion.h"

static void	destroy(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->number_of_coders)
	{
		pthread_cond_destroy(&data->coders[i].cond);
		pthread_mutex_destroy(&data->coders[i].mutex);
		pthread_mutex_destroy(&data->dongles[i].mutex);
		pthread_cond_destroy(&data->dongles[i].cond);
	}
}

void	cleanup(t_data *data)
{
	destroy(data);
	if (data->coders)
		free(data->coders);
	if (data->dongles)
		free(data->dongles);
	if (data->heap)
	{
		pthread_mutex_destroy(&data->heap->mutex);
		if (data->heap->node)
			free(data->heap->node);
		free(data->heap);
	}
	pthread_mutex_destroy(&data->gate_mutex);
	pthread_mutex_destroy(&data->finished_mutex);
	pthread_mutex_destroy(&data->end_mutex);
	pthread_mutex_destroy(&data->stdout_mutex);
	pthread_cond_destroy(&data->monitor_cond);
	pthread_cond_destroy(&data->gate_cond);
	if (data)
		free(data);
}
