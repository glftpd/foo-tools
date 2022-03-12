#!/bin/sh

. $MACRO_DIR/makegen/makegen.cf

src=$1
shift
comment=$*
cfile=`echo $src | sed "s/\.y\$/.c/"`
hfile=`echo $src | sed "s/\.y\$/.h/"`
obj=`echo $src | sed "s/\.y\$/.o/"`

echo	"$cfile : $src"
echo	"	@echo \"$comment\""
echo	'	$(YACC) $(YACC_FLAGS)'" $src"
echo	"	mv y.tab.c $cfile"
echo	"	mv y.tab.h $hfile"
echo
echo	"$obj : $cfile"
echo	'	$(CC) $(CC_FLAGS) -c $(SOURCE_DIR)/'$cfile
echo
echo	"clean ::"
echo	"	rm -f $cfile $hfile y.tab.*"
echo
