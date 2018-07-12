#!/bin/sh

# test_module : ./tester mod_name.so <file> <path>
# do_module   : ./tester <cfg> <path> <file>
# gdb --args ./tester ...

mod="mod_audiosort.so"
path="/jail/glftpd/site/incoming/mp3/Whatever-REL"
file="01-test.mp3"

#make clean && make && make tester
make clean && make $mod && make tester
echo
set -x 

#test_module
#./tester ./$mod $file $path

#do_module
./tester testpre.cfg $path $file

