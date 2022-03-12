#!/bin/sh

. $MACRO_DIR/makegen/makegen.cf

src=$1
outfile=`echo $src | sed "s/\.l\$/.c/"`
obj=`echo $src | sed "s/\.l\$/.o/"`

echo	"$outfile : $src"
echo	'	$(LEX) $(LEX_FLAGS)'" $src"
echo	"	mv lex.yy.c $outfile"
echo
echo	"$obj : $outfile"
echo    '	$(CC) $(CC_FLAGS)'" -c $outfile"
echo
echo	"clean ::"
echo	"	rm -f $outfile lex.yy.*"
echo
