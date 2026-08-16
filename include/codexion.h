#ifndef CODEXION_H
# define CODEXION_H

#include <unistd.h> // write
#include <stdlib.h> // malloc
#include <stdio.h> // ftprintf
#include <pthread.h> // thread tools
#include <string.h> // strlen, strcmp ...
#include <sys/time.h> // gettimeofday
#include <stdbool.h> // bool

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

struct s_data
{
    int         number_of_coders;
    int         time_to_burnout;
    int         time_to_compile;
    int         time_to_debug;
    int         time_to_refactor;
    int         number_of_compiles_required;
    int         dongle_cooldown;
    bool        coders_ready;
    bool        end;
    t_coder     *coders;
    t_dongle    *dongles;
    t_scheduler    scheduler;
    t_mtx       write_mutex;
    long long   simulation_start;
};

struct s_coder
{
    int         id;
    int         compile_count;
    long long   last_compile_time;
    t_data      *data_all;
    t_dongle    *first_dongle;
    t_dongle    *second_dongle;
    t_cond      cond;
    pthread_t   thread;
};

struct s_dongle
{
    int         id;
    t_mtx       mutex;
    t_cond      cond;
};

struct  s_monitor
{
    t_cond  cond;
};

bool    data_validator(int argc, char **argv);
bool    init_data(t_data *data, char **argv);
long    get_time(t_time time);

#endif