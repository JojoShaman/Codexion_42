#include "../../include/codexion.h"

void	*monitoring(void *all_data)
{
	t_heap          *heap;
	t_data          *data;
	struct timespec ts;
	int             rt;

	heap = NULL;
	data = (t_data *)all_data;
	heap = data->heap;
	while(!is_end(data))
	{
		pthread_mutex_lock(&data->heap->mutex);
		ts.tv_sec = heap->node[0].deadline / 1000;
		ts.tv_nsec = (heap->node[0].deadline % 1000) * 1000000;
		rt = pthread_cond_timedwait(&data->monitor_cond, &data->heap->mutex, &ts);
		if (rt == ETIMEDOUT && !is_end(data)
			&& !reached_compile_target(heap->node[0].coder))
		{
			set_end(data);
			output(heap->node[0].coder, BURNED_OUT);
		}
		pthread_mutex_unlock(&data->heap->mutex);
	}
	return (NULL);
}