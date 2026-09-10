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
	printf("codexion initialized with %d coder(s), scheduler=%s\n",
		sim.number_of_coders,
		sim.scheduler == CX_SCHED_FIFO ? "fifo" : "edf");
	destroy_simulation(&sim);
	return (0);
}