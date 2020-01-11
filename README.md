## cxterm 5.3.0 released on 2020/1/11.

cxterm source code has stayed on http://cxterm.sourceforge.net for 
quite some years, but it somehow failed to work with Ubuntu Linux recently.

Today I've made it work again on Ubuntu 18.04, by commenting out VEOL2, 
and VLNEXT lines in main.c . I don't know why, but it is working now. Cheers!

Ubuntu's default locale is `zh_CN.UTF-8`. Yes, UTF8 is more popular. If you 
use GB encoding, you may need add GB2312 locale: 

Login as root, add one line `"zh_CN GB2312"` to file: 

    /var/lib/locales/supported.d/local

then run command:

    locale-gen --purge

And add following to your ~/.bashrc:

    if [ "$CHAR_ENCODING" == "GB" ]; then
	    export LC_CTYPE=zh_CN.GB2312
    fi

I don't have time/knowledge to get it working with UTF8 or GBK locale
at this moment.

cxterm 的源码存在于http://cxterm.sourceforge.net, 有些年头了，但最近不再能
用于ubuntu 18.04。本次修复了一下。重新发布于gitee.com .
