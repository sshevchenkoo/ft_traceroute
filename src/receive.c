#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "ft_traceroute.h"

/*
 * Layout of an ICMP error as read from a raw socket:
 *
 *   [ outer IP hdr ][ ICMP hdr 8B ][ inner IP hdr ][ inner UDP hdr 8B ]
 *     ip_src = hop    type / code    = header of OUR probe, quoted back
 *
 * The inner UDP header carries our source port (identifies this process)
 * and the destination port (identifies the probe).
 */

/* Error mark printed after the time, same table as the reference traceroute.
 * Returns 1 if the reply ends the trace. */
static int	classify(int type, int code, char *err)
{
	err[0] = '\0';
	if (type == ICMP_TIME_EXCEEDED && code == ICMP_EXC_TTL)
		return (0);
	if (type != ICMP_DEST_UNREACH)
		snprintf(err, ERR_STR_LEN, "!<%d-%d>", type, code);
	else if (code == ICMP_PORT_UNREACH)
		err[0] = '\0';
	else if (code == ICMP_NET_UNREACH || code == ICMP_NET_UNKNOWN
		|| code == ICMP_HOST_ISOLATED || code == ICMP_NET_UNR_TOS)
		snprintf(err, ERR_STR_LEN, "!N");
	else if (code == ICMP_HOST_UNREACH || code == ICMP_HOST_UNKNOWN
		|| code == ICMP_HOST_UNR_TOS)
		snprintf(err, ERR_STR_LEN, "!H");
	else if (code == ICMP_NET_ANO || code == ICMP_HOST_ANO
		|| code == ICMP_PKT_FILTERED)
		snprintf(err, ERR_STR_LEN, "!X");
	else if (code == ICMP_PROT_UNREACH)
		snprintf(err, ERR_STR_LEN, "!P");
	else if (code == ICMP_FRAG_NEEDED)
		snprintf(err, ERR_STR_LEN, "!F");
	else if (code == ICMP_SR_FAILED)
		snprintf(err, ERR_STR_LEN, "!S");
	else if (code == ICMP_PREC_VIOLATION)
		snprintf(err, ERR_STR_LEN, "!V");
	else if (code == ICMP_PREC_CUTOFF)
		snprintf(err, ERR_STR_LEN, "!C");
	else
		snprintf(err, ERR_STR_LEN, "!<%d>", code);
	return (1);
}

static int	read_port(const unsigned char *p)
{
	uint16_t	port;

	memcpy(&port, p, sizeof(port));
	return (ntohs(port));
}

/* Returns the index of the probe this packet answers, or -1 if it is
 * not for us (echo replies, other programs' traffic, truncated packets,
 * late answers to probes that already timed out). */
static int	match_probe(t_trace *trace, const unsigned char *buf, ssize_t n)
{
	const struct ip		*outer;
	const struct icmp	*icmp;
	const struct ip		*inner;
	const unsigned char	*udp;
	int					idx;

	outer = (const struct ip *)buf;
	if (n < (ssize_t)sizeof(struct ip) || n < outer->ip_hl * 4 + ICMP_MINLEN)
		return (-1);
	icmp = (const struct icmp *)(buf + outer->ip_hl * 4);
	if (icmp->icmp_type != ICMP_TIME_EXCEEDED
		&& icmp->icmp_type != ICMP_DEST_UNREACH)
		return (-1);
	inner = (const struct ip *)((const unsigned char *)icmp + ICMP_MINLEN);
	if (n < outer->ip_hl * 4 + ICMP_MINLEN + (ssize_t)sizeof(struct ip)
		|| n < outer->ip_hl * 4 + ICMP_MINLEN + inner->ip_hl * 4 + UDP_HDR_LEN)
		return (-1);
	if (inner->ip_p != IPPROTO_UDP)
		return (-1);
	udp = (const unsigned char *)inner + inner->ip_hl * 4;
	if (read_port(udp) != trace->sport)
		return (-1);
	idx = read_port(udp + 2) - BASE_PORT;
	if (idx < 0 || idx >= trace->nsent || trace->probes[idx].done)
		return (-1);
	return (idx);
}

/* Read one packet from the raw socket (select() said it is ready, so this
 * does not block) and record it on the probe it answers. */
void	receive_packet(t_trace *trace)
{
	unsigned char		buf[RECV_BUF_LEN];
	struct sockaddr_in	from;
	socklen_t			len;
	ssize_t				n;
	struct timeval		now;
	int					idx;
	t_probe				*p;
	const struct icmp	*icmp;

	len = sizeof(from);
	n = recvfrom(trace->recv_fd, buf, sizeof(buf), 0,
			(struct sockaddr *)&from, &len);
	gettimeofday(&now, NULL);
	if (n < 0)
	{
		if (errno == EINTR)
			return ;
		fatal("recvfrom");
	}
	idx = match_probe(trace, buf, n);
	if (idx < 0)
		return ;
	icmp = (const struct icmp *)(buf + ((const struct ip *)buf)->ip_hl * 4);
	p = &trace->probes[idx];
	p->done = 1;
	p->answered = 1;
	p->recv = now;
	p->from = from.sin_addr;
	p->final = classify(icmp->icmp_type, icmp->icmp_code, p->err);
	if (p->final && idx / NPROBES + 1 < trace->last_ttl)
		trace->last_ttl = idx / NPROBES + 1;
}
