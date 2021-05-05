#!/bin/bash -x

./logger
./logger -h
./logger ls
./logger ls 2>/dev/null

./logger -o ls_al.txt -- ls -al
cat ls_al.txt
rm -f ls_al.txt