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
	scheduler_wait_for_turn(sim, coder, request_time);
	if (simulation_should_stop(sim))
		return (1);
	if (acquire_both_dongles(sim, coder) != 0)
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

void	*coder_routine(void *arg)
{
	t_coder			*coder;
	t_simulation	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (!simulation_should_stop(sim)
		&& coder_get_compiles_done(coder) < sim->number_of_compiles_required)
	{
		if (do_one_compile_round(sim, coder) != 0)
			break ;
		if (coder_get_compiles_done(coder) >= sim->number_of_compiles_required)
		{
			simulation_mark_finished(sim);
			break ;
		}
		log_event(sim, coder->id, "is debugging");
		sleep_ms_interruptible(sim, sim->time_to_debug);
		if (simulation_should_stop(sim))
			break ;
		log_event(sim, coder->id, "is refactoring");
		sleep_ms_interruptible(sim, sim->time_to_refactor);
	}
	return (NULL);
}
