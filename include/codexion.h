/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   codexion.h                                        :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:39:58 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h> // write
# include <stdlib.h> // malloc
# include <stdio.h> // ftprintf
# include <pthread.h> // thread tools
# include <string.h> // strlen, strcmp ...
# include <sys/time.h> // gettimeofday
# include <stdbool.h> // bool
# include <errno.h> // errors

typedef pthread_mutex_t		t_mtx;
typedef pthread_cond_t		t_cond;
typedef struct s_dongle		t_dongle;
typedef struct s_coder		t_coder;
typedef struct s_data		t_data;
typedef struct s_monitor	t_monitor;
typedef struct s_heap_node	t_heap_node;
typedef struct s_heap		t_heap;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef enum e_time
{
	SECOND,
	MILLISECOND,
	MICROSECOND
}	t_time;

typedef enum e_status
{
	COMPILING,
	DEBUGGING,
	REFACTORING,
	TOOK_FIRST,
	TOOK_SECOND,
	WAITING_DONGLE,
	BURNED_OUT,
	INIT
}	t_status;

struct s_data
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	bool		end_of_simulation;
	long long	simulation_start;
	int			coders_finished;
	bool		ready;
	t_heap		*heap;
	t_coder		*coders;
	t_dongle	*dongles;
	pthread_t	monitor;
	t_scheduler	scheduler;
	t_cond		gate_cond;
	t_cond		monitor_cond;
	t_mtx		finished_mutex;
	t_mtx		gate_mutex;
	t_mtx		stdout_mutex;
	t_mtx		end_mutex;
};

struct s_heap_node
{
	long long	deadline;
	t_coder		*coder;
};

struct s_heap
{
	int			size;
	t_heap_node	*node;
	t_mtx		mutex;
};

struct s_coder
{
	int			id;
	bool		finished;
	int			heap_index;
	int			compile_count;
	long long	last_compile_time;
	long long	arrival_time;
	t_data		*data;
	t_status	status;
	pthread_t	thread;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;
	t_dongle	*waiting_for;
	t_cond		cond;
	t_mtx		mutex;
};

struct s_dongle
{
	int			id;
	bool		taken;
	long long	last_release;
	t_heap		*heap;
	t_coder		*left;
	t_coder		*right;
	t_cond		cond;
	t_mtx		mutex;
};

bool		data_validator(int argc, char **argv);
void		init_data(t_data *data, char **argv);
long		get_time(t_time time);
bool		build_heap(t_data *data);
void		ft_usleep(int to_sleep, t_coder *coder);
void		dongle_cooldown(t_dongle *dongle, t_data *data);
void		heapify(t_heap *arr, int i);
void		output(t_coder *coder, t_status log);
void		create_join(t_data *data);
void		cleanup(t_data *data);
void		sift_up(t_heap *heap, int i);
void		attribute_dongle(t_coder *coder, int position);
bool		is_end(t_data *data);
void		set_status(t_coder *coder, t_status log, bool display);
void		set_end(t_data *data);
bool		reached_compile_target(t_coder *coder);
long long	get_long(t_mtx *mutex, long long *value);
bool		is_end(t_data *data);
bool		dongle_acquire(t_dongle *dongle, t_coder *coder);
void		dongle_release(t_dongle *dongle);
void		update_deadline(t_coder *coder);
void		*monitoring(void *all_data);
void		*run(void *data);
bool		init_simulation(t_data *data);
void		remove_node(t_coder *coder);
t_coder		*first_arrived(t_dongle *dongle);
t_coder		*shortest_deadline(t_dongle *dongle);
void		dongle_release(t_dongle *dongle);
bool		is_waiting(t_mtx *mutex, t_dongle *requested, t_dongle *current);
t_coder		*winner(long long right, long long left, t_dongle *dongle);
bool		safe_if_end(t_coder *coder, t_dongle *dongle);
t_coder		*priority(t_coder *coder, t_dongle *dongle);

#endif
