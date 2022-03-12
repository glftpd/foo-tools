#!/bin/sh

. $MACRO_DIR/makegen/makegen.cf

dest=$1
mode=$2
owner=$3
group=$4

echo	"install :"
echo	"	touch $dest"
if test "$mode." != "."
then
	echo	"	$chmod $mode $dest"
fi
if test "$owner." != "."
then
	echo	"	$chown $owner $dest"
fi
if test "$group." != "."
then
	echo	"	$chgrp $group $dest"
fi
