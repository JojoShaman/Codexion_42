/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   parsing.c                                         :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/19 19:26:07 by srosu            #+#    #+#              */
/*   Updated: 2026/08/19 19:26:07 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

static bool	nb_arg_validator(int argc)
{
	char	*error_msg;

	error_msg = "./codexion <number_of_coders> " \
"<time_to_burnout> " \
"<time_to_compile> " \
"<time_to_debug> " \
"<time_to_refactor> " \
"<number_of_compiles_required> " \
"<dongle_cooldown> " \
"<scheduler>";
	if (argc != 9)
	{
		fprintf(stderr, "USAGE ERROR: %s", error_msg);
		return (false);
	}
	return (true);
}

static bool	arg_validator(char *argv)
{
	int	i;

	i = -1;
	while (argv[++i])
	{
		if (!(argv[i] >= '0' && argv[i] <= '9'))
		{
			if (argv[i] == '-')
			{
				if (argv[i + 1] && argv[i + 1] >= '0' && argv[i + 1] <= '9')
					fprintf(stderr, "USAGE ERROR: numbers "
						"should not be negative\n");
				else
					fprintf(stderr, "USAGE ERROR: invalid argument\n");
			}
			else
				fprintf(stderr, "USAGE ERROR: arguments should "
					"only be numbers\n");
			return (false);
		}
	}
	return (true);
}

static bool	free_overflow(char *argv)
{
	int	i;
	int	nb;

	i = -1;
	nb = 0;
	while (argv[++i])
	{
		if ((nb > 214748364 || nb == 214748364) && (argv[i] - '0' > 7))
		{
			fprintf(stderr, "OVERFLOW DETECTED\n");
			return (false);
		}
		nb = (nb * 10) + argv[i] - '0';
	}
	return (true);
}

static bool	scheduler_validator(char *argv)
{
	if (!strcmp(argv, "fifo"))
		return (true);
	else if (!strcmp(argv, "edf"))
		return (true);
	fprintf(stderr, "SCHEDULER '%s' DOES NOT EXIST", argv);
	return (false);
}

bool	data_validator(int argc, char **argv)
{
	int	i;

	i = 0;
	if (!nb_arg_validator(argc))
		return (false);
	while (argv[++i])
	{
		if (i != 8)
		{
			if (!arg_validator(argv[i]) || !free_overflow(argv[i]))
				return (false);
		}
		else
		{
			if (!scheduler_validator(argv[i]))
				return (false);
		}
	}
	if (atoi(argv[1]) <= 1)
	{
		fprintf(stderr, "USAGE ERROR: nb_coders cannot "
			"be less or equal to 1\n");
		return (false);
	}
	return (true);
}
