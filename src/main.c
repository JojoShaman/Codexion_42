/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   main.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

int	main(int argc, char **argv)
{
	t_data	*data;

	data = malloc(sizeof(*data));
	if (!data)
		return (1);
	if (!data_validator(argc, argv))
	{
		free(data);
		return (1);
	}
	init_data(data, argv);
	if (!init_simulation(data))
	{
		fprintf(stderr, "MEMORY ALLOCATION FAILED");
		cleanup(data);
		return (1);
	}
	cleanup(data);
	return (0);
}
