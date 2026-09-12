/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:06:44 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 02:06:44 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEAP_H
# define HEAP_H

typedef struct s_heap_node
{
	long	priority;
	int		coder_id;
	long	sequence;
}	t_heap_node;

typedef struct s_heap
{
	t_heap_node	*nodes;
	int			size;
	int			capacity;
}	t_heap;

int		heap_init(t_heap *heap, int capacity);
void	heap_destroy(t_heap *heap);
int		heap_push(t_heap *heap, long priority, int coder_id, long sequence);
int		heap_pop(t_heap *heap, t_heap_node *out);
int		heap_peek(t_heap *heap, t_heap_node *out);
int		heap_is_empty(t_heap *heap);
int		heap_node_is_less(t_heap_node a, t_heap_node b);
void	heap_swap(t_heap_node *a, t_heap_node *b);
void	heap_sift_up(t_heap *heap, int idx);
void	heap_sift_down(t_heap *heap, int idx);

#endif
