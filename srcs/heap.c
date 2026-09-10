/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:33:10 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 02:33:10 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap.h"
#include <stdlib.h>

int	heap_init(t_heap *heap, int capacity)
{
	heap->nodes = malloc(sizeof(t_heap_node) * capacity);
	if (!heap->nodes)
		return (1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}

void	heap_destroy(t_heap *heap)
{
	free(heap->nodes);
	heap->nodes = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

int	heap_push(t_heap *heap, long priority, int coder_id)
{
	if (heap->size >= heap->capacity)
		return (1);
	heap->nodes[heap->size].priority = priority;
	heap->nodes[heap->size].coder_id = coder_id;
	heap_sift_up(heap, heap->size);
	heap->size++;
	return (0);
}

int	heap_pop(t_heap *heap, t_heap_node *out)
{
	if (heap_is_empty(heap))
		return (1);
	*out = heap->nodes[0];
	heap->size--;
	heap->nodes[0] = heap->nodes[heap->size];
	heap_sift_down(heap, 0);
	return (0);
}

void	heap_sift_down(t_heap *heap, int idx)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		smallest = idx;
		if (left < heap->size
			&& heap_node_is_less(heap->nodes[left], heap->nodes[smallest]))
			smallest = left;
		if (right < heap->size
			&& heap_node_is_less(heap->nodes[right], heap->nodes[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		heap_swap(&heap->nodes[idx], &heap->nodes[smallest]);
		idx = smallest;
	}
}
