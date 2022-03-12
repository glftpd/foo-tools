#!/bin/sh

. $MACRO_DIR/makegen/makegen.cf

prog=$1
progobjs=`echo $2 | sed "s/,/ /g"`
proglibs=`echo $3 | sed "s/,/ /g"`
depends=`echo $4 | sed "s/,/ /g"`
shift

echo
echo	"# Make rules for building $prog"
echo
echo	"all :: $prog"
echo
echo	"$prog : $progobjs Makefile.full $depends"
echo	'	$(LINK) $(CC_FLAGS)'" $progobjs -o $prog "'$(LD_LIBS)'" $proglibs"
echo
echo	"clean :: "
echo	"	rm -f $prog"
echo
