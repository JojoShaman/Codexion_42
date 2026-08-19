/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   coders.c                                          :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */
#include "../../include/codexion.h"

void	init_coders(t_data *data)
{
	int		i;
	t_coder	*coder;

	i = -1;
	while (++i < data->number_of_coders)
	{
		coder = &data->coders[i];
		coder->id = i + 1;
		coder->heap_index = i;
		coder->compile_count = 0;
		coder->last_compile_time = get_time(MILLISECOND);
		pthread_cond_init(&coder->cond, NULL);
		pthread_mutex_init(&coder->mutex, NULL);
		pthread_mutex_init(&coder->status_mutex, NULL);
		pthread_mutex_init(&coder->read_long_mutex, NULL);
		coder->data_all = data;
		coder->status = WAITING_DONGLE;
		coder->waiting_for = NULL;
		coder->finished = false;
		attribute_dongle(coder, i);
	}
}
