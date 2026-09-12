/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:45:54 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:45:54 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_threads(t_simulation *sim)
{
	int	i;

	if (pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0)
		return (1);
	sim->monitor_started = 1;
	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			simulation_request_stop(sim);
			return (1);
		}
		sim->created_coders++;
		i++;
	}
	return (0);
}

static void	join_threads(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->created_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	if (sim->monitor_started)
		pthread_join(sim->monitor_thread, NULL);
}

static int	run_simulation(t_simulation *sim)
{
	if (start_threads(sim) != 0)
	{
		fprintf(stderr, "Error: thread creation failed\n");
		simulation_request_stop(sim);
		join_threads(sim);
		destroy_simulation(sim);
		return (1);
	}
	join_threads(sim);
	destroy_simulation(sim);
	return (0);
}

static void	set_coder_start_times(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		coder_set_last_compile_start(&sim->coders[i],
			sim->start_time_ms);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_simulation	sim;

	memset(&sim, 0, sizeof(t_simulation));
	if (parse_args(argc, argv, &sim) != 0)
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	if (init_simulation(&sim) != 0)
	{
		fprintf(stderr, "Error: initialization failed\n");
		return (1);
	}
	sim.start_time_ms = get_current_time_ms();
	set_coder_start_times(&sim);
	return (run_simulation(&sim));
}
