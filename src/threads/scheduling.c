#include "../../include/codexion.h"

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
}

void	dongle_release(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);
    dongle->taken = false;
    dongle->last_release = get_time(MILLISECOND);
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}

static t_coder *first_arrived(t_dongle *dongle)
{
	long long	left_arrival;
	long long	right_arrival;
	bool		left_wait;
	bool		right_wait;
	t_coder		*first;

	pthread_mutex_lock(&dongle->left->mutex);
	left_wait = (dongle->left->waiting_for == dongle);
	pthread_mutex_unlock(&dongle->left->mutex);
	pthread_mutex_lock(&dongle->right->mutex);
	right_wait = (dongle->right->waiting_for == dongle);
	pthread_mutex_unlock(&dongle->right->mutex);
	left_arrival = get_long(
		&dongle->left->mutex, &dongle->left->arrival_time);
	right_arrival = get_long(
		&dongle->right->mutex, &dongle->right->arrival_time);
	if (left_wait && right_wait)
	{
		if (left_arrival > right_arrival)
			first = dongle->right;
		else
			first = dongle->left;
	}
	else if (left_wait)
		first = dongle->left;
	else
		first = dongle->right;
	return (first);
}

static t_coder	*shortest_deadline(t_dongle *dongle)
{
	long long	right_deadline;
	long long	left_deadline;
	bool		left_wait;
	bool		right_wait;

	pthread_mutex_lock(&dongle->right->mutex);
	right_wait = (dongle->right->waiting_for == dongle);
	pthread_mutex_unlock(&dongle->right->mutex);
	pthread_mutex_lock(&dongle->left->mutex);
	left_wait = (dongle->left->waiting_for == dongle);
	pthread_mutex_unlock(&dongle->left->mutex);
	pthread_mutex_lock(&dongle->heap->mutex);
	right_deadline = dongle->heap->node[dongle->right->heap_index].deadline;
	left_deadline = dongle->heap->node[dongle->left->heap_index].deadline;
	pthread_mutex_unlock(&dongle->heap->mutex);
	if (right_wait && left_wait)
	{
		if (right_deadline < left_deadline)
			return (dongle->right);
		else
			return (dongle->left);
	}
	else if (right_wait && !left_wait)
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
	while (dongle->taken && !is_end(coder->data_all))
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	if (is_end(coder->data_all))
	{
		pthread_mutex_lock(&coder->mutex);
		coder->waiting_for = NULL;
		pthread_mutex_unlock(&coder->mutex);
		pthread_mutex_unlock(&dongle->mutex);
		return (false);
	}
	if (get_time(MILLISECOND) < dongle->last_release +
			coder->data_all->dongle_cooldown)
		dongle_cooldown(dongle, coder->data_all);
	if (coder->data_all->scheduler == FIFO)
	{
		while (coder != first_arrived(dongle) && !is_end(coder->data_all))
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	else
	{
		while (coder != shortest_deadline(dongle) && !is_end(coder->data_all))
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	if (is_end(coder->data_all))
	{
		pthread_mutex_lock(&coder->mutex);
		coder->waiting_for = NULL;
		pthread_mutex_unlock(&coder->mutex);
		pthread_mutex_unlock(&dongle->mutex);
		return (false);
	}
	dongle->taken = true;
	pthread_mutex_lock(&coder->mutex);
	coder->waiting_for = NULL;
	pthread_mutex_unlock(&coder->mutex);
	pthread_mutex_unlock(&dongle->mutex);
	return (true);
}