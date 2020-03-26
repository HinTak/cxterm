Summary: Chinese X-term
Name: cxterm
Version: 5.2.4
Release: 1unix98
Source: http://download.sourceforge.net/sourceforge/cxterm/cxterm-%{version}.tgz

License: distributable
Group: X11/Utilities/terms
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: gcc pkgconfig ncurses-devel
BuildRequires: libXt-devel libXaw-devel
BuildRequires: xorg-x11-font-utils

%global x11_font_dir %(pkg-config --variable fontrootdir fontutil)

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

%setup -q

%build
%configure --prefix=/usr
make

%install
DESTDIR=$RPM_BUILD_ROOT make install

mkdir -p $RPM_BUILD_ROOT/%{x11_font_dir}/chinese
install -c -m 644 fonts/*.gz fonts/font* \
  $RPM_BUILD_ROOT/%{x11_font_dir}/chinese

ln -sf cxterm $RPM_BUILD_ROOT/usr/bin/cxtermgb
ln -sf cxterm $RPM_BUILD_ROOT/usr/bin/cxtermb5
ln -sf cxterm $RPM_BUILD_ROOT/usr/bin/cxtermjis
ln -sf cxterm $RPM_BUILD_ROOT/usr/bin/cxtermks
ln -sf %{x11_font_dir}/chinese $RPM_BUILD_ROOT/usr/share/cxterm/fonts

%clean
rm -fr $RPM_BUILD_ROOT

%post

%postun

%files
%defattr(-, root, root)

%doc Doc README* INSTALL-5.2 ChangeLog cxterm.term* emacs
%dir /usr/share/cxterm/dict
%config /usr/share/cxterm/cxtermrc

/usr/bin/cit2tit
/usr/bin/cxterm
/usr/bin/cxterm.bin
/usr/bin/hzimctrl
/usr/bin/tit2cit

/usr/man/man1/*.1*

%files big5
%attr(-,root,root) /usr/bin/cxtermb5
%attr(-,root,root) /usr/share/cxterm/dict/big5

%files gb
%attr(-,root,root) /usr/bin/cxtermgb
%attr(-,root,root) /usr/share/cxterm/dict/gb

%files jis
%attr(-,root,root) /usr/bin/cxtermjis
%attr(-,root,root) /usr/share/cxterm/dict/jis

%files ks
%attr(-,root,root) /usr/bin/cxtermks
%attr(-,root,root) /usr/share/cxterm/dict/ks

%files fonts
%attr(-,root,root) %{x11_font_dir}/chinese
%attr(-,root,root) /usr/share/cxterm/fonts

%changelog
* Mon May  5 2003 Hin-Tak Leung <htl10@users.sourceforge.net>
- RPMS packaging since a long while

