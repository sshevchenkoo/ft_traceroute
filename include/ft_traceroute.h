#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <stdint.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# define PROG_NAME		"ft_traceroute"

/* Exit codes used by the reference traceroute */
# define EXIT_OK		0
# define EXIT_FAIL		1
# define EXIT_USAGE		2

/* Defaults of the reference traceroute (UDP method) */
# define MAX_HOPS		30
# define NPROBES		3
# define SIM_PROBES		16
# define WAIT_SEC		5
# define BASE_PORT		33434
# define TOTAL_PROBES	(MAX_HOPS * NPROBES)

/* 20 (IP header) + 8 (UDP header) + 32 (payload) = 60 byte packets */
# define IP_HDR_LEN		20
# define UDP_HDR_LEN	8
# define DATA_LEN		32
# define PACKET_LEN		(IP_HDR_LEN + UDP_HDR_LEN + DATA_LEN)

# define RECV_BUF_LEN	1500
# define ERR_STR_LEN	16

/*
 * One probe = one UDP datagram. Probe number i (0-based) is sent with
 * TTL i / NPROBES + 1 to destination port BASE_PORT + i, so the port quoted
 * back inside an ICMP error tells us directly which probe it answers.
 */
typedef struct s_probe
{
	struct timeval	sent;
	struct timeval	recv;
	int				done;
	int				answered;
	struct in_addr	from;
	char			err[ERR_STR_LEN];
	int				final;
}	t_probe;

typedef struct s_trace
{
	char				*host;
	struct sockaddr_in	dest;
	char				dest_ip[INET_ADDRSTRLEN];
	int					send_fd;
	int					recv_fd;
	uint16_t			sport;
	int					nsent;
	int					last_ttl;
	t_probe				probes[TOTAL_PROBES];
}	t_trace;

/* args.c */
void	parse_args(int argc, char **argv, t_trace *trace);

/* usage.c */
void	print_help(void);

/* init.c */
void	resolve_host(t_trace *trace);
void	open_sockets(t_trace *trace);

/* probe.c */
void	send_probe(t_trace *trace, int idx);

/* receive.c */
void	receive_packet(t_trace *trace);

/* trace.c */
void	run_trace(t_trace *trace);

/* print.c */
void	print_header(t_trace *trace);
void	print_hop(int ttl, t_probe *probes, int count);

/* time.c */
double	tv_diff_ms(const struct timeval *end, const struct timeval *start);
int		tv_remaining(const struct timeval *deadline, struct timeval *out);

/* main.c */
void	fatal(const char *what);

#endif
