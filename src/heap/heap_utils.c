/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   heap_utils.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void	heapify(t_heap *arr, int i)
{
	t_heap_node	tmp;
	int			parent;
	int			child1;
	int			child2;
	int			size;

	parent = i;
	child1 = 2 * i + 1;
	child2 = 2 * i + 2;
	size = arr->size;
	if (child1 < size
		&& arr->node[child1].deadline < arr->node[parent].deadline)
		parent = child1;
	if (child2 < size
		&& arr->node[child2].deadline < arr->node[parent].deadline)
		parent = child2;
	if (parent != i)
	{
		tmp = arr->node[i];
		arr->node[i] = arr->node[parent];
		arr->node[parent] = tmp;
		arr->node[parent].coder->heap_index = parent;
		arr->node[i].coder->heap_index = i;
		heapify(arr, parent);
	}
}

void	sift_up(t_heap *heap, int i)
{
	int			parent;
	t_heap_node	tmp;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (heap->node[parent].deadline <= heap->node[i].deadline)
			break ;
		tmp = heap->node[i];
		heap->node[i] = heap->node[parent];
		heap->node[parent] = tmp;
		heap->node[i].coder->heap_index = i;
		heap->node[parent].coder->heap_index = parent;
		i = parent;
	}
}

void	remove_node(t_coder *coder)
{
	int		index;
	int		last;
	t_heap	*heap;

	pthread_mutex_lock(&coder->data->heap->mutex);
	heap = coder->data->heap;
	index = coder->heap_index;
	last = heap->size - 1;
	if (index != last)
	{
		heap->node[index] = heap->node[last];
		heap->node[index].coder->heap_index = index;
		heap->size--;
		sift_up(heap, index);
		heapify(heap, index);
	}
	else
		heap->size--;
	pthread_cond_signal(&coder->data->monitor_cond);
	pthread_mutex_unlock(&coder->data->heap->mutex);
}

void	update_deadline(t_coder *coder)
{
	int			time_to_burnout;
	long long	last_compile;
	long long	deadline_before;
	long long	deadline_after;
	t_heap		*heap;

	heap = NULL;
	last_compile = coder->last_compile_time;
	time_to_burnout = coder->data->time_to_burnout;
	deadline_before = 0;
	deadline_after = 0;
	pthread_mutex_lock(&coder->data->heap->mutex);
	heap = coder->data->heap;
	deadline_before = heap->node[0].deadline;
	heap->node[coder->heap_index].deadline = last_compile + time_to_burnout;
	sift_up(heap, coder->heap_index);
	heapify(heap, coder->heap_index);
	deadline_after = heap->node[0].deadline;
	if (deadline_after != deadline_before)
		pthread_cond_signal(&coder->data->monitor_cond);
	pthread_mutex_unlock(&coder->data->heap->mutex);
}
