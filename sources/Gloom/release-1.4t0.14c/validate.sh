#!/bin/sh
if objdump -T `make -s name | grep -v director` | grep UND | grep -ve GLIB | grep -v '000000  w'; then
  echo 'ERROR: Unresolved symbols' >&2
  exit 1
fi
