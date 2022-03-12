#!/bin/sh

. $MACRO_DIR/makegen/makegen.cf

DIR=$1
TARGETS=`echo $2 | sed "s/,/ /g"`

if test $OS_TYPE = "OS2"
then
	makevars="SHELL=SH"
fi
if test $OS_TYPE = "UNIX"
then
	makevars="SHELL=/bin/sh"
fi


for targ in $TARGETS
do
	echo "$targ ::"
	echo "	@echo					;\\"
	echo "	echo \"--> [$DIR] directory  \"        	;\\"
	echo "	cd $DIR                            	;\\"
	echo "	\$(MAKE) $makevars \$(MFLAGS)	$targ	;\\"
	echo "	echo \"<-- [$DIR] done       \"		"
	echo ""
done
