#!/bin/sh

# wrapper for module tester

mod="mod_audiosort.so"
path="/jail/glftpd/site/incoming/mp3/Whatever-REL"
file="01-test.mp3"

{ make clean && make ${mod}.debug && make tester; } || { echo "make failed, exiting"; exit 1; }
cp ${mod}.debug $mod
echo

# test_module : ./tester <cfg> mod_name.so <file> <path>
if [ "$1" = "strace" ]; then
  strace ./tester testpre.cfg "./$mod" "$file" "$path"
elif [ "$1" = "gdb" ]; then
  gdb --args ./tester testpre.cfg "./$mod" "$file" "$path"
else
 ./tester testpre.cfg "./$mod" "$file" "$path"
fi

# FIXME:
# do_module   : ./tester <cfg> <path> <file>
#./tester testpre.cfg $path $file

