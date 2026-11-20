#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ft_traceroute.h"

/*
 * Mirrors the reference traceroute's command line handling:
 *   no arguments at all        -> help on stderr, exit 0
 *   --help anywhere            -> help on stderr, exit 0
 *   any other "-x" / "--xxx"   -> "Bad option `-x' (argc N)", exit 2
 *   "--"                       -> end of options
 *   1st positional             -> host
 *   2nd positional             -> packetlen in the original; not supported here
 *   3rd positional             -> "Extra arg ..."
 * A lone "-" is a positional argument (and an invalid host), as in the original.
 */

static int	is_option(const char *arg)
{
	return (arg[0] == '-' && arg[1] != '\0');
}

/* First pass: options only. --help wins wherever it is, unless a bad option
 * comes before it (the original stops at the first bad option). */
static void	check_options(int argc, char **argv)
{
	int	i;

	i = 1;
	while (i < argc)
	{
		if (strcmp(argv[i], "--") == 0)
			return ;
		if (is_option(argv[i]))
		{
			if (strcmp(argv[i], "--help") != 0)
			{
				fprintf(stderr, "Bad option `%s' (argc %d)\n", argv[i], i);
				exit(EXIT_USAGE);
			}
			print_help();
			exit(EXIT_OK);
		}
		i++;
	}
}

/* Second pass: positional arguments, with their argv index for messages. */
static int	collect_positionals(int argc, char **argv, int *idx)
{
	int	i;
	int	n;
	int	end_of_opts;

	i = 1;
	n = 0;
	end_of_opts = 0;
	while (i < argc)
	{
		if (!end_of_opts && strcmp(argv[i], "--") == 0)
			end_of_opts = 1;
		else if (end_of_opts || !is_option(argv[i]))
		{
			if (n < 3)
				idx[n] = i;
			n++;
		}
		i++;
	}
	return (n);
}

void	parse_args(int argc, char **argv, t_trace *trace)
{
	int	idx[3];
	int	n;

	if (argc < 2)
	{
		print_help();
		exit(EXIT_OK);
	}
	check_options(argc, argv);
	n = collect_positionals(argc, argv, idx);
	if (n == 0)
	{
		fprintf(stderr, "Specify \"host\" missing argument.\n");
		exit(EXIT_USAGE);
	}
	if (n >= 3)
	{
		fprintf(stderr, "Extra arg `%s' (position 3, argc %d)\n",
			argv[idx[2]], idx[2]);
		exit(EXIT_USAGE);
	}
	if (n == 2)
	{
		fprintf(stderr, "Cannot handle \"packetlen\" cmdline arg `%s' "
			"on position 2 (argc %d)\n", argv[idx[1]], idx[1]);
		exit(EXIT_USAGE);
	}
	trace->host = argv[idx[0]];
}
