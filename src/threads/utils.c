#include "../../include/codexion.h"

bool    reached_compile_target(t_coder *coder)
{
	bool	ret;

	ret = false;
	pthread_mutex_lock(&coder->mutex);
	if (coder->finished)
		ret = true;
	pthread_mutex_unlock(&coder->mutex);
	return (ret);
}

long long	get_long(t_mtx *mutex, long long *value)
{
	long long   ret;

	pthread_mutex_lock(mutex);
	ret = *value;
	pthread_mutex_unlock(mutex);
	return (ret);
}

bool	is_end(t_data *data)
{
	bool	is_end;
	is_end = false;
	pthread_mutex_lock(&data->end_mutex);
	if (data->end_of_simulation)
		is_end = true;
	pthread_mutex_unlock(&data->end_mutex);
	return (is_end);
}

void	set_status(t_coder *coder, t_status log, bool display)
{
	pthread_mutex_lock(&coder->mutex);
	coder->status = log;
	if (display)
		output(coder, log);
	pthread_mutex_unlock(&coder->mutex);
}

void	set_end(t_data *data)
{
	int	i;

	i = -1;
	pthread_mutex_lock(&data->end_mutex);
	data->end_of_simulation = true;
	pthread_mutex_unlock(&data->end_mutex);
	while (++i < data->number_of_coders)
	{
		pthread_cond_broadcast(&data->coders[i].cond);
		pthread_mutex_lock(&data->dongles[i].mutex);
		pthread_cond_broadcast(&data->dongles[i].cond);
		pthread_mutex_unlock(&data->dongles[i].mutex);
	}
}