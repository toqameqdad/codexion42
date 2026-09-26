/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:09:37 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/25 23:50:33 by tmeqdad          ###   ########.fr       */
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

static int	queue_request(t_simulation *sim, t_coder *coder,
			long request_time)
{
	long	priority;
	long	sequence;

	sequence = sim->request_sequence++;
	priority = compute_priority(sim, coder, request_time);
	if (sim->scheduler == CX_SCHED_FIFO)
		priority = sequence;
	return (heap_push(&sim->scheduler_queue, priority,
			coder->id, sequence));
}

static int	wait_for_both(t_simulation *sim, t_coder *coder,
			t_dongle *first, t_dongle *second)
{
	while (!simulation_should_stop(sim))
	{
		if (scheduler_reserve_both(sim, coder, first, second))
		{
			pthread_mutex_unlock(&sim->queue_lock);
			log_event(sim, coder->id, "has taken a dongle");
			if (first != second)
				log_event(sim, coder->id, "has taken a dongle");
			return (0);
		}
	}
	pthread_mutex_unlock(&sim->queue_lock);
	return (1);
}

static void	set_dongle_order(t_coder *coder, t_dongle **first,
			t_dongle **second)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

int	scheduler_acquire_both(t_simulation *sim, t_coder *coder,
			long request_time)
{
	t_dongle	*first;
	t_dongle	*second;

	set_dongle_order(coder, &first, &second);
	pthread_mutex_lock(&sim->queue_lock);
	if (queue_request(sim, coder, request_time) != 0)
	{
		pthread_mutex_unlock(&sim->queue_lock);
		simulation_request_stop(sim);
		return (1);
	}
	if (wait_for_both(sim, coder, first, second) != 0)
		return (1);
	if (first == second)
	{
		while (!simulation_should_stop(sim))
			usleep(1000);
		return (1);
	}
	return (0);
}
