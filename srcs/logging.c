/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:45:51 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:45:51 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_event(t_simulation *sim, int coder_id, const char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->print_lock);
	if (simulation_should_stop(sim) && strcmp(msg, "burned out") != 0)
	{
		pthread_mutex_unlock(&sim->print_lock);
		return ;
	}
	ts = elapsed_ms(sim);
	printf("%ld %d %s\n", ts, coder_id, msg);
	pthread_mutex_unlock(&sim->print_lock);
}

int	simulation_should_stop(t_simulation *sim)
{
	int	value;

	pthread_mutex_lock(&sim->stop_lock);
	value = sim->stop_flag;
	pthread_mutex_unlock(&sim->stop_lock);
	return (value);
}

static void	wake_up_all_dongles(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
}

void	simulation_request_stop(t_simulation *sim)
{
	pthread_mutex_lock(&sim->stop_lock);
	sim->stop_flag = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	pthread_mutex_lock(&sim->queue_lock);
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_lock);
	wake_up_all_dongles(sim);
}

void	simulation_mark_finished(t_simulation *sim)
{
	int	should_stop_now;

	should_stop_now = 0;
	pthread_mutex_lock(&sim->stop_lock);
	sim->finished_count++;
	if (sim->finished_count >= sim->number_of_coders)
		should_stop_now = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	if (should_stop_now)
		simulation_request_stop(sim);
}
