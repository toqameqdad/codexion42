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
		pthread_cond_destroy(&sim->dongles[i].cond);
		i++;
	}
	free(sim->dongles);
}

static void	destroy_coders(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->coders[i].state_lock);
		i++;
	}
	free(sim->coders);
}

static int	init_dongles(t_simulation *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].last_released_ms = 0;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
		{
			destroy_dongles(sim, i);
			return (1);
		}
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&sim->dongles[i].lock);
			destroy_dongles(sim, i);
			return (1);
		}
		i++;
	}
	return (0);
}

static int	init_coders(t_simulation *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!sim->coders)
		return (1);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].last_compile_start_ms = 0;
		sim->coders[i].sim = sim;
		sim->coders[i].right_dongle = &sim->dongles[i];
		sim->coders[i].left_dongle
			= &sim->dongles[(i + sim->number_of_coders - 1)
			% sim->number_of_coders];
		if (pthread_mutex_init(&sim->coders[i].state_lock, NULL) != 0)
		{
			destroy_coders(sim, i);
			return (1);
		}
		i++;
	}
	return (0);
}

int	init_simulation(t_simulation *sim)
{
	if (init_dongles(sim) != 0)
		return (1);
	if (init_coders(sim) != 0)
	{
		destroy_dongles(sim, sim->number_of_coders);
		return (1);
	}
	pthread_mutex_init(&sim->print_lock, NULL);
	pthread_mutex_init(&sim->stop_lock, NULL);
	sim->stop_flag = 0;
	sim->finished_count = 0;
	if (heap_init(&sim->wait_queue, sim->number_of_coders) != 0)
	{
		destroy_dongles(sim, sim->number_of_coders);
		destroy_coders(sim, sim->number_of_coders);
		return (1);
	}
	pthread_mutex_init(&sim->queue_lock, NULL);
	pthread_cond_init(&sim->queue_cond, NULL);
	return (0);
}

void	destroy_simulation(t_simulation *sim)
{
	destroy_dongles(sim, sim->number_of_coders);
	destroy_coders(sim, sim->number_of_coders);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->stop_lock);
	heap_destroy(&sim->wait_queue);
	pthread_mutex_destroy(&sim->queue_lock);
	pthread_cond_destroy(&sim->queue_cond);
}
