/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   scheduling_utils.c                                :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void	dongle_release(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->taken = false;
	dongle->last_release = get_time(MILLISECOND);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

bool	is_waiting(t_coder *coder, t_dongle *current)
{
	bool		ret;
	t_dongle	*requested;

	ret = false;
	pthread_mutex_lock(&coder->mutex);
	requested = coder->waiting_for;
	if (requested == current)
		ret = true;
	pthread_mutex_unlock(&coder->mutex);
	return (ret);
}

t_coder	*winner(long long right, long long left, t_dongle *dongle)
{
	if (right < left)
		return (dongle->right);
	else
		return (dongle->left);
	return (NULL);
}

bool	safe_if_end(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&coder->mutex);
	coder->waiting_for = NULL;
	pthread_mutex_unlock(&coder->mutex);
	pthread_mutex_unlock(&dongle->mutex);
	return (false);
}

t_coder	*priority(t_coder *coder, t_dongle *dongle)
{
	if (coder->data->scheduler == FIFO)
		return (first_arrived(dongle));
	else
		return (shortest_deadline(dongle));
	return (NULL);
}
