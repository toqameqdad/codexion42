/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:09:37 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/19 00:00:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	compute_priority(t_simulation *sim, t_coder *coder,
			long request_time_ms)
{
	if (sim->scheduler == CX_SCHED_FIFO)
		return (request_time_ms);
	return (coder_get_last_compile_start(coder) + sim->time_to_burnout);
}

static int	enqueue_request(t_simulation *sim, t_coder *coder,
			long request_time, long sequence)
{
	t_dongle	*left;
	t_dongle	*right;
	long		priority;

	left = coder->left_dongle;
	right = coder->right_dongle;
	priority = compute_priority(sim, coder, request_time);
	if (sim->scheduler == CX_SCHED_FIFO)
		priority = sequence;
	if (left->wait_queue.size >= left->wait_queue.capacity)
		return (1);
	if (right != left
		&& right->wait_queue.size >= right->wait_queue.capacity)
		return (1);
	if (heap_push(&left->wait_queue, priority, coder->id, sequence) != 0)
		return (1);
	if (right != left
		&& heap_push(&right->wait_queue, priority,
			coder->id, sequence) != 0)
		return (1);
	return (0);
}

static int	finish_reservation(t_simulation *sim, t_coder *coder,
			t_heap_node *front)
{
	int	single;

	single = (coder->left_dongle == coder->right_dongle);
	heap_pop(&coder->left_dongle->wait_queue, front);
	if (!single)
		heap_pop(&coder->right_dongle->wait_queue, front);
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_lock);
	log_event(sim, coder->id, "has taken a dongle");
	if (single)
	{
		while (!simulation_should_stop(sim))
			usleep(1000);
		return (1);
	}
	log_event(sim, coder->id, "has taken a dongle");
	return (0);
}

int	scheduler_acquire_both(t_simulation *sim, t_coder *coder,
			long request_time_ms)
{
	long		sequence;
	t_heap_node	front;

	pthread_mutex_lock(&sim->queue_lock);
	sequence = sim->request_sequence++;
	if (enqueue_request(sim, coder, request_time_ms, sequence) != 0)
	{
		pthread_mutex_unlock(&sim->queue_lock);
		simulation_request_stop(sim);
		return (1);
	}
	while (!simulation_should_stop(sim))
	{
		if (scheduler_reserve_ready(sim, coder, &front))
			return (finish_reservation(sim, coder, &front));
	}
	pthread_mutex_unlock(&sim->queue_lock);
	return (1);
}
