#include <stdio.h>
#include "ft_traceroute.h"

void	print_header(t_trace *trace)
{
	printf("traceroute to %s (%s), %d hops max, %d byte packets\n",
		trace->host, trace->dest_ip, MAX_HOPS, PACKET_LEN);
	fflush(stdout);
}

/* The address is printed for the first answered probe of the line, and again
 * only when it differs from the last answered probe (load balancing). */
static int	must_print_addr(t_probe *probes, int i)
{
	int	j;

	j = i - 1;
	while (j >= 0 && !probes[j].answered)
		j--;
	if (j < 0)
		return (1);
	return (probes[j].from.s_addr != probes[i].from.s_addr);
}

/* " 1  10.0.2.2  0.512 ms  0.433 ms  0.401 ms"
 * " 2  * * *"
 * " 9  93.184.216.34  11.020 ms !H  ..." */
void	print_hop(int ttl, t_probe *probes, int count)
{
	char	addr[INET_ADDRSTRLEN];
	int		i;

	printf("%2d ", ttl);
	i = 0;
	while (i < count)
	{
		if (!probes[i].answered)
			printf(" *");
		else
		{
			if (must_print_addr(probes, i))
			{
				inet_ntop(AF_INET, &probes[i].from, addr, sizeof(addr));
				printf(" %s", addr);
			}
			printf("  %.3f ms",
				tv_diff_ms(&probes[i].recv, &probes[i].sent));
			if (probes[i].err[0])
				printf(" %s", probes[i].err);
		}
		i++;
	}
	printf("\n");
	fflush(stdout);
}
