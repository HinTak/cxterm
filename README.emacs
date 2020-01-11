Running emacs embedded within the cxterm environment with cemacs.el
===================================================================

I was having problem building emacs 19.34 on Slackware Linux 9 (whereas
an old binary compiled on Slackware 8 continued to work), and asked
on emacs-devel, and it led to a length discussion of MULE and other issues.
In the end, I got much more than what I wanted: letting the MULE developers
know how MULE compares less well with cxterm/cemacs (so that MULE can improve),
and sorted out problem with cemacs so that it works with all versions of 
emacs from 19 onwards, not just 19 and 18. 

In the emacs/ sub-directory, there is a cemacs.el script which has been updated
to include this development, and 3 versions of older cemacs.el from 
Stephen G. Simpson, Jyi-Jiin Luo, and Ju Li. The updated script is primarily
based on JJ Luo's (ditto Ju Li's) ; functionally there is little difference, 
but personally I found JJ Luo's cursor movement and screen refresh feels 
"better", except for the annoying help screen, which I disabled in the updated 
version. However, Simpson's had been perfectly adequate previously for years.

So here are some hints to running various versions of emacs with cemacs.el:

(0) v18, v19: Simpson still has older versions of cemacs.el on his web site,
                if you can get emacs to compile. 

(1) v19.34: There are various glibc2-related patches around to get it to compile
           on current glibc2-based systems.

(2) v19.34-v21: In March 2002, the default behaviour of GNU ld has changed to
                'combreloc', which breaks emacs. newer emacs's configure will add 
                '-z nocombreloc' to the linder flags on linux, but to build any 
                emacs before March 2002 with a new ld, one needs to define 
                LDFLAGS='-z nocombreloc'. Otherwise, the binary will segfault in
                X (irrelevant to running under cxterm, but still annoying).

(3) v20.1-v20.2: Kindly pointed out by Stephan Monnier (a core emacs developer)
                 it is possible to get the pre-MULE behaviour with    
                           (setq-default enable-multibyte-characters nil)
                 [Included in the updated cemacs.el script] 

(4) v20.3-v21.x: in addition to the one-liner in (3), from v20.3 onwards, one 
                 can get the older pre-MULE behaviour with the '--unibyte' command
                 line switch. Doing both causes no harm.

Have fun editing.

Hin-Tak Leung
Written on 13 June 2003.

=============================================================================
The lisp function 'set-buffer-multibyte' (from v20.3) affects the scratch buffer 
on start-up, and so does the environment variable EMACS_UNIBYTE.

Updated on 08 August 2004.

