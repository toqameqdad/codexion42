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
