/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_wait.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 00:00:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/19 00:00:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wait_for_queue(t_simulation *sim, long wait)
{
	struct timespec	ts;

	if (wait > 0)
	{
		ms_to_abs_timespec(wait, &ts);
		pthread_cond_timedwait(&sim->queue_cond,
			&sim->queue_lock, &ts);
	}
	else
		pthread_cond_wait(&sim->queue_cond, &sim->queue_lock);
}

static int	request_is_first(t_dongle *dongle, int coder_id)
{
	t_heap_node	front;

	if (heap_peek(&dongle->wait_queue, &front) != 0)
		return (0);
	return (front.coder_id == coder_id);
}

int	scheduler_reserve_dongle(t_simulation *sim, t_coder *coder,
			t_dongle *dongle)
{
	t_heap_node	front;
	long		wait;

	wait = 0;
	if (request_is_first(dongle, coder->id))
	{
		if (scheduler_try_dongle(sim, dongle, &wait))
		{
			heap_pop(&dongle->wait_queue, &front);
			pthread_cond_broadcast(&sim->queue_cond);
			return (1);
		}
	}
	wait_for_queue(sim, wait);
	return (0);
}
