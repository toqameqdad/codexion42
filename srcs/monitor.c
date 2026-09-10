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

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	int				i;
	long			now;
	long			last_start;

	sim = (t_simulation *)arg;
	while (!simulation_should_stop(sim))
	{
		i = 0;
		while (i < sim->number_of_coders)
		{
			if (simulation_should_stop(sim))
				break ;
			now = get_current_time_ms();
			last_start = coder_get_last_compile_start(&sim->coders[i]);
			if (now - last_start > sim->time_to_burnout)
			{
				log_event(sim, sim->coders[i].id, "burned out");
				simulation_request_stop(sim);
				break ;
			}
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
