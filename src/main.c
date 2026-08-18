#include "../include/codexion.h"

int     main(int argc, char **argv)
{
    t_data  *data;

    data = malloc(sizeof(*data));
    if (!data)
        return (1);
    if (!data_validator(argc, argv))
    {
        free(data);
        return (1);
    }
    if (!init_data(data, argv))
        fprintf(stderr, "ERROR ALLOCATION");
    run_thread(data);
    free(data);
    return 0;
}