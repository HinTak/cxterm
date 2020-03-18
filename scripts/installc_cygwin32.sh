#!/bin/sh
# install cxterm (after compilation)

if [ "$SDIR" = "" ]; then
    SDIR="./scripts"
fi
if [ "$ECHONONL" = "" ]; then
    ECHONONL="echo -n"
fi

INSTALLATION_FILE=cxterm.ins

# functions

mkalldir ()
{
    ALLDIRS=`echo $1 | awk -F/ '{ p=$1; print p; for (i=2; i<=NF; i++) {
		p=sprintf("%s/%s", p, $i); print p } }'`
    for d in $ALLDIRS ; do
	if [ ! -d $d ]; then
	    echo "mkdir $d"
	    mkdir $d || return $status
	fi
    done
}

challmod ()
{
    ALLDIRS=`echo $2 | awk -F/ '{ p=$1; print p; for (i=2; i<=NF; i++) {
		p=sprintf("%s/%s", p, $i); print p } }'`
    for d in $ALLDIRS ; do
	chmod $1 $d 2> /dev/null	# might not have permission
    done
}

do_install ()
{
    for f in $* ; do
	D=$f
    done
    for f in $* ; do
	if [ "$f" = "$D" ]; then
	    continue		# not copying myself
	fi
	if [ -d $f ]; then
	    echo "- copying directory  $f  to  $D"
	    cp -rp $f $D
	elif [ -f $f ]; then
	    echo "- copying  $f  to  $D"
	    cp $f $D
	fi
    done
}

testfullpath ()
{
    case $1 in
	/* )	return 1;;
	* )	echo 'This is not a *fullpath* name, try again.'; return 0;;
    esac
}

# main script

cat << __INSTALL_MENU__

You may install cxterm in the default X window directory or a directory
of your own choice.  If you are a system manager and wish to make cxterm
available to all the users, you may choose to install cxterm in the X
window system directory, along with other X applications.  Or, if you
do not have the privilege or wish to place it in a separately place, you
may install cxterm to any directory of your choice.

 1. Install in directories of your choice.
 2. Install in the system X Window directories (may need "root" access).

 0. Exit cxterm installation.

__INSTALL_MENU__

$ECHONONL "Please choose (1,2,0) [1] : " $NONL
read ans
if [ "$ans" = "" ]; then
    ans="1"
fi
case "$ans" in
1)
    while [ "$exec_dir" = "" ]; do
	echo ""
	echo "Where do you want to put the executable binary files for cxterm?"
	echo "I would suggest $HOME/bin.  You may choose any directory."
	$ECHONONL "Enter the full path name [$HOME/bin] : " $NONL
	read exec_dir
	if [ "$exec_dir" = "" ]; then
	    exec_dir="$HOME/bin"
	fi
	if testfullpath $exec_dir; then
	    exec_dir=""
	    continue
	fi
	mkalldir $exec_dir
	if [ "$?" -ne 0 ]; then
	    echo "The given directory cannot be created."
	    echo "Please double check and try again."
	    exec_dir=""
	fi
    done

    while [ "$dict_dir" = "" ]; do
	echo ""
	echo "Where do you want to put the dictionary files for cxterm?"
	echo "I would suggest $HOME/dict.  You may choose any directory."
	$ECHONONL "Enter the full path name [$HOME/dict] : " $NONL
	read dict_dir
	if [ "$dict_dir" = "" ]; then
	    dict_dir="$HOME/dict"
	fi
	if testfullpath $dict_dir; then
	    dict_dir=""
	    continue
	fi
	mkalldir $dict_dir
	if [ "$?" -ne 0 ]; then
	    echo "The given directory cannot be created."
	    echo "Please double check and try again."
	    dict_dir=""
	fi
    done

    while [ "$man_dir" = "" ]; do
	echo ""
	echo "Where do you want to put the cxterm manual pages?"
	echo "I would suggest $HOME/man.  You may choose any directory."
	$ECHONONL "Enter the full path name [$HOME/man] : " $NONL
	read man_dir
	if [ "$man_dir" = "" ]; then
	    man_dir="$HOME/man"
	fi
	if testfullpath $man_dir; then
	    man_dir=""
	    continue
	fi
	mkalldir $man_dir/man1
	if [ "$?" -ne 0 ]; then
	    echo "The given directory cannot be created."
	    echo "Please double check and try again."
	    man_dir=""
	fi
    done

    while [ "$doc_dir" = "" ]; do
	echo ""
	echo "Where do you want to put the cxterm documentation?"
	echo "I would suggest $HOME/doc/cxterm.  You may choose any directory."
	$ECHONONL "Enter the full path name [$HOME/doc/cxterm] : " $NONL
	read doc_dir
	if [ "$doc_dir" = "" ]; then
	    doc_dir="$HOME/doc/cxterm"
	fi
	if testfullpath $doc_dir; then
	    doc_dir=""
	    continue
	fi
	mkalldir $doc_dir
	if [ "$?" -ne 0 ]; then
	    echo "The given directory cannot be created."
	    echo "Please double check and try again."
	    doc_dir=""
	fi
    done

    # FONTS, FONTS, FONTS.
    echo ""
    echo "This cxterm package includes the following Chinese fonts:"
    echo ""
    cat fonts/fonts.alias
    echo ""
    cat << _ABOUT_FONTS_
The one with "gb2312" is GB-encoding (simplified Chinese).
The one with "big5" is Big5-encoding (traditional Chinese).

Usually you should install these fonts in order to use cxterm --
unless your system already has some kinds of similar fonts:

Case 1: X11R6 (and XFree86 3.1) has three pre-installed GB-encoding
Chinese fonts: hanzigb16st, hanzigb16fs, and hanzigb24st.
If you only use GB, you should skip the font installation.

Case 2: if you have used some Chinese X window applications like
older versions of cxterm, xhzview, or Mosaic-l10n, you might have
already installed such fonts as cclib16st and taipei15.  You may
continue using such fonts for cxterm and skip the font installation.

_ABOUT_FONTS_

    HAVE_FONTS=0
    if [ "$DISPLAY" != "" ]; then

	$ECHONONL "Press return to continue: " $NONL
	read ans

	echo "Checking Chinese fonts in your X window ..."

	# keep it simple & stupid!
	GB_FONTS="`xlsfonts '*-gb2312*' 2>/dev/null`"
	if [ "$GB_FONTS" != "" ]; then
	    echo "... found the following GB fonts:"
	    echo "$GB_FONTS"
	    HAVE_FONTS=1
	fi
	B5_FONTS="`xlsfonts '*-big5*' 2>/dev/null`"
	if [ "$B5_FONTS" != "" ]; then
	    echo "... found the following Big5 fonts:"
	    echo "$B5_FONTS"
	    HAVE_FONTS=1
	fi
	if [ "$HAVE_FONTS" -eq 1 ]; then
	    echo "Since you do have Chinese fonts in your X window,"
	    echo "you do not have to re-install them again."
	    if [ "`xlsfonts 'hku-ch16' 2>/dev/null`" != "" ]; then
		echo ''
		echo 'Warning: you have a "hku-ch16" font that is obsoleted'
		echo 'and should be replaced by the new "hku16et"'
		echo ''
	    fi
	else
	    # check for non-standard Chinese fonts
	    FONTS="`xlsfonts | awk '/cclib/ 	{ print; next }
				/beijing[0-9]/	{ print; next }
				/taipei[0-9]/	{ print; next }' 2>/dev/null`"
	    if [ "$FONTS" != "" ]; then
		echo "...  these fonts in your X window smell like Chinese:"
		echo "$FONTS"
		echo "You may use these fonts for cxterm, but you'd better"
		echo "install the fonts that come with cxterm too."
	    else
		echo "Cannot find any Chinese font."
		echo "You may answer 'y' below to install the fonts."
	    fi
	fi
    fi
    if [ "$DISPLAY" != ":0.0" ]; then
	cat << _FONT_REMOTE_WARNING_

WARNING:  If you are now using a remote machine that is different from
your X Window display host, this font installation script may be incorrect.
Usually you should install fonts only in the same host as your X Window.

_FONT_REMOTE_WARNING_
    fi

    if [ "$HAVE_FONTS" -eq 1 ]; then
	a='n'
    else
	a='y'
    fi
    $ECHONONL "Do you want to do the font installation (y/n) [$a] ? " $NONL
    read ans
    if [ "$ans" = "" ]; then
	ans="$a"
    fi
    case "$ans" in
    y* | Y* )
	while [ "$font_dir" = "" ]; do
	    echo ""
	    echo "Where do you want to put the X11 font files?"
	    echo "I would suggest $HOME/xfonts.  You may choose any directory."
	    $ECHONONL "Enter the full path name [$HOME/xfonts] : " $NONL
	    read font_dir
	    if [ "$font_dir" = "" ]; then
	        font_dir="$HOME/xfonts"
	    fi
	    if testfullpath $font_dir; then
		font_dir=""
		continue
	    fi
	    mkalldir $font_dir
	    if [ "$?" -ne 0 ]; then
		echo "The given directory cannot be created."
		echo "Please double check and try again."
		font_dir=""
	    fi
	done
	;;
    *)
	font_dir=""
	echo "Font installation skipped"
	;;
    esac

    echo "Installing binaries ..."
    do_install cxterm/cxterm.exe utils/hzimctrl.exe utils/tit2cit.exe utils/cit2tit.exe \
		$exec_dir
    mv $exec_dir/cxterm.exe $exec_dir/cxterm_exe.exe

    echo "Installing dictionaries ..."
    for i in gb big5 jis ks ; do
	if [ -d dict/$i ]; then
	    if [ ! -d $dict_dir/$i ]; then
		echo "mkdir $dict_dir/$i"
		mkdir $dict_dir/$i || continue
	    fi
	    do_install dict/$i/*.cit dict/$i/*.lx $dict_dir/$i
	fi
    done

    echo "Installing manual pages ..."
    for f in cxterm/*.man utils/*.man ; do
	if [ -f $f ]; then
	    f1=$man_dir/man1/`basename $f .man`.1
	    echo "- copying  $f  to  $f1"
	    cp $f $f1
	fi
    done

    echo "Installing documents ..."
    do_install Doc/* $doc_dir
   
    # FONTS, FONTS, FONTS ...

    if [ "$font_dir" != "" ]; then
	echo "Installing fonts ..."
	do_install `awk 'NR > 1 { print "fonts/"$1 }' fonts/fonts.dir` \
		fonts/fonts.alias fonts/fonts.dir $font_dir
	chmod go+r $font_dir/*			# maybe needed by XDM, etc. 

	echo ""
	echo "Font installation is sometimes very tricky, especially"
	echo "when you use X terminals (instead of real workstations)."
	echo ""
	echo "Now verifying the font installation.  If any error happened"
	echo "during the verification, the font installation had failed."
	echo ""

	$ECHONONL "Press return to continue: " $NONL
	read ans

	FONT=hanzigb16st

	GO_ON=1

	echo "step 1, adding the font path ..."

        xset fp- $font_dir 2>/dev/null		#  first took it off if any
	echo "xset fp+ $font_dir" >> Install.log
	xset fp+ $font_dir  2>&1 | tee -a Install.log
	if [ "$?" -ne 0 ]; then
	    echo "Failed to add font path."

	    # guessing what was wrong
	    ALLDIRS=`echo $font_dir | awk -F/ '
		{ p=$1; print p; for (i=2; i<=NF; i++)
		  { p=sprintf("%s/%s", p, $i); print p } }'`
	    for d in $ALLDIRS ; do
		p=`dirname $d`;  b=`basename $d`
		if [ "`find $p -name $b -perm -0011`" = "" ]; then
		    cat << _FONTDIR_NOPERM_
The directory "$font_dir" is not globally executable.
This may cause problems in some X window configuration.
For example, if you login from XDM, the X Window started by XDM
might not be "owned" by you.  The X Window server might not have
permission to access the font directory.  To correct this, you
need to make every components in the font path globally executable.
_FONTDIR_NOPERM_
		    break;
		fi
	    done
	    GO_ON=0
	fi
	sleep 1

	if [ "$GO_ON" -eq 1 ]; then
	    echo "step 2, checking the fontname in the path ..."

	    if [ "`xlsfonts $FONT 2>/dev/null`" = "" ]; then
		echo "Failed to list the font."
		GO_ON=0
	    fi
	    sleep 1
	fi

	if [ "$GO_ON" -eq 1 ]; then
	    echo "step 3, display the font ..."
	    cat << _XFD_MSG_
A window will popup to display nearly a half page of Chinese characters.
Please take a look.  Then click (with the left mouse button) in the
little  [Quit]  box to exit that window.
_XFD_MSG_
	    sleep 2
	    xfd -fn $FONT -start 12288  2>&1 | tee -a Install.log
	    if [ "$?" -ne 0 ]; then
		echo "Failed to display the font."
		GO_ON=0
	    fi
	fi

	fontsrcdir=`pwd`/fonts/
	if [ "$GO_ON" -eq 1 ]; then
	    cat << _FONT_VERIFICATION_ 

Font verification done.  If everything goes well, and you have seen some
Chinese characters in the popup window, the font installation is successful.
Write down the following line.  You may need to add it to your X window
startup files (follow the README file under "3. HOW TO RUN CXTERM").

        xset fp+ $font_dir

However if you got any error or problem during the above steps, the font
installation may have failed.  Check Doc/CXTERM.FAQ for more information.
Ask your system manager or local X gurus to help you --- tell them that
you have some X11 fonts in directory $fontsrcdir
and you wish to install them to  $font_dir

_FONT_VERIFICATION_

	else
	    cat << _FONT_FAILED_ 

Seems like the font installation have failed.
Check Doc/CXTERM.FAQ on what might go wrong.
Ask your system manager or local X gurus to help you --- tell them that
you have some X11 fonts in directory $fontsrcdir
and you wish to install them to  $font_dir

_FONT_FAILED_

	    xset fp- $font_dir 2>/dev/null
	fi

	$ECHONONL "Press return to continue: " $NONL
	read ans
    fi

    # set up the resource file
    sed -e 's|^\(cxterm[*.].*hanziInputDir:\).*$|\1	'"$dict_dir"'/gb|;
	    s|^\(cxtermb5[*.].*hanziInputDir:\).*$|\1	'"$dict_dir"'/big5|
	    s|^\(cxtermjis[*.].*hanziInputDir:\).*$|\1	'"$dict_dir"'/jis|
	    s|^\(cxtermks[*.].*hanziInputDir:\).*$|\1	'"$dict_dir"'/ks|' \
		cxterm/CXterm.ad > $dict_dir/CXterm.ad

    # save these ...
    echo "BINDIR=$exec_dir"			>> $INSTALLATION_FILE
    if [ "$font_dir" != "" ]; then
	echo "XFONTDIR=$font_dir"		>> $INSTALLATION_FILE
    fi
    echo "HZINPUTDIR=$dict_dir"			>> $INSTALLATION_FILE
    echo "CXTERM_AD=$dict_dir/CXterm.ad"	>> $INSTALLATION_FILE


    # for newbie ...
    echo "creating  $exec_dir/CXterm  for inexperienced user"
    CXTERM_SH=$exec_dir/CXterm

    echo "#!/bin/sh"				>  $CXTERM_SH
    echo "CXTERM_EXE=cxterm_exe.exe"		>> $CXTERM_SH
    echo "BINDIR=$exec_dir"			>> $CXTERM_SH
    if [ "$font_dir" = "" ]; then
	echo "XFONTDIR=$HOME/xfonts"		>> $CXTERM_SH
    else
	echo "XFONTDIR=$font_dir"		>> $CXTERM_SH
    fi
    echo "CXTERM_AD=$dict_dir/CXterm.ad"	>> $CXTERM_SH
    cat $SDIR/CXterm.sh				>> $CXTERM_SH
    chmod a+x $CXTERM_SH


    exit 0
    ;;

2)
    make install	2>&1 | tee -a Install.log
    make install.man	2>&1 | tee -a Install.log
    $SDIR/installCX.sh
    exit 0
    ;;
*)
    exit 0
    ;;
esac

