/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:45:59 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:45:59 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_scheduler(const char *s, t_scheduler *out)
{
	if (strcmp(s, "fifo") == 0)
	{
		*out = CX_SCHED_FIFO;
		return (0);
	}
	if (strcmp(s, "edf") == 0)
	{
		*out = CX_SCHED_EDF;
		return (0);
	}
	return (1);
}

static int	parse_numbers(char **argv, long *values)
{
	int	i;

	i = 0;
	while (i < 7)
	{
		if (!is_valid_positive_number(argv[i + 1]))
			return (1);
		if (str_to_long(argv[i + 1], &values[i]) != 0)
			return (1);
		i++;
	}
	return (0);
}

static void	store_numbers(t_simulation *sim, long *values)
{
	sim->number_of_coders = (int)values[0];
	sim->time_to_burnout = values[1];
	sim->time_to_compile = values[2];
	sim->time_to_debug = values[3];
	sim->time_to_refactor = values[4];
	sim->number_of_compiles_required = (int)values[5];
	sim->dongle_cooldown = values[6];
}

int	parse_args(int argc, char **argv, t_simulation *sim)
{
	long	values[7];

	if (argc != 9)
		return (1);
	if (parse_numbers(argv, values) != 0)
		return (1);
	if (values[0] < 1 || values[0] > INT_MAX)
		return (1);
	if (values[1] == 0 || values[2] == 0 || values[3] == 0
		|| values[4] == 0 || values[5] == 0)
		return (1);
	if (values[5] > INT_MAX)
		return (1);
	store_numbers(sim, values);
	if (parse_scheduler(argv[8], &sim->scheduler) != 0)
		return (1);
	return (0);
}
