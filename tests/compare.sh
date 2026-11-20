#!/bin/bash
# Edge cases: compare stdout, stderr and exit code with the real traceroute.
# RTT values are masked. Traces are compared with `traceroute -n`, because
# the mandatory part prints hops without reverse DNS.
# Run: make && ./dev.sh bash tests/compare.sh
mask() { sed -E 's/[0-9]+\.[0-9]{3} ms/X ms/g'; }

# $1 = extra args for the real traceroute ("" or "-n"), rest = arguments
run() {
  real_opt=$1; shift
  ./ft_traceroute "$@" >/tmp/a.o 2>/tmp/a.e; ra=$?
  [ -n "$real_opt" ] && sleep 2   # let routers refill their ICMP rate limit
  traceroute $real_opt "$@" >/tmp/b.o 2>/tmp/b.e; rb=$?
  if [ $ra = $rb ] && diff <(mask </tmp/a.o) <(mask </tmp/b.o) >/dev/null \
     && diff <(mask </tmp/a.e) <(mask </tmp/b.e) >/dev/null; then
    echo "OK   [$*] rc=$ra"
  else
    echo "DIFF [$*] rc ft=$ra real=$rb"
    echo "-- ft:";   cat /tmp/a.o /tmp/a.e | head -8
    echo "-- real:"; cat /tmp/b.o /tmp/b.e | head -8
  fi
}

# help: same exit code and stream (stderr), text is shorter on purpose
for args in "" "--help" "8.8.8.8 --help"; do
  ./ft_traceroute $args >/tmp/a.o 2>/tmp/a.e; ra=$?
  traceroute $args >/tmp/b.o 2>/tmp/b.e; rb=$?
  if [ $ra = $rb ] && [ ! -s /tmp/a.o ] && [ ! -s /tmp/b.o ] && [ -s /tmp/a.e ]; then
    echo "OK   [$args] help on stderr, rc=$ra"
  else
    echo "DIFF [$args] help rc ft=$ra real=$rb"
  fi
done

# argument errors
run "" --
run "" --hel
run "" -Z 8.8.8.8
run "" -
run "" ""
run "" 300.1.1.1
run "" nosuchhost.invalid
run "" 8.8.8.8 abc
run "" a b c

# traces
run -n -- 127.0.0.1
run -n 127.0.0.1
run -n 0.0.0.0
run -n localhost
run -n "$(hostname -i | cut -d' ' -f1)"
run -n "$(ip -4 route | awk '/default/ {print $3}')"
run -n "$(ip -4 route | awk '/default/ {split($3, a, "."); print a[1]"."a[2]"."a[3]".99"}')"
