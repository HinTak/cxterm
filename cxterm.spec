Summary: Chinese X-term
Name: cxterm
Version: 5.2.4
Release: 1unix98
Packager: Hin-Tak Leung <htl10@users.sourceforge.net>
Source: http://osdn.dl.sourceforge.net/sourceforge/cxterm/cxterm-%{version}.tgz

License: distributable
Group: X11/Utilities/terms
BuildRoot: /tmp/cxterm-root
%description
A terminal emulator for X11, just like "xterm", but with the
capability of displaying and inputting Chinese.

It supports GB, Big5, JIS, and KS encoding.  HZ support is through
"hztty", a separated program avaliable from:
     ftp://ftp.cs.purdue.edu/pub/ygz/hztty.tar.gz

%package big5
Group: X11/Utilities/terms
Summary: big5 Traditional
%description big5
Chinese X-term for Big5

%package gb
Group: X11/Utilities/terms
Summary: GB Simplified
Requires: cxterm
%description gb
Chinese X-term for GB

%package jis
Group: X11/Utilities/terms
Summary: JIS Japanese
Requires: cxterm
%description jis
Japanese X-term for JIS

%package ks
Group: X11/Utilities/terms
Summary: KS Korean
Requires: cxterm
%description ks
Korean X-term for KS

%package fonts
Group: X11/Utilities/terms
Summary: Fonts
%description fonts
Fonts used by cxterm

%prep

%setup 

%build
CFLAGS="-Wall -O3" ./configure --prefix=/usr/X11R6
make

%install
DESTDIR=$RPM_BUILD_ROOT make install

mkdir -p $RPM_BUILD_ROOT/usr/X11R6/lib/X11/fonts/chinese
install -c -m 644 fonts/*.gz fonts/font* \
  $RPM_BUILD_ROOT/usr/X11R6/lib/X11/fonts/chinese

ln -sf cxterm $RPM_BUILD_ROOT/usr/X11R6/bin/cxtermgb
ln -sf cxterm $RPM_BUILD_ROOT/usr/X11R6/bin/cxtermb5
ln -sf cxterm $RPM_BUILD_ROOT/usr/X11R6/bin/cxtermjis
ln -sf cxterm $RPM_BUILD_ROOT/usr/X11R6/bin/cxtermks
ln -sf ../../lib/X11/fonts/chinese $RPM_BUILD_ROOT/usr/X11R6/share/cxterm/fonts

%clean
rm -fr $RPM_BUILD_ROOT

%post

%postun

%files
%defattr(-, root, root)

%doc Doc README* INSTALL-5.2 ChangeLog cxterm.term* emacs
%dir /usr/X11R6/share/cxterm/dict
%config /usr/X11R6/share/cxterm/cxtermrc

/usr/X11R6/bin/cit2tit
/usr/X11R6/bin/cxterm
/usr/X11R6/bin/cxterm.bin
/usr/X11R6/bin/hzimctrl
/usr/X11R6/bin/tit2cit

/usr/X11R6/man/man1/*.1*

%files big5
%attr(-,root,root) /usr/X11R6/bin/cxtermb5
%attr(-,root,root) /usr/X11R6/share/cxterm/dict/big5

%files gb
%attr(-,root,root) /usr/X11R6/bin/cxtermgb
%attr(-,root,root) /usr/X11R6/share/cxterm/dict/gb

%files jis
%attr(-,root,root) /usr/X11R6/bin/cxtermjis
%attr(-,root,root) /usr/X11R6/share/cxterm/dict/jis

%files ks
%attr(-,root,root) /usr/X11R6/bin/cxtermks
%attr(-,root,root) /usr/X11R6/share/cxterm/dict/ks

%files fonts
%attr(-,root,root) /usr/X11R6/lib/X11/fonts/chinese
%attr(-,root,root) /usr/X11R6/share/cxterm/fonts

%changelog
* Mon May  5 2003 Hin-Tak Leung <htl10@users.sourceforge.net>
- RPMS packaging since a long while

