#include <stddef.h>
#include "ft_traceroute.h"

double	tv_diff_ms(const struct timeval *end, const struct timeval *start)
{
	return ((end->tv_sec - start->tv_sec) * 1000.0
		+ (end->tv_usec - start->tv_usec) / 1000.0);
}

/* Time left until deadline, written to out. Returns 0 if already passed. */
int	tv_remaining(const struct timeval *deadline, struct timeval *out)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	out->tv_sec = deadline->tv_sec - now.tv_sec;
	out->tv_usec = deadline->tv_usec - now.tv_usec;
	if (out->tv_usec < 0)
	{
		out->tv_sec--;
		out->tv_usec += 1000000;
	}
	return (out->tv_sec >= 0 && (out->tv_sec > 0 || out->tv_usec > 0));
}
