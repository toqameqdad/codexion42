/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:10:15 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 02:10:15 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	select_dongle_order(t_coder *coder, t_dongle **first,
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

static void	mark_dongles_released(t_dongle *first, t_dongle *second,
			long released_at)
{
	first->in_use = 0;
	first->last_released_ms = released_at;
	if (first != second)
	{
		second->in_use = 0;
		second->last_released_ms = released_at;
		pthread_mutex_unlock(&second->lock);
	}
}

void	release_both_dongles(t_simulation *sim, t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long		released_at;

	select_dongle_order(coder, &first, &second);
	pthread_mutex_lock(&sim->queue_lock);
	pthread_mutex_lock(&first->lock);
	if (first != second)
		pthread_mutex_lock(&second->lock);
	released_at = get_current_time_ms();
	mark_dongles_released(first, second, released_at);
	pthread_mutex_unlock(&first->lock);
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_lock);
}
