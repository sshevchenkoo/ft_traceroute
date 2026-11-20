#include <stddef.h>
#include <errno.h>
#include <sys/select.h>
#include "ft_traceroute.h"

/*
 * Like the reference traceroute (its -N option, default 16), up to
 * SIM_PROBES probes are "in flight" at the same time, so several hops are
 * probed in parallel. Results are still printed strictly hop by hop:
 * a hop is printed once each of its probes is answered or timed out.
 * Without this, a path that ends in "* * *" would take 30 x 5 s.
 */

static struct timeval	deadline_of(const t_probe *p)
{
	struct timeval	d;

	d = p->sent;
	d.tv_sec += WAIT_SEC;
	return (d);
}

/* Send new probes while fewer than SIM_PROBES are waiting for an answer.
 * Never send past the hop where the target has already answered. */
static void	fill_window(t_trace *trace, int first)
{
	int	inflight;
	int	i;

	inflight = 0;
	i = first;
	while (i < trace->nsent)
	{
		if (!trace->probes[i].done)
			inflight++;
		i++;
	}
	while (inflight < SIM_PROBES && trace->nsent < trace->last_ttl * NPROBES)
	{
		send_probe(trace, trace->nsent);
		trace->nsent++;
		inflight++;
	}
}

static int	tv_before(const struct timeval *a, const struct timeval *b)
{
	return (a->tv_sec < b->tv_sec
		|| (a->tv_sec == b->tv_sec && a->tv_usec < b->tv_usec));
}

/* Mark timed-out probes as done (they will print as "*"); store the time
 * until the nearest deadline of the probes still waiting into timeout.
 * Returns 0 if no probe is waiting. */
static int	expire_probes(t_trace *trace, int first, struct timeval *timeout)
{
	struct timeval	d;
	struct timeval	left;
	t_probe			*p;
	int				waiting;
	int				i;

	waiting = 0;
	i = first;
	while (i < trace->nsent)
	{
		p = &trace->probes[i];
		if (!p->done)
		{
			d = deadline_of(p);
			if (!tv_remaining(&d, &left))
				p->done = 1;
			else if (!waiting || tv_before(&left, timeout))
			{
				*timeout = left;
				waiting = 1;
			}
		}
		i++;
	}
	return (waiting);
}

static int	hop_done(t_trace *trace, int ttl)
{
	int	i;

	i = (ttl - 1) * NPROBES;
	while (i < ttl * NPROBES)
	{
		if (i >= trace->nsent || !trace->probes[i].done)
			return (0);
		i++;
	}
	return (1);
}

/* Block in select() until a packet arrives or the nearest probe deadline. */
static void	wait_packet(t_trace *trace, struct timeval *timeout)
{
	fd_set	rfds;
	int		ret;

	FD_ZERO(&rfds);
	FD_SET(trace->recv_fd, &rfds);
	ret = select(trace->recv_fd + 1, &rfds, NULL, NULL, timeout);
	if (ret < 0 && errno != EINTR)
		fatal("select");
	if (ret > 0)
		receive_packet(trace);
}

void	run_trace(t_trace *trace)
{
	struct timeval	timeout;
	int				ttl;

	ttl = 1;
	while (ttl <= trace->last_ttl)
	{
		fill_window(trace, (ttl - 1) * NPROBES);
		if (expire_probes(trace, (ttl - 1) * NPROBES, &timeout)
			&& !hop_done(trace, ttl))
			wait_packet(trace, &timeout);
		while (ttl <= trace->last_ttl && hop_done(trace, ttl))
		{
			print_hop(ttl, &trace->probes[(ttl - 1) * NPROBES], NPROBES);
			ttl++;
		}
	}
}
