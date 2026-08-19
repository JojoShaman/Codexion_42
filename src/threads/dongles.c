/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   dongles.c.                                        :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */
#include "../../include/codexion.h"

void	init_dongle(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->number_of_coders)
	{
		data->dongles[i].id = i;
		data->dongles[i].last_release = 0;
		data->dongles[i].heap = data->heap;
		data->dongles[i].taken = false;
		pthread_cond_init(&data->dongles[i].cond, NULL);
		pthread_mutex_init(&data->dongles[i].mutex, NULL);
	}
}

void	attribute_dongle(t_coder *coder, int position)
{
	t_data	*data;
	int		n;

	data = coder->data_all;
	n = data->number_of_coders;
	if (position % 2)
	{
		coder->first_dongle = &data->dongles[(position + 1) % n];
		coder->second_dongle = &data->dongles[position];
	}
	else
	{
		coder->first_dongle = &data->dongles[position];
		coder->second_dongle = &data->dongles[(position + 1) % n];
	}
	data->dongles[position].left = &data->coders[position];
	data->dongles[position].right = &data->coders[(position - 1 + n) % n];
}
