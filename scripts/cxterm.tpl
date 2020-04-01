
NAME=cxterm
ENCODING=GB
INPUTDIR=$DICTDIR/gb
FONT="-fh hanzigb16fs -fhb hanzigb16fs" #XFree86 has this font already

if [ -d $FONTSDIR ]; then
    xset fp+ $FONTSDIR
fi

case "$0" in
    *b5 )
        FONT="-fh hku16et"
        ENCODING=BIG5
        NAME=cxtermb5
        INPUTDIR=$DICTDIR/big5
        ;;
    *ks )
        NAME=cxtermks
        ENCODING=KS
        INPUTDIR=$DICTDIR/ks
        ;;
    *jis )
        NAME=cxtermjis
        ENCODING=JIS
        INPUTDIR=$DICTDIR/jis
        ;;
esac

case "$1" in
	-gb* | -GB* )
		# this is the default configuration
		shift
		;;
	-big5 | -Big5 | -BIG5 | -b5 | -B5 )
		FONT="-fh hku16et"
		ENCODING=BIG5
		NAME=cxtermb5
		INPUTDIR=$DICTDIR/big5
		shift
		;;
	-ks | -KS | -ko | -KO )
		NAME=cxtermks
		ENCODING=KS
		INPUTDIR=$DICTDIR/ks
		shift
		;;
	-jis | -jpn | -jp )
		NAME=cxtermjis
		ENCODING=JIS
		INPUTDIR=$DICTDIR/jis
		shift
		;;
esac

XENVIRONMENT=$CXTERM_RC
export XENVIRONMENT

$CXTERM_BIN -hz $ENCODING -name $NAME -hid $INPUTDIR $FONT \
    -bg black -fg lightgrey -sb -sl 1000 "$@" &
