#ifndef CODEXION_H
# define CODEXION_H

#include <unistd.h> // write
#include <stdlib.h> // malloc
#include <stdio.h> // ftprintf
#include <pthread.h> // thread tools
#include <string.h> // strlen, strcmp ...
#include <sys/time.h> // gettimeofday
#include <stdbool.h> // bool
#include <errno.h> // errors

# define USAGE  "./codexion <number_of_coders> " \
                "<time_to_burnout> " \
                "<time_to_compile> " \
                "<time_to_debug> " \
                "<time_to_refactor> " \
                "<number_of_compiles_required> " \
                "<dongle_cooldown> " \
                "<scheduler>"

typedef pthread_mutex_t t_mtx;
typedef pthread_cond_t t_cond;
typedef struct s_dongle t_dongle;
typedef struct s_coder t_coder;
typedef struct s_data t_data;
typedef struct s_monitor t_monitor;
typedef struct s_heap_node t_heap_node;
typedef struct s_heap t_heap;

typedef enum e_scheduler
{
    FIFO,
    EDF
}   t_scheduler;

typedef enum e_time
{
    SECOND,
    MILLISECOND,
    MICROSECOND
}   t_time;

typedef enum e_status
{
    COMPILING,
    DEBUGGING,
    REFACTORING,
    TOOK_FIRST,
    TOOK_SECOND,
    WAITING_DONGLE,
    BURNED_OUT
}   t_status;



struct s_data
{
    int         number_of_coders;
    int         time_to_burnout;
    int         time_to_compile;
    int         time_to_debug;
    int         time_to_refactor;
    int         number_of_compiles_required;
    int         dongle_cooldown;
    int         coders_finished;
    bool        ready;
    bool        end_of_simulation;
    t_coder     *coders;
    t_dongle    *dongles;
    t_scheduler    scheduler;
    t_mtx       end_mutex;
    t_mtx       write_mutex;
    t_mtx       ready_mutex;
    t_mtx       another_mutex;
    t_cond      ready_cond;
    t_heap      *heap;
    pthread_t   monitor;
    t_cond      monitor_cond;
    long long   simulation_start;
};


struct s_heap_node
{
    t_coder     *coder;
    long long   deadline;
};

struct s_heap
{
    t_mtx       mutex;
    t_heap_node *node;
    int         size;
};

struct s_coder
{
    int         heap_index;
    int         id;
    int         compile_count;
    long long   last_compile_time;
    long long   arrival_time;
    bool        finished;
    t_data      *data_all;
    t_dongle    *first_dongle;
    t_dongle    *second_dongle;
    t_dongle    *waiting_for;
    t_cond      cond;
    pthread_t   thread;
    t_mtx       read_long_mutex;
    t_mtx       mutex;
    t_mtx       status_mutex;
    t_status    status;
};

struct s_dongle
{
    int         id;
    bool        taken;
    long long   last_release;
    t_coder     *left;
    t_coder     *right;
    t_heap      *heap;
    t_mtx       mutex;
    t_cond      cond;
};

bool    data_validator(int argc, char **argv);
bool    init_data(t_data *data, char **argv);
long    get_time(t_time time);
bool    build_heap(t_data *data);
void    ft_usleep(int to_sleep, t_coder *coder);
void    dongle_cooldown(t_dongle *dongle, t_data *data);
void    heapify(t_heap *arr, int i);
void    output(t_coder *coder, t_status log);
void    run_thread(t_data *data);
void    cleanup(t_data *data);
#endif