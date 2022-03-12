#!/bin/sh
#
# This makegen macro contributed by Yuri Dario <mc6530@mclink.it> for
# use with OS2 DLL's
#

. $MACRO_DIR/makegen/makegen.cf

if test $OS_TYPE != "_OS_OS2"
then
	exit
fi


dll=$1
dllsrc=`echo $2 | sed "s/,/ /g"`
dllobj=`echo $3 | sed "s/,/ /g"`
dlllib=`echo $4 | sed "s/,/ -l/g"`
shift

for src in $dllsrc
do
	if test -c /dev/tty
	then
		$ECHO_N ".$ECHO_C" > /dev/tty
	fi
	base=`echo $src | sed "s/\..*//"`
	obj=`echo $src | sed "s/\.c\$/.o/"`
	dllobj="$dllobj $obj"
	echo	"$obj : $src Makefile.full"
	echo	'	$(CC) $(CC_FLAGS) -o '$obj' -c $(SOURCE_DIR)/'$src
	echo
	echo	"clean ::"
	echo	"	rm -f $obj"
	echo
done

echo
echo	"# Make rules for building $dll"
echo
echo	"all : $dll.dll"
echo
echo	"$dll.dll : $dllobj Makefile.full"
echo	'ifeq ($(OS_NAME),OS_2)'
echo	'	$(subst Zexe,Zdll,$(LINK)) $(CC_FLAGS)'" $dllobj -o $dll.dll $dll.def -L"'$(INST_DIR)/lib $(LD_LIBS) '$dlllib
echo	'	emximp -o ' $dll'.lib ' $dll'.def'
echo	'	cp '$dll'.lib ' lib$dll'.a'
echo	"else"
echo	"	@echo DLL not supported on this platform."
echo	"endif"
echo
echo	"clean :: "
echo	"	rm -f $dll"
echo
