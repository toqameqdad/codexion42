/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 17:30:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 17:30:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap.h"

int	heap_is_empty(t_heap *heap)
{
	return (heap->size == 0);
}

int	heap_node_is_less(t_heap_node a, t_heap_node b)
{
	if (a.priority != b.priority)
		return (a.priority < b.priority);
	if (a.sequence != b.sequence)
		return (a.sequence < b.sequence);
	return (a.coder_id < b.coder_id);
}

void	heap_swap(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

void	heap_sift_up(t_heap *heap, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!heap_node_is_less(heap->nodes[idx], heap->nodes[parent]))
			break ;
		heap_swap(&heap->nodes[idx], &heap->nodes[parent]);
		idx = parent;
	}
}

int	heap_peek(t_heap *heap, t_heap_node *out)
{
	if (heap_is_empty(heap))
		return (1);
	*out = heap->nodes[0];
	return (0);
}
