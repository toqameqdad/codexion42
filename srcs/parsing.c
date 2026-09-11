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

static int	is_valid_positive_number(const char *s)
{
	int	i;

	i = 0;
	if (!s || !s[0])
		return (0);
	if (s[i] == '+')
		i++;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static long	str_to_long(const char *s)
{
	long	result;
	int		i;

	result = 0;
	i = 0;
	if (s[i] == '+')
		i++;
	while (s[i])
	{
		result = result * 10 + (s[i] - '0');
		i++;
	}
	return (result);
}

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

int	parse_args(int argc, char **argv, t_simulation *sim)
{
	int	i;

	if (argc != 9)
		return (1);
	i = 1;
	while (i <= 7)
	{
		if (!is_valid_positive_number(argv[i]))
			return (1);
		i++;
	}
	sim->number_of_coders = (int)str_to_long(argv[1]);
	sim->time_to_burnout = str_to_long(argv[2]);
	sim->time_to_compile = str_to_long(argv[3]);
	sim->time_to_debug = str_to_long(argv[4]);
	sim->time_to_refactor = str_to_long(argv[5]);
	sim->number_of_compiles_required = (int)str_to_long(argv[6]);
	sim->dongle_cooldown = str_to_long(argv[7]);
	if (sim->number_of_coders < 1)
		return (1);
	if (parse_scheduler(argv[8], &sim->scheduler) != 0)
		return (1);
	return (0);
}
