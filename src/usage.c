#include <stdio.h>
#include "ft_traceroute.h"

/* Same layout as the reference traceroute's help, limited to what we support.
 * Like the original, it goes to stderr. */
void	print_help(void)
{
	fprintf(stderr,
		"Usage:\n"
		"  " PROG_NAME " [ --help ] host\n"
		"Options:\n"
		"  --help                      Read this help and exit\n"
		"\n"
		"Arguments:\n"
		"+     host          The host to traceroute to\n");
}
