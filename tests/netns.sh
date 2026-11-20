#!/bin/bash
# Multi-hop test without Internet: builds a chain of routers out of network
# namespaces and compares ft_traceroute with `traceroute -n`.
#
#   client 10.0.1.1 -- r1 -- 10.0.2.0/24 -- r2 -- 10.0.3.2 target
#   on r2: 10.9.1.0/24 prohibit (!X), 10.9.2.0/24 unreachable (!H),
#          10.9.3.0/24 blackhole (* * * up to 30 hops)
#
# Run: make && PRIVILEGED=1 ./dev.sh bash tests/netns.sh
set -e
for n in r1 r2 tgt; do ip netns add $n; done
ip link add c0 type veth peer name r1a netns r1
ip -n r1 link add r1b type veth peer name r2a netns r2
ip -n r2 link add r2b type veth peer name t0 netns tgt
ip addr add 10.0.1.1/24 dev c0; ip link set c0 up
ip -n r1 addr add 10.0.1.2/24 dev r1a; ip -n r1 addr add 10.0.2.1/24 dev r1b
ip -n r2 addr add 10.0.2.2/24 dev r2a; ip -n r2 addr add 10.0.3.1/24 dev r2b
ip -n tgt addr add 10.0.3.2/24 dev t0
for n in r1 r2 tgt; do ip -n $n link set lo up; done
ip -n r1 link set r1a up; ip -n r1 link set r1b up
ip -n r2 link set r2a up; ip -n r2 link set r2b up
ip -n tgt link set t0 up
ip netns exec r1 sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
ip netns exec r2 sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
ip route add 10.0.0.0/8 via 10.0.1.2
ip -n r1 route add 10.0.3.0/24 via 10.0.2.2
ip -n r1 route add 10.9.0.0/16 via 10.0.2.2
ip -n r2 route add default via 10.0.2.1
ip -n tgt route add default via 10.0.3.1
ip -n r2 route add prohibit 10.9.1.0/24
ip -n r2 route add unreachable 10.9.2.0/24
ip -n r2 route add blackhole 10.9.3.0/24
# no ICMP rate limiting on the routers, otherwise back-to-back runs get "*"
for n in r1 r2 tgt; do ip netns exec $n sh -c "echo 0 > /proc/sys/net/ipv4/icmp_ratelimit"; done
set +e
mask() { sed -E 's/[0-9]+\.[0-9]{3} ms/X ms/g'; }
for t in 10.0.3.2 10.0.2.2 10.9.1.1 10.9.2.1 10.9.3.1; do
  sleep 2; s=$(date +%s); ./ft_traceroute $t > /tmp/a 2>&1; e=$(date +%s); traceroute -n $t > /tmp/b 2>&1; f=$(date +%s)
  if diff <(mask </tmp/a) <(mask </tmp/b) >/dev/null; then echo "OK   $t  "; else echo "DIFF $t"; cat /tmp/b | head -6; fi
  echo "  ft $((e-s))s, real $((f-e))s"; head -5 /tmp/a
done
