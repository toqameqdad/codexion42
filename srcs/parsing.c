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

static int	str_to_long(const char *s, long *result)
{
	long	value;
	int		i;
	int		digit;

	value = 0;
	i = 0;
	if (s[i] == '+')
		i++;
	while (s[i])
	{
		digit = s[i] - '0';
		if (value > (LONG_MAX - digit) / 10)
			return (1);
		value = value * 10 + digit;
		i++;
	}
	*result = value;
	return (0);
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
	long	values[7];
	int		i;

	if (argc != 9)
		return (1);
	i = 0;
	while (i < 7)
	{
		if (!is_valid_positive_number(argv[i + 1]))
			return (1);
		if (str_to_long(argv[i + 1], &values[i]) != 0)
			return (1);
		i++;
	}
	if (values[0] < 1 || values[0] > INT_MAX)
		return (1);
	if (values[5] > INT_MAX)
		return (1);
	sim->number_of_coders = (int)values[0];
	sim->time_to_burnout = values[1];
	sim->time_to_compile = values[2];
	sim->time_to_debug = values[3];
	sim->time_to_refactor = values[4];
	sim->number_of_compiles_required = (int)values[5];
	sim->dongle_cooldown = values[6];
	if (parse_scheduler(argv[8], &sim->scheduler) != 0)
		return (1);
	return (0);
}
