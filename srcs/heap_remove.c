/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_remove.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 00:00:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/19 00:00:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap.h"

static void	restore_heap(t_heap *heap, int i)
{
	if (i > 0 && heap_node_is_less(heap->nodes[i],
			heap->nodes[(i - 1) / 2]))
		heap_sift_up(heap, i);
	else
		heap_sift_down(heap, i);
}

int	heap_remove_coder(t_heap *heap, int coder_id, t_heap_node *out)
{
	int	i;

	i = 0;
	while (i < heap->size && heap->nodes[i].coder_id != coder_id)
		i++;
	if (i == heap->size)
		return (1);
	*out = heap->nodes[i];
	heap->size--;
	if (i < heap->size)
	{
		heap->nodes[i] = heap->nodes[heap->size];
		restore_heap(heap, i);
	}
	return (0);
}
