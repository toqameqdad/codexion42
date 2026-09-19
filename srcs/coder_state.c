/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_state.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 00:00:00 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/19 00:00:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	coder_get_finished(t_coder *coder)
{
	int	value;

	pthread_mutex_lock(&coder->state_lock);
	value = coder->finished;
	pthread_mutex_unlock(&coder->state_lock);
	return (value);
}

void	coder_set_finished(t_coder *coder, int value)
{
	pthread_mutex_lock(&coder->state_lock);
	coder->finished = value;
	pthread_mutex_unlock(&coder->state_lock);
}
