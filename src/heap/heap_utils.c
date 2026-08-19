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
