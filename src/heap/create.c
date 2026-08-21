/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   create.c                                          :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

static bool	init_heap(t_data *data)
{
	data->heap = malloc(sizeof(*data->heap));
	if (!data->heap)
		return (false);
	data->heap->node = malloc(
			data->number_of_coders * sizeof(*data->heap->node));
	if (!data->heap->node)
	{
		free(data->heap);
		data->heap = NULL;
		return (false);
	}
	data->heap->size = data->number_of_coders;
	return (true);
}

bool	build_heap(t_data *data)
{
	int	start;
	int	i;

	start = (data->number_of_coders / 2) - 1;
	i = -1;
	if (!init_heap(data))
		return (false);
	pthread_mutex_init(&data->heap->mutex, NULL);
	while (++i < data->number_of_coders)
	{
		data->heap->node[i].coder = &data->coders[i];
		data->heap->node[i].deadline = (data->simulation_start
				+ data->time_to_burnout);
	}
	i = start + 1;
	while (--i >= 0)
		heapify(data->heap, i);
	return (true);
}
