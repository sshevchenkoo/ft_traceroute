#!/bin/sh
# Dev shell: Debian 12 with the reference traceroute (Dmitry Butskoy's, same as
# the one evaluators use), tcpdump and valgrind. Project mounted at /ft_traceroute.
# Usage: ./dev.sh            -> interactive shell
#        ./dev.sh <command>  -> run one command
#        PRIVILEGED=1 ./dev.sh bash tests/netns.sh  -> multi-hop test
set -e

IMAGE=ft_traceroute-dev

docker build -q -t "$IMAGE" - >/dev/null <<'DOCKERFILE'
FROM debian:12
RUN apt-get update && apt-get install -y --no-install-recommends \
	build-essential traceroute tcpdump valgrind iproute2 \
	&& rm -rf /var/lib/apt/lists/*
WORKDIR /ft_traceroute
DOCKERFILE

EXTRA=""
[ -n "$PRIVILEGED" ] && EXTRA="--privileged"
DIR="$(cd "$(dirname "$0")" && pwd)"
if [ $# -eq 0 ]; then
	exec docker run -it --rm $EXTRA -v "$DIR":/ft_traceroute "$IMAGE" bash
fi
exec docker run --rm $EXTRA -v "$DIR":/ft_traceroute "$IMAGE" "$@"
