#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "ft_traceroute.h"

/* Same as the reference traceroute's error(): "<what>: <strerror>", exit 1 */
void	fatal(const char *what)
{
	fprintf(stderr, "%s: %s\n", what, strerror(errno));
	exit(EXIT_FAIL);
}

int	main(int argc, char **argv)
{
	static t_trace	trace;

	trace.send_fd = -1;
	trace.recv_fd = -1;
	trace.last_ttl = MAX_HOPS;
	parse_args(argc, argv, &trace);
	resolve_host(&trace);
	open_sockets(&trace);
	print_header(&trace);
	run_trace(&trace);
	close(trace.send_fd);
	close(trace.recv_fd);
	return (EXIT_OK);
}
