#include "../include/codexion.h"

static void    debug_output(t_coder *coder, t_status log)
{
    long long   start;
    long long   now;
    char        *red;
    char        *green;
    char        *reset;

    red = "\033[31m";
    green = "\033[32m";
    reset = "\033[0m";
    start = coder->data_all->simulation_start;
    now = get_time(MILLISECOND);
    pthread_mutex_lock(&coder->data_all->write_mutex);
    if (log == TOOK_FIRST)
        fprintf(stdout, "%lld %d has taken dongle D%d\n", now - start, coder->id, coder->first_dongle->id);
    else if (log == TOOK_SECOND)
        fprintf(stdout, "%lld %d has taken dongle D%d\n", now - start, coder->id, coder->second_dongle->id);
    else if (log == COMPILING)
        fprintf(stdout, "%s%lld %d is compiling%s\n", green, now - start, coder->id, reset);
    else if (log == DEBUGGING)
        fprintf(stdout, "%lld %d is debugging\n", now - start, coder->id);
    else if (log == REFACTORING)
        fprintf(stdout, "%lld %d is refactoring\n", now - start, coder->id);
    else if (log == BURNED_OUT)
        fprintf(stdout, "%s%lld %d burned out%s\n", red, now - start, coder->id, reset);
    else
        fprintf(stderr, "log is not valid");
    pthread_mutex_unlock(&coder->data_all->write_mutex);
}

void    normal_output(t_coder *coder, t_status log)
{
    long long   start;
    long long   now;

    start = coder->data_all->simulation_start;
    now = get_time(MILLISECOND);
    pthread_mutex_lock(&coder->data_all->write_mutex);
    if (log == TOOK_FIRST)
        fprintf(stdout, "%lld %d has a taken dongle\n", now - start, coder->id);
    else if (log == TOOK_SECOND)
        fprintf(stdout, "%lld %d has a taken dongle\n", now - start, coder->id);
    else if (log == COMPILING)
        fprintf(stdout, "%lld %d is compiling\n", now - start, coder->id);
    else if (log == DEBUGGING)
        fprintf(stdout, "%lld %d is debugging\n", now - start, coder->id);
    else if (log == REFACTORING)
        fprintf(stdout, "%lld %d is refactoring\n", now - start, coder->id);
    else if (log == BURNED_OUT)
        fprintf(stdout, "%lld %d burned out\n", now - start, coder->id);
    else
        fprintf(stderr, "log is not valid");
    pthread_mutex_unlock(&coder->data_all->write_mutex);
}

void    output(t_coder *coder, t_status log)
{
    bool    debug;

    debug = true;
    if (debug)
        debug_output(coder, log);
    else
        normal_output(coder, log);
}
