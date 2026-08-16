#include "../include/codexion.h"

long get_time(t_time time)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    if (time == SECOND)
        return (tv.tv_sec);
    else if (time == MILLISECOND)
        return (tv.tv_sec * 1e3 + tv.tv_usec / 1e3);
    else if (time == MICROSECOND)
        return (tv.tv_usec);
    return 0;
}