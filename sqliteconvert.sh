#! /bin/sh

TEMPLATE="@SHAREDIR@/schema.xml"
ARGS=`getopt if: $*`
INPUT=
IMAGE=

if [ $? -ne 0 ]
then
	echo "usage: $0 [-i] [-f template] schema.sql" 1>&2
	exit 1
fi

set -- $ARGS

while [ $# -ne 0 ]
do
	case "$1" in
	-i)
		IMAGE=1
		shift ;;
	-f)
		TEMPLATE="$2"
		shift ; shift ;;
	--)
		shift ; break ;;
	esac
done

if [ $# -ne 1 ]
then
	echo "missing input filename" 1>&2
	exit 1
fi
INPUT="$1"

DOTFLAGS='-h BGCOLOR="red" -t CELLBORDER="0" -t CELLSPACING="0"'

set -e

if [ -z "$IMAGE" ]
then
	sed -n '1,/@SCHEMA@/p' "$TEMPLATE"
	sqlite2html "$INPUT"
	sqlite2dot $DOTFLAGS "$INPUT" | dot -Tcmapx
	sed -n '/@SCHEMA@/,$p' "$TEMPLATE"
else
	sqlite2dot $DOTFLAGS "$INPUT" | dot -Tpng 
fi
