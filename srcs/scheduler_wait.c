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

static long	dongle_wait(t_simulation *sim, t_dongle *dongle, long now)
{
	long	wait;

	if (dongle->in_use || dongle->last_released_ms == 0)
		return (0);
	wait = sim->dongle_cooldown - (now - dongle->last_released_ms);
	if (wait < 0)
		wait = 0;
	return (wait);
}

static int	coder_ready(t_simulation *sim, t_coder *coder, long now)
{
	return (!coder->left_dongle->in_use && !coder->right_dongle->in_use
		&& dongle_wait(sim, coder->left_dongle, now) == 0
		&& dongle_wait(sim, coder->right_dongle, now) == 0);
}

static int	find_request(t_simulation *sim, int coder_id, t_heap_node *own)
{
	int	i;

	i = 0;
	while (i < sim->scheduler_queue.size)
	{
		if (sim->scheduler_queue.nodes[i].coder_id == coder_id)
		{
			*own = sim->scheduler_queue.nodes[i];
			return (0);
		}
		i++;
	}
	return (1);
}

static void	wait_for_change(t_simulation *sim, long wait)
{
	struct timespec	ts;

	if (wait > 0)
	{
		ms_to_abs_timespec(wait, &ts);
		pthread_cond_timedwait(&sim->queue_cond, &sim->queue_lock, &ts);
	}
	else
		pthread_cond_wait(&sim->queue_cond, &sim->queue_lock);
}

int	scheduler_reserve_both(t_simulation *sim, t_coder *coder,
			t_dongle *first, t_dongle *second)
{
	t_heap_node	own;
	long		wait;
	long		now;

	wait = 0;
	now = get_current_time_ms();
	if (find_request(sim, coder->id, &own) == 0
		&& coder_ready(sim, coder, now)
		&& scheduler_request_may_run(sim, coder, own, now)
		&& scheduler_try_both(sim, first, second, &wait))
	{
		heap_remove_coder(&sim->scheduler_queue, coder->id, &own);
		pthread_cond_broadcast(&sim->queue_cond);
		return (1);
	}
	wait = dongle_wait(sim, first, now);
	if (dongle_wait(sim, second, now) > wait)
		wait = dongle_wait(sim, second, now);
	wait_for_change(sim, wait);
	return (0);
}
