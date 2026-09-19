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
		pthread_cond_timedwait(&sim->queue_cond, &sim->queue_lock, &ts);
	}
	else
		pthread_cond_wait(&sim->queue_cond, &sim->queue_lock);
}

static int	request_is_first(t_coder *coder, t_heap_node *front)
{
	t_heap_node	right_front;

	if (heap_peek(&coder->left_dongle->wait_queue, front) != 0)
		return (0);
	if (front->coder_id != coder->id)
		return (0);
	if (coder->left_dongle == coder->right_dongle)
		return (1);
	if (heap_peek(&coder->right_dongle->wait_queue,
			&right_front) != 0)
		return (0);
	if (right_front.coder_id != coder->id)
		return (0);
	return (1);
}

int	scheduler_reserve_ready(t_simulation *sim, t_coder *coder,
			t_heap_node *front)
{
	long	wait;
	int		reserved;

	wait = 0;
	reserved = 0;
	if (request_is_first(coder, front))
	{
		if (coder->left_dongle == coder->right_dongle)
			reserved = scheduler_try_single(sim, coder, &wait);
		else
			reserved = scheduler_try_pair(sim, coder, &wait);
		if (reserved)
			return (1);
	}
	wait_for_queue(sim, wait);
	return (0);
}
