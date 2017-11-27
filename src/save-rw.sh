#!/bin/sh

if [ ! -f "$1" ]; then
  echo save.sh: not found: $1
  exit
fi

cp $1 /etc/nshrc
#cp /var/run/pf.conf /etc/pf.conf
sync
