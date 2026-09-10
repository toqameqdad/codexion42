/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_time.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:46:02 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:46:02 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


long	get_current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

long	elapsed_ms(t_simulation *sim)
{
	return (get_current_time_ms() - sim->start_time_ms);
}