/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:09:37 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/16 17:47:00 by tmeqdad          ###   ########.fr       */
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

static int	push_request(t_simulation *sim, t_coder *coder,
			long priority, long sequence)
{
	return (heap_push(&sim->wait_queue, priority, coder->id, sequence));
}

void	scheduler_wait_for_turn(t_simulation *sim, t_coder *coder,
			long request_time_ms)
{
	long		priority;
	long		sequence;
	t_heap_node	front;

	pthread_mutex_lock(&sim->queue_lock);
	sequence = sim->request_sequence++;
	if (sim->scheduler == CX_SCHED_FIFO)
		priority = sequence;
	else
		priority = compute_priority(sim, coder, request_time_ms);
	if (push_request(sim, coder, priority, sequence) != 0)
	{
		pthread_mutex_unlock(&sim->queue_lock);
		simulation_request_stop(sim);
		return ;
	}
	while (!simulation_should_stop(sim))
	{
		if (heap_peek(&sim->wait_queue, &front) != 0)
			break ;
		if (front.coder_id == coder->id)
		{
			heap_pop(&sim->wait_queue, &front);
			pthread_cond_broadcast(&sim->queue_cond);
			pthread_mutex_unlock(&sim->queue_lock);
			return ;
		}
		pthread_cond_wait(&sim->queue_cond, &sim->queue_lock);
	}
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_lock);
}
