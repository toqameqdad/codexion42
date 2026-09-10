/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmeqdad <toqa.meqdad@learner.42.tech>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 20:45:39 by tmeqdad           #+#    #+#             */
/*   Updated: 2026/09/10 20:45:39 by tmeqdad          ###   ########.fr       */
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
	int				in_use;
	long			last_released_ms;
}	t_dongle;


typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	int				compiles_done;
	long			last_compile_start_ms; 
	int				is_compiling;		
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
	long			dongle_cooldown;
	t_scheduler		scheduler;

	t_dongle		*dongles;		
	t_coder			*coders;		

	pthread_mutex_t	print_lock;		
	pthread_mutex_t	stop_lock;		
	int				stop_flag;		

	long			start_time_ms;	
	pthread_t		monitor_thread;
}	t_simulation;


long	get_current_time_ms(void);
long	elapsed_ms(t_simulation *sim);

int		parse_args(int argc, char **argv, t_simulation *sim);

int		init_simulation(t_simulation *sim);
void	destroy_simulation(t_simulation *sim);

void	log_event(t_simulation *sim, int coder_id, const char *msg);

int		simulation_should_stop(t_simulation *sim);
void	simulation_request_stop(t_simulation *sim);

#endif