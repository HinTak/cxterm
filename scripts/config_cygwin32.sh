#!/bin/sh

# definitions

SDIR="./scripts"
CXTERMNAME="CXTERM 5.1"

## which echo?
ECHONONL="echo"
NONL=""

AB=`echo -n "a"; echo "b"`
if [ "$AB" = "ab" ]; then
    ECHONONL="echo -n"
else
    ## echo ...\c ?
    AB=`echo "a\c"; echo "b"`
    if [ "$AB" = "ab" ]; then
	NONL="\c"
    fi
fi

test -f version  &&  CXTERMNAME="`head -1 version`"

export SDIR CXTERMNAME ECHONONL	NONL		# for subshell calls


## Main menu

cat > /dev/tty << __END__

CXTERM VERSION 5.1
---------------------------------------------------------------
 * Originally written by Yongguang Zhang (C) 1995
 * Color patches by Ji-Tzay Yang and Bruce Cheng in 1996-1997
 * Blink/input patches and bug fixes by ZHUANG Hao, 1997-1999
---------------------------------------------------------------

Please read the COPYRIGHT file on the copyright and permission
notices.  If you do not agree the terms, stop by pressing <Ctrl-C>.

To make things smooth, you'd better run this configuration
program under X Window.  You may type <Ctrl-C> any time to
stop the configuration.

You need to answer several questions, so don't run this program
in background or redirect the output.  Multiple choices are
listed in parentheses (), and default or suggested answers are
given in brackets [].  The error messages during the make will
be saved automatically in "./Install.log".

__END__

$ECHONONL 'Continue with config.sh (y/n) [y] ?' $NONL
read ans
case "$ans" in
    n* | N* )	exit ;;
esac

if [ "$DISPLAY" = "" ]; then
    $ECHONONL 'You are not in X window, still continue with config.sh (y/n) [n] ?' $NONL
    read ans
    case "$ans" in
	y* | Y* )	;;
	* )	exit ;;
    esac
fi

## Main loop

while true ; do

    # 1.0 Main menu
    cat << __MAIN_MENU__

-----------------------------------------------------------------------------
    --- BASIC MENU ---
  0. Read COPYRIGHT Notice
  1. Compile, Install, and Configure "$CXTERMNAME" in One Step

    --- OPTION MENU ---
  2. Compile cxterm (not to install)
  3. Install cxterm (after successful compilation in 2)
  4. Install additional Chinese font(s) for your X window
  5. Configure your account for using cxterm (after installation in 3)

  x. Exit
-----------------------------------------------------------------------------
__MAIN_MENU__

    $ECHONONL 'Please choose (0/1/2/3/4/5/x) :' $NONL
    read ans
    case "$ans" in
    0)
	more ./COPYRIGHT
	;;

    1)
	echo "1.1  compiling cxterm ..."
	$SDIR/compile.sh || exit 1		# if compile failed

	echo ""
	echo "1.2  installing cxterm ..."
	rm -f cxterm.ins
	touch cxterm.ins
	$SDIR/installc_cygwin32.sh
	# don't care about installation error

	# want to setup your account for cxterm ?
	echo ""
	$ECHONONL 'Do you want to set up your account (y/n) [n] ?' $NONL
	read ans
	if [ "$ans" = "" ]; then
	    ans="n"
	fi
	case "$ans" in
	Y* | y* )
	    echo "1.4  setting up your account ..."
	    $SDIR/setup.sh
	    ;;
	esac

	echo ""
	echo "$CXTERMNAME installation completed."
	exit 0

	;;

    2)
	$SDIR/compile.sh || exit 1		# if compile failed
	;;

    3)
	$SDIR/installc_cygwin32.sh || exit 1		# if install failed
	;;

    4)
	$SDIR/installf.sh || exit 1		# if install failed
	;;

    5)
	$SDIR/setup.sh || exit 1		# if setup failed
	;;

    x)
	exit 0;
	;;

    *)
	echo "Unknown option.  Try again."
	echo ""
	;;

    esac
done

