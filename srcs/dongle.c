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

int	dongle_acquire(t_simulation *sim, t_dongle *d)
{
	long			now;
	long			remaining;
	struct timespec	ts;

	pthread_mutex_lock(&d->lock);
	while (1)
	{
		if (simulation_should_stop(sim))
		{
			pthread_mutex_unlock(&d->lock);
			return (1);
		}
		now = get_current_time_ms();
		if (!d->in_use && (d->last_released_ms == 0
				|| now - d->last_released_ms >= sim->dongle_cooldown))
			break ;
		if (d->in_use)
			pthread_cond_wait(&d->cond, &d->lock);
		else
		{
			remaining = sim->dongle_cooldown - (now - d->last_released_ms);
			ms_to_abs_timespec(remaining, &ts);
			pthread_cond_timedwait(&d->cond, &d->lock, &ts);
		}
	}
	d->in_use = 1;
	pthread_mutex_unlock(&d->lock);
	return (0);
}

void	dongle_release(t_simulation *sim, t_dongle *d)
{
	(void)sim;
	pthread_mutex_lock(&d->lock);
	d->in_use = 0;
	d->last_released_ms = get_current_time_ms();
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}

int	acquire_both_dongles(t_simulation *sim, t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	if (dongle_acquire(sim, first) != 0)
		return (1);
	log_event(sim, coder->id, "has taken a dongle");
	if (dongle_acquire(sim, second) != 0)
	{
		dongle_release(sim, first);
		return (1);
	}
	log_event(sim, coder->id, "has taken a dongle");
	return (0);
}

void	release_both_dongles(t_simulation *sim, t_coder *coder)
{
	dongle_release(sim, coder->left_dongle);
	dongle_release(sim, coder->right_dongle);
}
