/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   scheduling.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void	attribute_dongle(t_coder *coder, int position)
{
	t_data	*data;
	int		n;

	data = coder->data;
	n = data->number_of_coders;
	if (position % 2)
	{
		coder->first_dongle = &data->dongles[(position + 1) % n];
		coder->second_dongle = &data->dongles[position];
		return ;
	}
	coder->first_dongle = &data->dongles[position];
	coder->second_dongle = &data->dongles[(position + 1) % n];
}

t_coder	*first_arrived(t_dongle *dongle)
{
	long long	left_arrival;
	long long	right_arrival;
	bool		left_wait;
	bool		right_wait;

	right_wait = is_waiting(&dongle->right->mutex,
			dongle->right->waiting_for, dongle);
	left_wait = is_waiting(&dongle->left->mutex,
			dongle->left->waiting_for, dongle);
	left_arrival = get_long(
			&dongle->left->mutex, &dongle->left->arrival_time);
	right_arrival = get_long(
			&dongle->right->mutex, &dongle->right->arrival_time);
	if (right_wait && left_wait)
		return (winner(right_arrival, left_arrival, dongle));
	else if (left_wait)
		return (dongle->left);
	else
		return (dongle->right);
	return (NULL);
}

t_coder	*shortest_deadline(t_dongle *dongle)
{
	long long	right_deadline;
	long long	left_deadline;
	bool		left_wait;
	bool		right_wait;

	right_wait = is_waiting(&dongle->right->mutex,
			dongle->right->waiting_for, dongle);
	left_wait = is_waiting(&dongle->left->mutex,
			dongle->left->waiting_for, dongle);
	pthread_mutex_lock(&dongle->heap->mutex);
	right_deadline = dongle->heap->node[dongle->right->heap_index].deadline;
	left_deadline = dongle->heap->node[dongle->left->heap_index].deadline;
	pthread_mutex_unlock(&dongle->heap->mutex);
	if (right_wait && left_wait)
		return (winner(right_deadline, left_deadline, dongle));
	else if (right_wait)
		return (dongle->right);
	return (dongle->left);
}

bool	dongle_acquire(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	pthread_mutex_lock(&coder->mutex);
	coder->waiting_for = dongle;
	pthread_mutex_unlock(&coder->mutex);
	set_status(coder, WAITING_DONGLE, false);
	while (dongle->taken && !is_end(coder->data))
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	if (is_end(coder->data))
		return (safe_if_end(coder, dongle));
	if (get_time(MILLISECOND) < dongle->last_release
		+ coder->data->dongle_cooldown)
		dongle_cooldown(dongle, coder->data);
	while (coder != priority(coder, dongle) && !is_end(coder->data))
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	if (is_end(coder->data))
		return (safe_if_end(coder, dongle));
	dongle->taken = true;
	pthread_mutex_lock(&coder->mutex);
	coder->waiting_for = NULL;
	pthread_mutex_unlock(&coder->mutex);
	pthread_mutex_unlock(&dongle->mutex);
	return (true);
}
