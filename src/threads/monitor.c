/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   monitor.c                                         :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

static void	burned_out(t_data *data, t_heap *heap)
{
	set_end(data);
	output(heap->node[0].coder, BURNED_OUT);
}

void	*monitoring(void *all_data)
{
	t_data			*data;
	struct timespec	ts;
	int				rt;

	data = (t_data *)all_data;
	while (!is_end(data))
	{
		pthread_mutex_lock(&data->heap->mutex);
		if (data->heap->size == 0)
		{
			pthread_mutex_unlock(&data->heap->mutex);
			break ;
		}
		ts.tv_sec = data->heap->node[0].deadline / 1000;
		ts.tv_nsec = (data->heap->node[0].deadline % 1000) * 1000000;
		rt = pthread_cond_timedwait(&data->monitor_cond,
				&data->heap->mutex, &ts);
		if (rt == ETIMEDOUT && !is_end(data)
			&& !reached_compile_target(data->heap->node[0].coder))
			burned_out(data, data->heap);
		pthread_mutex_unlock(&data->heap->mutex);
	}
	return (NULL);
}
