/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_control.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 00:00:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/19 00:00:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup_shared_data(t_simulation *sim)
{
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->stop_lock);
	cleanup_dongles(sim, sim->number_of_coders);
	cleanup_coders(sim, sim->number_of_coders);
}

int	init_shared_locks(t_simulation *sim)
{
	if (pthread_mutex_init(&sim->print_lock, NULL) != 0)
	{
		cleanup_dongles(sim, sim->number_of_coders);
		cleanup_coders(sim, sim->number_of_coders);
		return (1);
	}
	if (pthread_mutex_init(&sim->stop_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->print_lock);
		cleanup_dongles(sim, sim->number_of_coders);
		cleanup_coders(sim, sim->number_of_coders);
		return (1);
	}
	return (0);
}

int	init_queue(t_simulation *sim)
{
	if (pthread_mutex_init(&sim->queue_lock, NULL) != 0)
		return (1);
	if (pthread_cond_init(&sim->queue_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->queue_lock);
		return (1);
	}
	return (0);
}

void	destroy_simulation(t_simulation *sim)
{
	cleanup_dongles(sim, sim->number_of_coders);
	cleanup_coders(sim, sim->number_of_coders);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->stop_lock);
	pthread_mutex_destroy(&sim->queue_lock);
	pthread_cond_destroy(&sim->queue_cond);
}
