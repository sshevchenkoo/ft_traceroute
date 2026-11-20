#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "ft_traceroute.h"

/*
 * Resolve the host (IPv4 literal or name) to one IPv4 address.
 * This is the only DNS lookup the mandatory part does: hops are shown as
 * plain addresses, without reverse resolution.
 */
void	resolve_host(t_trace *trace)
{
	struct addrinfo	hints;
	struct addrinfo	*res;
	int				err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	err = getaddrinfo(trace->host, NULL, &hints, &res);
	if (err != 0)
	{
		fprintf(stderr, "%s: %s\n", trace->host, gai_strerror(err));
		fprintf(stderr, "Cannot handle \"host\" cmdline arg `%s' "
			"on position 1 (argc 1)\n", trace->host);
		exit(EXIT_USAGE);
	}
	memcpy(&trace->dest, res->ai_addr, sizeof(trace->dest));
	freeaddrinfo(res);
	inet_ntop(AF_INET, &trace->dest.sin_addr, trace->dest_ip,
		sizeof(trace->dest_ip));
}

/*
 * send_fd: ordinary UDP socket. The kernel builds the IP and UDP headers;
 *          we only change the TTL with setsockopt(IP_TTL) before sending.
 * recv_fd: raw ICMP socket. It is the only way to see the ICMP errors
 *          (Time Exceeded / Port Unreachable) that come back. Needs root.
 * Both stay in blocking mode; timeouts are done with select().
 */
void	open_sockets(t_trace *trace)
{
	struct sockaddr_in	local;
	socklen_t			len;

	trace->recv_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (trace->recv_fd < 0)
		fatal("socket");
	trace->send_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (trace->send_fd < 0)
		fatal("socket");
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = 0;
	if (bind(trace->send_fd, (struct sockaddr *)&local, sizeof(local)) < 0)
		fatal("bind");
	len = sizeof(local);
	if (getsockname(trace->send_fd, (struct sockaddr *)&local, &len) < 0)
		fatal("getsockname");
	trace->sport = ntohs(local.sin_port);
}
