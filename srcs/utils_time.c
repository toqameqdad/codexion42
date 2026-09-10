/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_time.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:46:02 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:46:02 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

long	elapsed_ms(t_simulation *sim)
{
	return (get_current_time_ms() - sim->start_time_ms);
}

void	ms_to_abs_timespec(long ms_from_now, struct timespec *ts)
{
	struct timeval	now;
	long			total_usec;

	if (ms_from_now < 0)
		ms_from_now = 0;
	gettimeofday(&now, NULL);
	total_usec = now.tv_usec + (ms_from_now % 1000) * 1000;
	ts->tv_sec = now.tv_sec + (ms_from_now / 1000) + (total_usec / 1000000);
	ts->tv_nsec = (total_usec % 1000000) * 1000;
}

void	sleep_ms_interruptible(t_simulation *sim, long ms)
{
	long	slept;
	long	chunk;
	long	this_sleep;

	slept = 0;
	chunk = 5;
	while (slept < ms && !simulation_should_stop(sim))
	{
		this_sleep = ms - slept;
		if (this_sleep > chunk)
			this_sleep = chunk;
		usleep(this_sleep * 1000);
		slept += this_sleep;
	}
}
