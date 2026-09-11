/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:11:22 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 02:11:22 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_coder(t_simulation *sim, t_coder *coder)
{
	long	now;
	long	last_start;

	now = get_current_time_ms();
	last_start = coder_get_last_compile_start(coder);
	if (now - last_start >= sim->time_to_burnout)
	{
		log_event(sim, coder->id, "burned out");
		simulation_request_stop(sim);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	int				i;

	sim = (t_simulation *)arg;
	while (!simulation_should_stop(sim))
	{
		i = 0;
		while (i < sim->number_of_coders)
		{
			if (simulation_should_stop(sim))
				break ;
			if (check_coder(sim, &sim->coders[i]))
				break ;
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
