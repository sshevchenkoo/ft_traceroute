#include <string.h>
#include <sys/socket.h>
#include "ft_traceroute.h"

/*
 * Send probe number idx: TTL = idx / NPROBES + 1, port = BASE_PORT + idx.
 * The kernel builds the IP and UDP headers; we only set the TTL.
 */
void	send_probe(t_trace *trace, int idx)
{
	char	data[DATA_LEN];
	int		ttl;

	memset(data, 0, sizeof(data));
	ttl = idx / NPROBES + 1;
	if (setsockopt(trace->send_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
		fatal("setsockopt");
	trace->dest.sin_port = htons(BASE_PORT + idx);
	gettimeofday(&trace->probes[idx].sent, NULL);
	if (sendto(trace->send_fd, data, sizeof(data), 0,
			(struct sockaddr *)&trace->dest, sizeof(trace->dest)) < 0)
		fatal("send");
}
