/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_dongle.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 00:00:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/19 00:00:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	lock_dongle_pair(t_coder *coder, t_dongle **first,
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
	pthread_mutex_lock(&(*first)->lock);
	if (*first != *second)
		pthread_mutex_lock(&(*second)->lock);
}

static void	unlock_dongle_pair(t_dongle *first, t_dongle *second)
{
	if (first != second)
		pthread_mutex_unlock(&second->lock);
	pthread_mutex_unlock(&first->lock);
}

static long	cooldown_left(t_simulation *sim, t_dongle *dongle, long now)
{
	long	left;

	if (dongle->last_released_ms == 0)
		return (0);
	left = sim->dongle_cooldown - (now - dongle->last_released_ms);
	if (left < 0)
		left = 0;
	return (left);
}

int	scheduler_try_pair(t_simulation *sim, t_coder *coder, long *wait)
{
	t_dongle	*first;
	t_dongle	*second;
	long		left_first;
	long		left_second;
	long		now;

	lock_dongle_pair(coder, &first, &second);
	now = get_current_time_ms();
	left_first = cooldown_left(sim, first, now);
	left_second = cooldown_left(sim, second, now);
	*wait = left_first;
	if (left_second > *wait)
		*wait = left_second;
	if (!first->in_use && !second->in_use && *wait == 0)
	{
		first->in_use = 1;
		second->in_use = 1;
		unlock_dongle_pair(first, second);
		return (1);
	}
	unlock_dongle_pair(first, second);
	return (0);
}

int	scheduler_try_single(t_simulation *sim, t_coder *coder, long *wait)
{
	t_dongle	*dongle;
	int			available;

	dongle = coder->left_dongle;
	pthread_mutex_lock(&dongle->lock);
	*wait = cooldown_left(sim, dongle, get_current_time_ms());
	available = !dongle->in_use && *wait == 0;
	if (available)
		dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->lock);
	return (available);
}
