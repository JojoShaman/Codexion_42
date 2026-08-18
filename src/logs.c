#include "../include/codexion.h"

void    output(t_coder *coder, t_status log)
{
    long long   start;
    long long   now;

    start = coder->data_all->simulation_start;
    now = get_time(MILLISECOND);
    pthread_mutex_lock(&coder->data_all->write_mutex);
    if (log == TAKING_DONGLE)
        fprintf(stdout, "%lld %d has taken a dongle\n", now - start, coder->id);
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
