#!/bin/sh

#   do_module   -> to run: ./tester <cfg> <path> <file>
#   test_module -> to run: ./tester mod_name.so <file> <path>

make clean && make && make tester
./tester ./mod_audiosort.so "01-formek-danger_situations_(original_mix)-48941a85.mp3" /jail/glftpd/site/groups/CoolGroup/Whatever-REL
