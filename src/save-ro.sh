#!/bin/sh

if [ ! -f "$1" ]; then
  echo save.sh: not found: $1
  exit
fi

rw
cp $1 /etc/nshrc
sync
ro
