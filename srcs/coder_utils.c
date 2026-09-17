/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 17:26:02 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/11 17:26:02 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	coder_get_last_compile_start(t_coder *coder)
{
	long	value;

	pthread_mutex_lock(&coder->state_lock);
	value = coder->last_compile_start_ms;
	pthread_mutex_unlock(&coder->state_lock);
	return (value);
}

void	coder_set_last_compile_start(t_coder *coder, long value)
{
	pthread_mutex_lock(&coder->state_lock);
	coder->last_compile_start_ms = value;
	pthread_mutex_unlock(&coder->state_lock);
}

int	coder_get_compiles_done(t_coder *coder)
{
	int	value;

	pthread_mutex_lock(&coder->state_lock);
	value = coder->compiles_done;
	pthread_mutex_unlock(&coder->state_lock);
	return (value);
}

void	coder_inc_compiles_done(t_coder *coder)
{
	pthread_mutex_lock(&coder->state_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->state_lock);
}

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
