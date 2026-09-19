/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_priority.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 00:00:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/20 00:02:24 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	shares_dongle(t_coder *first, t_coder *second)
{
	return (first->left_dongle == second->left_dongle
		|| first->left_dongle == second->right_dongle
		|| first->right_dongle == second->left_dongle
		|| first->right_dongle == second->right_dongle);
}

static int	dongles_ready(t_simulation *sim, t_coder *coder, long now)
{
	long	left_wait;
	long	right_wait;

	left_wait = now - coder->left_dongle->last_released_ms;
	right_wait = now - coder->right_dongle->last_released_ms;
	return (!coder->left_dongle->in_use && !coder->right_dongle->in_use
		&& (coder->left_dongle->last_released_ms == 0
			|| left_wait >= sim->dongle_cooldown)
		&& (coder->right_dongle->last_released_ms == 0
			|| right_wait >= sim->dongle_cooldown));
}

int	scheduler_request_may_run(t_simulation *sim, t_coder *coder,
			t_heap_node own, long now)
{
	t_heap_node	other;
	t_coder		*other_coder;
	int			i;

	i = 0;
	while (i < sim->scheduler_queue.size)
	{
		other = sim->scheduler_queue.nodes[i++];
		other_coder = &sim->coders[other.coder_id - 1];
		if (heap_node_is_less(other, own)
			&& shares_dongle(coder, other_coder))
		{
			if (sim->scheduler == CX_SCHED_EDF
				&& other.priority < own.priority)
				return (0);
			if (dongles_ready(sim, other_coder, now))
				return (0);
		}
	}
	return (1);
}
