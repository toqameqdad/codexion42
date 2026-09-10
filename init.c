/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:45:46 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:45:46 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_dongles(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	free(sim->dongles);
}

int	init_simulation(t_simulation *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!sim->dongles || !sim->coders)
	{
		free(sim->dongles);
		free(sim->coders);
		return (1);
	}
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].last_released_ms = 0;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
		{
			destroy_dongles(sim, i);
			free(sim->coders);
			return (1);
		}
		i++;
	}
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].last_compile_start_ms = 0;
		sim->coders[i].is_compiling = 0;
		sim->coders[i].sim = sim;
		sim->coders[i].right_dongle = &sim->dongles[i];
		sim->coders[i].left_dongle =
			&sim->dongles[(i + sim->number_of_coders - 1) % sim->number_of_coders];
		i++;
	}
	pthread_mutex_init(&sim->print_lock, NULL);
	pthread_mutex_init(&sim->stop_lock, NULL);
	sim->stop_flag = 0;
	return (0);
}

void	destroy_simulation(t_simulation *sim)
{
	destroy_dongles(sim, sim->number_of_coders);
	free(sim->coders);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->stop_lock);
}