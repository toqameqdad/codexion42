/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:45:39 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/17 16:35:00 by tmeqdad          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <string.h>
# include <limits.h>
# include "heap.h"

typedef enum e_scheduler
{
	CX_SCHED_FIFO,
	CX_SCHED_EDF
}	t_scheduler;

typedef enum e_state
{
	STATE_COMPILING,
	STATE_DEBUGGING,
	STATE_REFACTORING
}	t_state;

typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	int				in_use;
	long			last_released_ms;
	t_heap			wait_queue;
}	t_dongle;

typedef struct s_coder
{
	int					id;
	pthread_t			thread;
	t_dongle			*left_dongle;
	t_dongle			*right_dongle;
	pthread_mutex_t		state_lock;
	int					compiles_done;
	int					finished;
	long				last_compile_start_ms;
	struct s_simulation	*sim;
}	t_coder;

typedef struct s_simulation
{
	int				number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				number_of_compiles_required;
	int				created_coders;
	int				monitor_started;
	long			dongle_cooldown;
	long			request_sequence;
	t_scheduler		scheduler;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	stop_lock;
	int				stop_flag;
	int				finished_count;
	pthread_mutex_t	queue_lock;
	pthread_cond_t	queue_cond;
	long			start_time_ms;
	pthread_t		monitor_thread;
}	t_simulation;

long	get_current_time_ms(void);
long	elapsed_ms(t_simulation *sim);
int		parse_args(int argc, char **argv, t_simulation *sim);
int		is_valid_positive_number(const char *s);
int		str_to_long(const char *s, long *result);
int		init_simulation(t_simulation *sim);
int		init_shared_locks(t_simulation *sim);
int		init_queue(t_simulation *sim);
void	cleanup_shared_data(t_simulation *sim);
void	destroy_simulation(t_simulation *sim);
void	cleanup_dongles(t_simulation *sim, int count);
void	cleanup_coders(t_simulation *sim, int count);
void	log_event(t_simulation *sim, int coder_id, const char *msg);
int		simulation_should_stop(t_simulation *sim);
void	simulation_request_stop(t_simulation *sim);
void	simulation_mark_finished(t_simulation *sim);
long	compute_priority(t_simulation *sim, t_coder *coder,
			long request_time_ms);
int		scheduler_acquire_both(t_simulation *sim, t_coder *coder,
			long request_time_ms);
int		scheduler_try_pair(t_simulation *sim, t_coder *coder, long *wait);
int		scheduler_try_single(t_simulation *sim, t_coder *coder, long *wait);
int		scheduler_reserve_ready(t_simulation *sim, t_coder *coder,
			t_heap_node *front);
void	ms_to_abs_timespec(long ms_from_now, struct timespec *ts);
void	sleep_ms_interruptible(t_simulation *sim, long ms);
void	release_both_dongles(t_simulation *sim, t_coder *coder);
long	coder_get_last_compile_start(t_coder *coder);
void	coder_set_last_compile_start(t_coder *coder, long value);
int		coder_get_compiles_done(t_coder *coder);
void	coder_inc_compiles_done(t_coder *coder);
int		coder_get_finished(t_coder *coder);
void	coder_set_finished(t_coder *coder, int value);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

#endif
