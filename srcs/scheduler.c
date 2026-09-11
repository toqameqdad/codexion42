/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:09:37 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 02:09:37 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	compute_priority(t_simulation *sim, t_coder *coder,
			long request_time_ms)
{
	if (sim->scheduler == CX_SCHED_FIFO)
		return (request_time_ms);
	return (coder_get_last_compile_start(coder)
		+ sim->time_to_burnout);
}

void	scheduler_wait_for_turn(t_simulation *sim, t_coder *coder,
		long request_time_ms)
{
	long		priority;
	t_heap_node	front;

	priority = compute_priority(sim, coder, request_time_ms);
	pthread_mutex_lock(&sim->queue_lock);
	heap_push(&sim->wait_queue, priority, coder->id);
	heap_peek(&sim->wait_queue, &front);
	while (front.coder_id != coder->id && !simulation_should_stop(sim))
	{
		pthread_cond_wait(&sim->queue_cond, &sim->queue_lock);
		heap_peek(&sim->wait_queue, &front);
	}
	if (front.coder_id == coder->id)
		heap_pop(&sim->wait_queue, &front);
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_lock);
}
