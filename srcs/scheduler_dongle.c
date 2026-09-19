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

static long	cooldown_left(t_simulation *sim, t_dongle *dongle,
			long now)
{
	long	left;

	if (dongle->last_released_ms == 0)
		return (0);
	left = sim->dongle_cooldown - (now - dongle->last_released_ms);
	if (left < 0)
		left = 0;
	return (left);
}

int	scheduler_try_dongle(t_simulation *sim, t_dongle *dongle,
			long *wait)
{
	int	available;

	pthread_mutex_lock(&dongle->lock);
	*wait = cooldown_left(sim, dongle, get_current_time_ms());
	available = !dongle->in_use && *wait == 0;
	if (available)
		dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->lock);
	return (available);
}

int	scheduler_try_both(t_simulation *sim, t_dongle *first,
			t_dongle *second, long *wait)
{
	long	first_wait;
	long	second_wait;
	int		available;

	if (first == second)
		return (scheduler_try_dongle(sim, first, wait));
	pthread_mutex_lock(&first->lock);
	pthread_mutex_lock(&second->lock);
	first_wait = cooldown_left(sim, first, get_current_time_ms());
	second_wait = cooldown_left(sim, second, get_current_time_ms());
	*wait = first_wait;
	if (second_wait > *wait)
		*wait = second_wait;
	available = !first->in_use && !second->in_use && *wait == 0;
	if (available)
	{
		first->in_use = 1;
		second->in_use = 1;
	}
	pthread_mutex_unlock(&second->lock);
	pthread_mutex_unlock(&first->lock);
	return (available);
}
