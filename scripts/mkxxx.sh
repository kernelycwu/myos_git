#!/bin/bash
dd if=/dev/zero of=xxx.img bs=4096  count=32768
gcc mkxxx.c -o mkxxx -g
./mkxxx xxx.img kernel
