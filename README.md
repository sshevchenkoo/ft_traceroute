# ft_traceroute

A reimplementation of the Linux `traceroute` command in C (42 school project, mandatory part).

It sends UDP probes with increasing TTL and reads the ICMP *Time Exceeded* and
*Port Unreachable* replies on a raw socket. The output matches `traceroute -n`
(Dmitry Butskoy's traceroute, the one shipped by Debian).

```
$ sudo ./ft_traceroute google.com
traceroute to google.com (142.250.74.46), 30 hops max, 60 byte packets
 1  10.0.2.2  0.512 ms  0.433 ms  0.401 ms
 2  * * *
 ...
```

## Build & run

```sh
make
sudo ./ft_traceroute <host>     # raw socket -> needs root
./ft_traceroute --help
```

Linux only (uses Linux `<netinet/ip_icmp.h>` constants).

## Behaviour

- 30 hops max, 3 probes per hop, 5 s timeout, UDP destination ports from 33434
- up to 16 probes in flight at once, as in the original (`-N 16`), printed hop by hop
- target resolved once, hops shown as plain IPv4 addresses (no reverse DNS)
- `!H !N !P !F !S !X !V !C` marks for ICMP unreachable codes, `*` for timeouts
- argument errors, messages and exit codes as in the original
- blocking sockets + `select()`, no `O_NONBLOCK`, libc only, no heap allocation

## Development

```sh
./dev.sh                                                     # Debian 12 shell with traceroute, tcpdump, valgrind
./dev.sh bash -c 'make && bash tests/compare.sh'             # edge cases vs real traceroute
PRIVILEGED=1 ./dev.sh bash -c 'make && bash tests/netns.sh'  # multi-hop test on a namespace router chain
```
