#include "../include/codexion.h"

static bool    nb_arg_validator(int argc)
{
    if (argc != 9)
    {
        fprintf(stderr, "USAGE ERROR: %s", USAGE);
        return false;
    }
    return true;
}

static bool     arg_validator(char *argv)
{
    int     i;

    i = -1;
    while (argv[++i])
    {
        if (!(argv[i] >= '0' && argv[i] <= '9'))
        {
            fprintf(stderr, "USAGE ERROR: " \
                "arguments should only be numbers");
            return false;
        }
        if (i > 0 && argv[i - 1] == '-')
        {
            fprintf(stderr, "USAGE ERROR: " \
                    "numbers should not be negative");
                return false;
        }
    }
    return true;
}

static bool     free_overflow(char *argv)
{
    int     i;
    int     nb;

    i = -1;
    nb = 0;
    while (argv[++i])
    {
        if (nb > 214748364 || nb == 214748364 && (argv[i] - '0' > 7))
        {
            fprintf(stderr, "OVERFLOW DETECTED");
            return false;
        }
        nb = (nb * 10) + argv[i] - '0';
    }

    return true;
}

static bool     scheduler_validator(char *argv)
{
    if (!strcmp(argv, "fifo"))
        return true;
    else if (!strcmp(argv, "edf"))
        return true;
    fprintf(stderr, "SCHEDULER '%s' DOES NOT EXIST", argv);
    return false;
}

bool    data_validator(int argc, char **argv)
{
    int     i;

    i = 0;
    if (!nb_arg_validator(argc))
        return false;
    while (argv[++i])
    {
        if (i != 8)
        {
            if (!arg_validator(argv[i]) || !free_overflow(argv[i]))
                return false;
        }
        else
        {
            if (!scheduler_validator(argv[i]))
                return false;
        }
    }

    return true;
}