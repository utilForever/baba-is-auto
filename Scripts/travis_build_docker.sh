#!/usr/bin/env bash

set -e

if [ $# -eq 0 ]
  then
    docker build -t utilforever/baba-is-auto .
else
    docker build -f $1 -t utilforever/baba-is-auto:$2 .
fi
docker run utilforever/baba-is-auto