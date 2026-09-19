/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 02:10:54 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 02:10:54 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	do_one_compile_round(t_simulation *sim, t_coder *coder)
{
	long	request_time;

	request_time = elapsed_ms(sim);
	if (scheduler_acquire_both(sim, coder, request_time) != 0)
		return (1);
	coder_set_last_compile_start(coder, get_current_time_ms());
	log_event(sim, coder->id, "is compiling");
	sleep_ms_interruptible(sim, sim->time_to_compile);
	release_both_dongles(sim, coder);
	if (simulation_should_stop(sim))
		return (1);
	coder_inc_compiles_done(coder);
	return (0);
}

static int	coder_completed_work(t_simulation *sim, t_coder *coder)
{
	if (coder_get_compiles_done(coder) < sim->number_of_compiles_required)
		return (0);
	coder_set_finished(coder, 1);
	simulation_mark_finished(sim);
	return (1);
}

static void	coder_rest(t_simulation *sim, t_coder *coder)
{
	log_event(sim, coder->id, "is debugging");
	sleep_ms_interruptible(sim, sim->time_to_debug);
	if (simulation_should_stop(sim))
		return ;
	log_event(sim, coder->id, "is refactoring");
	sleep_ms_interruptible(sim, sim->time_to_refactor);
}

void	*coder_routine(void *arg)
{
	t_coder			*coder;
	t_simulation	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	if (coder_completed_work(sim, coder))
		return (NULL);
	while (!simulation_should_stop(sim)
		&& coder_get_compiles_done(coder) < sim->number_of_compiles_required)
	{
		if (do_one_compile_round(sim, coder) != 0)
			break ;
		if (coder_completed_work(sim, coder))
			break ;
		coder_rest(sim, coder);
	}
	return (NULL);
}
