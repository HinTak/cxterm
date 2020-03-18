;;; cemacs.el --- helps Emacs work on Chinese files within cxterm
;;
;; Author: Stephen G. Simpson <simpson@math.psu.edu>
;;         Jyi-Jiin Luo <jjluo@nwu.edu>
;; Maintainer: Jyi-Jiin Luo <jjluo@nwu.edu>
;; Created: February 1991
;; Last Revised: June 3, 1997
;; Keywords: cemacs chinese big5 bg5 gb
;;
;;
;;; Commentary:
;;    This code enables Emacs to edit Chinese files within cxterm
;;    or other Chinese terminal emulators. It works with both BIG5 and
;;    GB code.
;;
;;    It is assumed that you are running GNU Emacs Release 19.
;;
;;    The code was heavily rewritten by Jyi-Jiin from the original, which
;;    was written by Stephen. Some big5 bugs have been fixed, and new
;;    functions added.
;;
;;    The latest version of this file will be available for anonymous FTP
;;    in qefp486.cqe.nwu.edu:/pub/chinese/cemacs/emacs-19
;;
;;
;;; Bug report
;;
;;    Please direct any bug report to Jyi-Jiin by email to jjluo@nwu.edu
;;
;;
;;; Change log:
;;  05/20/97           Jyi-Jiin Luo
;;             fix an obvious typo in the dummy help page
;;             adding sentence related functions
;;             cemacs-input-lookup now searches for all possible input
;;                 sequences and works when keyprompts are only partially
;;                 specified, like PY.tit for GB codes
;;
;;  05/16/97           Jyi-Jiin Luo
;;             cemacs-help-next added
;;             cemacs-input-lookup added
;;  05/02/97           Jyi-Jiin Luo
;;             rewriting the backward move/delete algorithm
;;             cemacs-fill-region added
;;             cemacs-count-region added
;;  04/15/97           Jyi-Jiin Luo
;;             A fix for mixed big5 Chinese/ASCII files, by Jyi-Jiin
;;  06/22/94           Stephen G. Simpson
;;             Original cemacs.el for emacs 19
;;
;;------------------------
;; Setup and installation
;;------------------------
;;
;;; Basic setup:
;;  ------------
;;
;; Go to your cxterm or other Chinese terminal emulator, start
;; Emacs by 'emacs -nw', and load this file by typing, for instance,
;; 
;;   ESC x load-file RET cemacs.el RET
;; 
;; Your Emacs will now be able to display and edit Chinese files.
;;
;; If you want to automate this procedure, you should place cemacs.el
;; in your site-lisp directory with other Emacs Lisp code, so that
;; your Emacs will be able to load it.  (If you wish, you may
;; byte-compile cemacs.el.)  You may then put the following line into
;; your ~/.cshrc, ~/.profile or whatever startup file for your shell:
;; 
;;    alias cemacs 'emacs -nw -l cemacs'
;; 
;; You will then be able to bring up Emacs in Chinese mode by simply
;; typing "cemacs" (inside cxterm, of course).
;;
;;
;;; Additional setup for more features:
;;  -----------------------------------
;;
;; There are a few features/problems which require further setup. It
;; doesn't hurt if you don't want to use those features.
;;
;; Q1: How do I setup the auto-fill-mode for Chinese?
;; Q2: How do I setup the input lookup function?
;; Q3: How do I get the keyboard layout for my input method?
;; Q4: Can I disable the annoying helper pages?
;; Q5: Why doesn't cemacs work right for my Chinese emulator under GB code?
;; Q6: Why doesn't cemacs find the right Big5 character during search?
;; Q7: Why is the screen garbled when I scroll up, or rotate the helper page?
;; Q8: Why does it look messed up when a line is longer than the window width?
;; Q9: Is there a list of features/functions of cemacs?
;;
;; 
;; Q1: How do I setup the auto-fill-mode for Chinese?
;;
;;    In order for cemacs-auto-fill-mode to work right, you have to make
;;    a simple one-line patch to emacs 19 source and recompile.
;;    See the patch "emacs-19.34b.patch" included in the distribution.
;;
;;      Hint: the following commands would patch your eamcs-19.34b source
;;
;;            cd <your-emacs-source-directory>
;;            patch -p1 < emacs-19.34b.patch
;;
;;      For earlier versions of emacs, you can just do it manually.
;;
;;    Auto-fill-mode can be activated by "ESC x cemacs-auto-fill-mode RET".
;;
;;
;; Q2: How do I setup the input lookup function?
;;
;;    A lot of users need the capability to lookup the input key sequence
;;    for a Chinese character they couldn't figure out, so here it is:
;;    to find the Chinese input key sequence for a Chinese character
;;    under or before cursor, say, for CangJie input method, get
;;    CangJie.tit from cxterm distribution, save it somewhere, and put
;;    the following lines in your ~/.emacs file:
;;
;;        (setq cemacs-input-lookup-file
;;              "/your-directory-for/CangJie.tit")
;;
;;    where CangJie.tit can be replaced by any other .tit file you prefer.
;;    The .tit file usually is the one you are interested in, but sometimes
;;    have trouble using it.  It doesn't have to be the current input method
;;    of cxterm.  The functions and default key bindings for input lookup are:
;;
;;        cemacs-input-lookup               : bound to Ctrl-c Ctrl-i
;;        cemacs-input-lookup-previous      : bound to Ctrl-c Ctrl-p
;;
;;    If cemacs-input-lookup-file is not defined, the above functions
;;    give you warning and do nothing.
;;
;; Q3: How do I get the keyboard layout for my input method?
;;
;;    Put the following lines in your ~/.emacs:
;;
;;    (setq cemacs-keyboard-list
;;          (list "/your-directory-for/ZOZY.tit"
;;                "/your-directory-for/CangJie.tit"))
;;
;;    where the .tit files come with standard cxterm distribution,
;;    and can be replaced by the ones you have trouble remember the
;;    keyboard location.  Now start emacs with cemacs.el, and
;;    Ctrl-C Ctrl-L" will cycle through those keyboard layouts
;;    and possibly some helper pages.
;;
;;    The number of .tit files in the list is arbitrary, you can add
;;    more by inserting .tit files to the above list command.
;;
;;    If cemacs-keyboard-list is not defined, "Ctrl-C Ctrl-L" will
;;    just cycle through the remaining enabled helper pages.
;;
;; Q4: Can I disable the annoying helper pages?
;;
;;    The default behaviour of cemacs.el turns on helper pages during startup.
;;    If you feel annoyed or immensely insulted, put the following lines
;;    in your ~/.emacs file to disable it:
;;
;;        (setq cemacs-enable-help-dummy nil)
;;        (setq cemacs-enable-help-setup nil)
;;
;;    If you have have cemacs-keyboard-list defined, the setup help page
;;    will be turned off anyway.
;;
;; Q5: Why doesn't cemacs work right for my Chinese emulator under GB code?
;;
;;    Cemacs.el has to know the encoding (GB or BIG5) for the following
;;    functions to work right:
;;
;;      cemacs-count-buffer
;;      cemacs-count-region
;;      any sentence related function
;;
;;    To tell which encoding the terminal is using, cemacs.el looks at
;;    the environmental variable CHAR_ENCODING, which is set to GB or
;;    BIG5 by cxterm automatically.  So it will work correctly for cxterm.
;;    For a Chinese emulator which doesn't define the environmental variable
;;    CHAR_ENCODING, cemacs.el assumes it is big5 encoding.
;;
;;    If you are using GB encoding, AND your Chinese terminal emulator
;;    doesn't set the variable, you have to do it manually. For bash
;;    or sh user, put the following line in your ~/.profile:
;;        export CHAR_ENCODING=GB
;;    For tcsh or csh user, put the following line in your ~/.cshrc:
;;        setenv CHAR_ENCODING GB
;;
;;
;; Q6: Why doesn't cemacs find the right Big5 character during search?
;;
;;    The default search functions for emacs are not case-sensitive,
;;    so it might find a wrong 2nd byte of a Big5 character.  This can
;;    be avoided by setting case-fold-search to nil so that the search is
;;    case sensitive.  One way to do it is:
;;
;;      ESC x set-variable RET case-fold-search RET nil RET
;;
;;    To make it back to case-insensitive, set case-fold-search to t by
;;
;;      ESC x set-variable RET case-fold-search RET t RET
;;
;;
;; Q7: Why is the screen garbled when I scroll up, or rotate the helper page?
;;
;;    This is a problem of cxterm -- you have to use matched font size for
;;    Chinese and Ascii for cxterm.  For example, taipei16 (size 16x16) 
;;    has to be used with ascii font with size 8x16.  If your ascii
;;    font height is not exactly twice its width, cxterm could have problem
;;    updating the screen.
;;
;;    Though not a good solution, an explicit redraw would clean up the screen.
;;    Recenter (bound to Ctrl-L) is one way to do it.
;;
;;
;; Q8: Why does it look messed up when a line is longer than the window width?
;;
;;    When a line is longer than the window width, emacs shows a backslash(\) 
;;    at the right edge of the the window, and the remaining on the next
;;    line.  This might split a two byte Chinese character and mess up the
;;    following line.  While it looks messed up, the content is intact.
;;    What I usually do is to use cxterm with odd number of columns, say, 
;;    run cxterm by "cxterm -geometry 81x28", and it works pretty well most
;;    of the time.
;;
;; Q9: Is there a list of functions available to users?
;;
;;    Here is a list of functions useful for users:
;;
;;    cemacs-mark-end-of-sentence          : replacement of mark-end-of-sentence
;;    cemacs-transpose-sentences           : replacement of transpose-sentence
;;    cemacs-count-region                  : count characters in a region
;;    cemacs-count-buffer                  : count characters in a buffer
;;    cemacs-auto-fill-mode                : replacement of auto-fill-mode
;;    cemacs-input-lookup                  : find the chinese input for the
;;    cemacs-input-lookup-previous         : find the chinese input for the
;;    cemacs-help-next                     : rotate to the next helper page
;;    cemacs-fill-region-as-paragraph      : replacement of
;;                                           fill-region-as-paragraph
;;    cemacs-fill-region                   : replacement of fill-region
;;
;;
;;  Here is a list of functions whose key-bindings directly substitue
;;  their ascii counterparts.  They work pretty much the same way their
;;  ascii counterparts do.
;;
;;    cemacs-fill-paragraph                : replacement of fill-paragraph
;;     (bound to ESC q)
;;     (hint: press "ESC q" and see what happens to the Chinese paragraph
;;      you are working on, or, even better, try "Ctrl-u 1 ESC q" )
;;    cemacs-forward-char                  : replacement of forward-char
;;     (bound to C-f)
;;    cemacs-backward-char                 : replacement of backward-char
;;     (bound to C-b)
;;    cemacs-delete-char                   : replacement of delete-char
;;     (bound to C-d)
;;    cemacs-backward-delete-char          : replacement of backward-delete-char
;;     (usually not bound)
;;    cemacs-backward-delete-char-untabify : replacement of 
;;     (bound to DEL)                        backward-delete-char-untabify
;; 
;;    cemacs-next-line                     : replacement of next-line
;;     (bound to C-n)
;;    cemacs-previous-line                 : replacement of previous-line 
;;     (bound to C-p)
;;    cemacs-forward-sentence              : replacement of forward-sentence
;;     (bound to ESC f)
;;    cemacs-backward-sentence             : replacement of backward-sentence
;;     (bound to ESC a)
;;    cemacs-kill-sentence                 : replacement of kill-sentence
;;     (bound to ESC k)
;;    cemacs-backward-kill-sentence        : replacement of
;;     (bound to C-x DEL)                    backward-kill-sentence
;;
;;---------- end of setup/installation -----------------------------------
;;
;;

;;; Code:

;;; -----------------
;;; Version problems
;;; -----------------
;; cemacs.el doesn't work if...
(cond
 ((= emacs-major-version 18)
  (error "Error: this version of cemacs.el does not work on emacs-18"))
 (window-system
  (error
   "Error: cemacs.el does not work on a windows system, please try option -nw"))
 ((boundp 'mule-version)
  (error "Error: this version of cemacs.el does not work with mule")))

;; Older versions of emacs-19 do not have those functions:
;;  current-left-margin
;;  current-fill-column
;;  move-to-left-margin
;;
(if (fboundp 'current-left-margin)
    (defmacro cemacs-current-left-margin () (current-left-margin))
  (defmacro cemacs-current-left-margin () left-margin))

(if (fboundp 'current-fill-column)
    (defmacro cemacs-current-fill-column () (current-fill-column))
  (defmacro cemacs-current-fill-column () fill-column))

(if (fboundp 'move-to-left-margin)
    (defmacro cemacs-move-to-left-margin () (move-to-left-margin))
  (defmacro cemacs-move-to-left-margin ()
    (beginning-of-line)
    (while (< (current-column) left-margin)
      (forward-char 1))))

;;; ------------------------
;;; 8-bit display and input
;;; ------------------------
;; Put Emacs 19 into 8-bit display
(require 'disp-table)
(standard-display-8bit 160 255)

;; 8-bit characters for input (Meta keys will be disabled)
(set-input-mode
  (car (current-input-mode)) (car (cdr (current-input-mode))) 0)

;;; -------------------------
;;; cursor movement/deleting
;;; -------------------------

(defvar cemacs-ctype-list nil
  "A list containing integer elements representing types of characters in a region.")
;;
;; Note from the author:
;
; The variable cemacs-ctype-list is the core variable for backward-delete
; algorithm. The list contains integer elements representing the character
; types of characters, in reversed order, in a region typically from 
; before the cursor to the beginning of a line.
;
; It is designed to make backward deleting and backward cursor
; movement easier, especially for big5 characters. It is re-calculated
; whenever the function cemacs-set-ctype-list-region is called.
; The generation of the list is not an efficient task, so it is meant
; to be created only when necessary.

; Each integer on the list represents a character type of one character
; (2-byte Chinese or 1-byte ascii.) For BIG5 coding, character types are
; defined as follows:

; integer character range          description
; ------- -----------------        ------------------------------------------
;     -3: \t space                 tab or space
;     -2: \n                       newline
;     -1: non-readable ascii       ascii in [0,31] or [127,159], except above
;      0: readable ascii	          ascii characters other than the above
;      1: A140-A24E     		  punctuation and other symbols
;      2: A24F-A258     		  various units
;      3: A259-A261     		  Chinese units
;      4: A262-A2AE     		  box-drawing pieces and shapes
;      5: A2AF-A2B8     		  Arabic numerals
;      6: A2B9-A2C2     		  Roman numerals
;      7: A2C3-A2CE     		  Hangzhou numerals
;      8: A2CF-A2E8     		  Latin capital letters
;      9: A2E9-A343     		  Latin small letters
;     10: A344-A35B     		  Greek capital letters
;     11: A35C-A373     		  Greek small letters
;     12: A374-A3BA     		  zhuyin symbols
;     13: A3BB-A3BF     		  zhuyin diacritics
;     14: A440-C67E     		  frequently used hanzi (5401)
;     15: C6A1-C6F7     		  hiragana
;     16: C6F8-C7B0     		  katakana
;     17: C7B1-C7E8     		  Cyrillic letters
;     18: C7E9-C7F2     		  circled numbers
;     19: C7F3-C7FC     		  parenthesized numbers
;     20: C940-F9D5     		  less frequently used hanzi (7652)
;     21: others        		  none of the above

; For GB coding, character types are defined as follows:

; integer character range          description
; ------- -----------------        ------------------------------------------
;     -3: \t space                 tab or space
;     -2: \n                       newline
;     -1: non-readable ascii       ascii in [0,31] or [127,159], other than the
;                                  above
;      0: readable ascii	   ascii characters other than the above
;      1: A1A1-A1FE                Qu 1, punctuations and other symbols
;         A3A1-A3AF                Qu 3, misc. symbols
;         A3BA-A3C0
;         A3DB-A3E0
;         A3FB-A3FE  
;      4: A9A4-A9FE                Qu 9, drawing boxes
;      2: A1A1-A9FE except above   Qu 1-9, except the above
;     14: B0A1-D7FE                Qu 15-55, first level Chinese
;     20: D8A1-F7FE                Qu 56-87, second level Chinese
;     21: others                   none of the above
;

(defun cemacs-set-ctype-list-region (&optional begin &optional end)
  "Creat the list cemacs-ctype-list of a region.
If the region is not specified, BEGIN is default to beginning of line,
and END is default to the current point.

Since the creation process is not efficient, it should be called in a
list function only when necessary. The region size is better kept
small too.

If the cursor position is on the second byte of a Chinese character before
calling this function, it will be moved to the first byte (i.e. moved
1 byte left), otherwise moved to the end of the buffer. So the side effect
of this function is to correct the cursor position in case it is stepping
on the 2nd byte."
  (or end (setq end (point)))		; default to the current point
  (or begin (setq begin (progn (beginning-of-line) (point)))) ; default to bol
  (goto-char begin)
  (setq cemacs-ctype-list nil)

  (if (not (equal (getenv "CHAR_ENCODING") "GB"))
      ;; if not GB, then it is default to BIG5 encodings
      (while (< (point) end)
	(let ((byte1 (char-after (point))))
	  (if (< byte1 160)		; ascii
	      (progn
		(forward-char 1)
		(cond
		 ((or (= byte1 ?\ ) (= byte1 ?\t))
		  (setq cemacs-ctype-list (cons '-3 cemacs-ctype-list)))
		 ((= byte1 ?\n)
		  (setq cemacs-ctype-list (cons '-2 cemacs-ctype-list)))
		 ((or (< byte1 32) (> byte1 126))
		  (setq cemacs-ctype-list (cons '-1 cemacs-ctype-list)))
		 (t
		  (setq cemacs-ctype-list (cons '0 cemacs-ctype-list)))))
	    (let* ((byte2 (char-after (1+ (point))))
		   (big5 (+ byte2 (* byte1 256)))) ; else Chinese
	      (forward-char 2)
	      (cond
	       ((and (>= big5 ?\xa440) (<= big5 ?\xc67e)); most common one first
		(setq cemacs-ctype-list (cons '14 cemacs-ctype-list)))
	       ((and (>= big5 ?\xc940) (<= big5 ?\xf9d5))
		(setq cemacs-ctype-list (cons '20 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa140) (<= big5 ?\xa24e))
		(setq cemacs-ctype-list (cons '1 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa24f) (<= big5 ?\xa258))
		(setq cemacs-ctype-list (cons '2 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa259) (<= big5 ?\xa261))
		(setq cemacs-ctype-list (cons '3 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa262) (<= big5 ?\xa2ae))
		(setq cemacs-ctype-list (cons '4 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa2af) (<= big5 ?\xa2b8))
		(setq cemacs-ctype-list (cons '5 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa2b9) (<= big5 ?\xa2c2))
		(setq cemacs-ctype-list (cons '6 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa2c3) (<= big5 ?\xa2ce))
		(setq cemacs-ctype-list (cons '7 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa2cf) (<= big5 ?\xa2e8))
		(setq cemacs-ctype-list (cons '8 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa2e9) (<= big5 ?\xa343))
		(setq cemacs-ctype-list (cons '9 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa344) (<= big5 ?\xa35b))
		(setq cemacs-ctype-list (cons '10 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa35c) (<= big5 ?\xa373))
		(setq cemacs-ctype-list (cons '11 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa374) (<= big5 ?\xa3ba))
		(setq cemacs-ctype-list (cons '12 cemacs-ctype-list)))
	       ((and (>= big5 ?\xa3bb) (<= big5 ?\xa3bf))
		(setq cemacs-ctype-list (cons '13 cemacs-ctype-list)))
	       ((and (>= big5 ?\xc6a1) (<= big5 ?\xc6f7))
		(setq cemacs-ctype-list (cons '15 cemacs-ctype-list)))
	       ((and (>= big5 ?\xc6f8) (<= big5 ?\xc7b0))
		(setq cemacs-ctype-list (cons '16 cemacs-ctype-list)))
	       ((and (>= big5 ?\xc7b1) (<= big5 ?\xc7e8))
		(setq cemacs-ctype-list (cons '17 cemacs-ctype-list)))
	       ((and (>= big5 ?\xc7e9) (<= big5 ?\xc7f2))
		(setq cemacs-ctype-list (cons '18 cemacs-ctype-list)))
	       ((and (>= big5 ?\xc7f3) (<= big5 ?\xc7fc))
		(setq cemacs-ctype-list (cons '19 cemacs-ctype-list)))
	       (t
		(setq cemacs-ctype-list (cons '21 cemacs-ctype-list))))
	      ))))
    ;; else it is GB encoding
    (while (< (point) end)
      (let ((byte1 (char-after (point))))
	(if (< byte1 160)		; ascii
	    (progn
	      (forward-char 1)
	      (cond
	       ((or (= byte1 ?\ ) (= byte1 ?\t))
		(setq cemacs-ctype-list (cons '-3 cemacs-ctype-list)))
	       ((= byte1 ?\n)
		(setq cemacs-ctype-list (cons '-2 cemacs-ctype-list)))
	       ((or (< byte1 32) (> byte1 126))
		(setq cemacs-ctype-list (cons '-1 cemacs-ctype-list)))
	       (t
		(setq cemacs-ctype-list (cons '0 cemacs-ctype-list)))))
	  (let* ((byte2 (char-after (1+ (point))))
		 (gb (+ byte2 (* byte1 256)))) ; else Chinese
	    (forward-char 2)
	    (cond
	     ((and (>= gb ?\xb0a1) (<= gb ?\xd7fe)) ; most common one first
	      (setq cemacs-ctype-list (cons '14 cemacs-ctype-list)))
	     ((and (>= gb ?\xd8a1) (<= gb ?\xf7fe))
	      (setq cemacs-ctype-list (cons '20 cemacs-ctype-list)))
	     ((or (and (>= gb ?\xa1a1) (<= gb ?\xa1fe)) ; GB code is messy in
		  (and (>= gb ?\xa3a1) (<= gb ?\xa3af)) ; punctuation codes!
		  (and (>= gb ?\xa3ba) (<= gb ?\xa3c0))
		  (and (>= gb ?\xa3db) (<= gb ?\xa3e0))
		  (and (>= gb ?\xa3fb) (<= gb ?\xa3fe)))
	      (setq cemacs-ctype-list (cons '1 cemacs-ctype-list)))
	     ((and (>= gb ?\xa9a4) (<= gb ?\xa9fe))
	      (setq cemacs-ctype-list (cons '4 cemacs-ctype-list)))
	     ((and (>= gb ?\xa1a1) (<= gb ?\xa9fe))
	      (setq cemacs-ctype-list (cons '2 cemacs-ctype-list)))
	     (t
	      (setq cemacs-ctype-list (cons '21 cemacs-ctype-list)))))))))
  (or (= (point) end) ; see if you need the 2nd byte correction
      (progn
	(setq cemacs-ctype-list (cdr cemacs-ctype-list))
	(backward-char 2)))		; move to the 1st byte
  ;;  (message "%s" cemacs-ctype-list)   ; only for debug
)

(defun cemacs-forward-char (&optional arg)
  "cemacs replacement for forward-char\n
Move point right ARG characters.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (1- arg))
    (cond
     ((eobp)
      (message "End of buffer")
      (setq arg 0))
     ((>= (char-after (point)) 160)
      (forward-char 2))
     (t (forward-char 1)))))

(defvar cemacs-update-cmds '(cemacs-backward-char
			     cemacs-backward-delete-char-untabify
			     cemacs-backward-delete-char
			     cemacs-previous-char
			     cemacs-next-char)
  "Those are commands which keep cemacs-ctype-list updated after
execution.")

(defun cemacs-backward-char (&optional arg)
  "cemacs replacement for backward-char\n
Move point left ARG characters.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (1- arg))
    (cond
      ((= (point) (point-min))		; beginning of buffer
       (message "Beginning of buffer")
       (setq arg 0))
      ((= (char-after (1- (point))) ?\n) ; newline, which is not used
       (backward-char 1)		;    by big5 encoding on the 2nd byte
       (cemacs-set-ctype-list-region))
      (t				; otherwise on this line
       (if (not (memq last-command cemacs-update-cmds))
	   (cemacs-set-ctype-list-region))
       (if (< (car cemacs-ctype-list) 1) ; ascii character
	   (progn
	     (backward-char 1)
	     (setq cemacs-ctype-list (cdr cemacs-ctype-list)))
	 (progn
	   (backward-char 2)		; Chinese character
	   (setq cemacs-ctype-list (cdr cemacs-ctype-list))))))))
	 
(defun cemacs-delete-char (&optional arg)
  "cemacs replacement for delete-char\n
Delete the following ARG characters.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (1- arg))
    (cond
      ((eobp)
       (message "End of buffer")
       (setq arg 0))
      ((>= (char-after (point)) 160)
       (delete-char 2))
      (t (delete-char 1)))
))

(defun cemacs-backward-delete-char (&optional arg)
  "cemacs replacement for backward-delete-char\n
Delete ARG characters backward,On reaching end of buffer,\n
stop and signal error.\n
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (1- arg))
      (cond
       ((bobp)
	(message "Beginning of buffer")
	(setq arg 0))
       ((= (char-after (1- (point))) ?\n) ; newline, which is not used
	(backward-delete-char 1)	;    by big5 encoding on the 2nd byte
	(cemacs-set-ctype-list-region))
       (t				; otherwise on this line
	(if (not (memq last-command cemacs-update-cmds))
	    (cemacs-set-ctype-list-region))
	(if (< (car cemacs-ctype-list) 1) ; ascii character
	    (progn
	      (backward-delete-char 1)
	      (setq cemacs-ctype-list (cdr cemacs-ctype-list)))
	  (backward-delete-char 2)	; Chinese character
	  (setq cemacs-ctype-list (cdr cemacs-ctype-list)))))))

(defun cemacs-backward-delete-char-untabify (&optional arg)
  "cemacs replacement for backward-delete-char-untabify\n
Delete ARG characters backward,On reaching end of buffer,\n
stop and signal error.\n
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (1- arg))
      (cond
       ((bobp)
	(message "Beginning of buffer")
	(setq arg 0))
       ((= (char-after (1- (point))) ?\n) ; newline, which is not used
	(backward-delete-char-untabify 1) ;    by big5 encoding on the 2nd byte
	(cemacs-set-ctype-list-region))
       (t				; otherwise on this line
	(if (not (memq last-command cemacs-update-cmds))
	    (cemacs-set-ctype-list-region))
	(if (< (car cemacs-ctype-list) 1) ; ascii character
	    (progn
	      (backward-delete-char-untabify 1)
	      (setq cemacs-ctype-list (cdr cemacs-ctype-list)))
	  (backward-delete-char-untabify 2)	; Chinese character
	  (setq cemacs-ctype-list (cdr cemacs-ctype-list)))))))

(defvar cemacs-goal-column 0
  "Cursor position used by cemacs-next-line and cemacs-previous-line")

(defun cemacs-next-line(&optional arg)
  "cemacs replacement for next-line\n
Move cursor vertically down ARG lines.
ARG defaults to 1.\n
argument: (&optional n)\n"
  (interactive "p")
  (or (or (eq last-command 'cemacs-next-line)
	  (eq last-command 'cemacs-previous-line))
      (setq cemacs-goal-column (current-column)))
  (forward-line arg)
  (move-to-column cemacs-goal-column)
  (cemacs-set-ctype-list-region)	; correct cursor location if it's wrong
)

(defun cemacs-previous-line(&optional arg)
  "cemacs replacement for previous-line\n
Move cursor vertically up ARG lines.
ARG defaults to 2.\n
argument: (&optional n)\n"
  (interactive "p")
  (or (or (eq last-command 'cemacs-next-line)
	  (eq last-command 'cemacs-previous-line))
      (setq cemacs-goal-column (current-column)))
  (forward-line (* arg -1))
  (move-to-column cemacs-goal-column)
  (cemacs-set-ctype-list-region)	; correct cursor location if it's wrong
)

;;; ----------------------------
;;; sentence related functions 
;;; ----------------------------
;; Redefine the sentence end. Make the original recognied BIG5 or GB
;; end of sentence, and make the search case sensitive for BIG5.
(if (not (equal (getenv "CHAR_ENCODING") "GB"))	; default is big5
    (progn
      (defconst cemacs-closures
	(list "¡^" "¡b" "¡f" "¡j" "¡n"
	      "¡r" "¡v" "¡z" "¡~" "¡¢"
	      "¡¤" "¡¦" "¡¨" "¡ª" "¡¬"))
      (defvar cemacs-chinese-sentence-end ; must include the above closures
	(purecopy "\\(¡C\\|¡H\\|¡I\\)\\(¡\\^\\|¡b\\|¡f\\|¡j\\|¡n\\|¡r\\|¡v\\|¡z\\|¡~\\|¡¢\\|¡¤\\|¡¦\\|¡¨\\|¡ª\\|¡¬\\)*[]\"')} \t\n]*")))
  (defconst cemacs-closures
    (list "¡¯" "¡±" "¡³" "¡µ" "¡·"
	  "¡¹" "¡»" "¡½" "¡¿" "£¢"
	  "£§" "£©" "£Ý" "£ý"))
  (defvar cemacs-chinese-sentence-end	; must include the above closures
    (purecopy "\\(¡£\\|£¿\\|£¡\\)\\(¡¯\\|¡±\\|¡³\\|¡µ\\|¡·\\|¡¹\\|¡»\\|¡½\\|¡¿\\|£¢\\|£§\\|£©\\|£Ý£ý\\)*[]\"')} \t\n]*")))

(defvar cemacs-sentence-end
  (concat cemacs-chinese-sentence-end "\\|"  sentence-end)
  "cemacs replacement of sentence-end")

(defun cemacs-forward-sentence (&optional arg)
  "cemacs replacement of forward-sentence\n
Move forward to next `cemacs-sentence-end'.  With argument, repeat.
With negative argument, move backward repeatedly to the sentence beginning.

The variable `cemacs-sentence-end' is a regular expression that matches ends of
sentences.  Also, every paragraph boundary terminates sentences as well."
  (interactive "p")
  (or arg (setq arg 1))
  (let ((case-fold-search nil))		; you need case-sensitivity for big5
    (while (< arg 0)			; find the last sentence start
      (let ((par-beg (save-excursion (start-of-paragraph-text) (point))))
	(if (re-search-backward (concat cemacs-chinese-sentence-end
					"[^ \t\n]\\|"
					sentence-end
					"[^ \t\n]") par-beg t)
	    (goto-char (1- (match-end 0)))
	  (goto-char par-beg)))
      (if (not (member (buffer-substring (point) (+ (point) 2))
		       cemacs-closures)) ; no stop at closure
	  (setq arg (1+ arg))))
    (while (> arg 0)			; find the next sentence end
      (let ((par-end (save-excursion (end-of-paragraph-text) (point))))
	(if (re-search-forward cemacs-sentence-end par-end t)
	    (skip-chars-backward " \t\n")
	  (goto-char par-end)))
      (setq arg (1- arg)))))

(defun cemacs-backward-sentence (&optional arg)
  "cemacs replacement of backward-sentence\n
Move backward to start of sentence.  With arg, do it arg times.
See `cemacs-forward-sentence' for more information."
  (interactive "p")
  (or arg (setq arg 1))
  (cemacs-forward-sentence (- arg)))

(defun cemacs-kill-sentence (&optional arg)
  "cemacs replacement of cemacs-kill-sentence\n
Kill from point to end of sentence.
With arg, repeat; negative arg -N means kill back to Nth start of sentence."
  (interactive "p")
  (kill-region (point) (progn (cemacs-forward-sentence arg) (point))))

(defun cemacs-backward-kill-sentence (&optional arg)
  "cemacs replacement of backward-kill-sentence\n
Kill back from point to start of sentence.
With arg, repeat, or kill forward to Nth end of sentence if negative arg -N."
  (interactive "p")
  (kill-region (point) (progn (cemacs-backward-sentence arg) (point))))

(defun cemacs-mark-end-of-sentence (arg)
  "cemacs replacement of mark-end-of-sentence\n
Put mark at end of sentence.  Arg works as in `cemacs-forward-sentence'."
  (interactive "p")
  (push-mark
    (save-excursion
      (cemacs-forward-sentence arg)
      (point))
    nil t))

(defun cemacs-transpose-sentences (arg)
  "cemacs replacement of transpost-sentence\n
Interchange this (next) and previous sentence."
  (interactive "*p")
  (transpose-subr 'cemacs-forward-sentence arg))

;;; --------------
;;; word counting
;;; --------------
(defun cemacs-count-region (begin end)
  "Print number of Chinese and ascii characters in the region.
The results stats the following four catagories:
    1. Chinese characters (including 2-byte, or full-size space) except
       the 2nd catagories
    2. full-size punctuation, symbols and boxes, which your publisher
       probably wouldn't count as characters
    3. readable asciis, which do not include space, tabs, or newlines
    4. number of lines."
  (interactive "r")
  (let* ((chunk-size 8192)	; let's do a chunk of characters at a time
	 (chunk-start begin)	;   so that cemacs-ctype-list won't use
	 (chunk-end (+ begin chunk-size)) ; too much memory
	 (line-count 0)
	 (asc-count 0)
	 (chi-count 0)
	 (chi1-count 0)
	 (chi4-count 0))
    (save-excursion
      (while (< chunk-start end)
	(or (<= chunk-end end) (setq chunk-end end))
	(cemacs-set-ctype-list-region chunk-start chunk-end)
	(let ((ctype-ptr cemacs-ctype-list))
	  (while ctype-ptr
	    (let ((ctype (car ctype-ptr)))
	      (cond
	       ((= -3 ctype))		; tab & space, do nothing
	       ((= -2 ctype)		; newline
		(setq line-count (1+ line-count)))
	       ((= -1 ctype))		; non-readable ascii, do nothing
	       ((= 0 ctype)		; readable ascii
		(setq asc-count (1+ asc-count)))
	       ((= 1 ctype)
		(setq chi1-count (1+ chi1-count)))
	       ((= 4 ctype)
		(setq chi4-count (1+ chi4-count)))
	       (t
		(setq chi-count (1+ chi-count)))))
	    (setq ctype-ptr (cdr ctype-ptr))))
	(message "counting: %d%%"
		 (/ (* 100 (- chunk-end begin)) (- end begin)))
	(setq chunk-start (point)
	      chunk-end (+ chunk-end chunk-size))))
    (message
     "%d Chinese, %d punctuation/symbols/boxes, %d readable ascii, %d lines"
     chi-count (+ chi1-count chi4-count) asc-count (1+ line-count))))

(defun cemacs-count-buffer ()
  "Print number of Chinese and ascii characters in the current buffer.
The results stats the following four catagories:
   1. Chinese characters (including 2-byte, or full-size space) except
      the 2nd catagories
   2. full-size punctuation, symbols and boxes, which your publisher
      probably wouldn't count as characters
   3. readable asciis, which do not include space, tabs, or newlines
   4. number of lines."
  (interactive)
  (cemacs-count-region (point-min) (point-max)))

;; -----------------------
;; fill related functions
;; -----------------------

(defun non-multiple (a b)
  "Test to see if an integer A is not a multiple of B"
  (/= (% a b) 0))

;; cemacs-fill-region-as-paragraph
;; This uses a much simplier algorithm, which align and justify Chinese
;; characters, but ascii words are treated as non-separable.
(defun cemacs-fill-region-as-paragraph (from to &optional spacing)
  "Fill the region as one Chinese paragraph.
It removes any paragraph breaks, extra whitespace in the region,
and extra newlines at the end, indents and fills lines between 
the margins given by the `current-left-margin' and `current-fill-column'
functions. It leaves point at the beginning of the line following the
paragraph.

If the optional prefix argument SPACING is given, there will be
SPACING spaces between adjacent Chinese characters."
  (interactive "r\nP")
  (or spacing (setq spacing 0))
  (or (< from to)
      (let ((temp to))
        (setq to from from temp)))
  (let ((whitespace '(?\n ?\ ?\t))
	(from-plus-indent nil)
	(end nil)
	(begin nil)
	(spacing-plus-2 (+ 2 spacing)))
    (goto-char from)
    (skip-chars-forward " \t\n")
    (while (non-multiple (- (current-column) (cemacs-current-left-margin))
			 spacing-plus-2) ; make sure the first readable
      (progn				; char starts at the right edge
	(insert ?\ )
	(setq to (1+ to))))
    (setq from-plus-indent (point))
    (setq end to)
    (beginning-of-line)
    (setq begin (point))

    ;; 1st run through the buffer to do some household cleanup
    ;;   1. add space after Chinese characters
    ;;   2. shrink consecutive whitespaces in ascii chunk to one space
    ;;   3. align ascii with Chinese by erasing isolated space character, or
    ;;      adding spaces to the end of a chunk of ascii
    (save-restriction
      (narrow-to-region begin end)
      (goto-char from-plus-indent)
      (while (not (eobp))
	(let ((byte1 (char-after (point))))
	  (if (< byte1 160)		; ascii chunk must be aligned ----
	      (let ((asc-count 0))
		(while (and (not (eobp)) ; chunk of ascii
			    (< (setq byte1 (char-after (point))) 160))
		  (cond
		   ((memq byte1 whitespace) ; shrink consecutive whitespaces
		    (while (and (not (eobp))
				(memq (char-after (point)) whitespace))
		      (delete-char 1))
		    (insert ?\ )		; replace by a single space
		    (setq asc-count (1+ asc-count))) ; end of whitespaces chunk
		   (t			; other ascii, just pass over
		    (forward-char 1)
		    (setq asc-count (1+ asc-count)))))
		(if (= (char-after (1- (point))) ?\ ) ; erasing single space
		    (progn 
		      (delete-backward-char 1)
		      (setq asc-count (1- asc-count))))
		(while (non-multiple asc-count spacing-plus-2)
					; aliagn ascii with Chinese by
		  (insert ?\ )
		  (setq asc-count (1+ asc-count))))
					; end of ascii chunk ----------
	    (forward-char 2)		; else it's a Chinese character
	    (let ((i 0))		; insert spaces between Chinese char.
	      (while (< i spacing)
		(insert ?\ )
		(setq i (1+ i)))))))

      ; 2nd run through the buffer: now the region becomes a long string
      ; with single space as the only possible whitesapce, filling is easy.
      ; The following characters are breakable:
      ;     ** the right side of a Chinese character
      ;     ** the left side of a space (don't use the right side!)
      ;     ** the right side of any readable ascii immediately followed
      ;        by a space or a Chinese character
      (beginning-of-buffer)
      (goto-char from-plus-indent)
      (while (not (eobp))
	(let ((last-breakable (point)))
	  ;; let's do a line at a time
	  (while (and (not (eobp))
		      (< (current-column) (cemacs-current-fill-column)))
	    ;; find the breakable point
	    (let ((byte1 (char-after (point))))
	      (cond
	       ((= byte1 ?\ )		; space is breakable on the left
		(setq last-breakable (point))
		(forward-char 1))
	       ((< byte1 160)	; other asciis breakable only when followed by
		(forward-char 1)
		(if (or (eobp)
			(>= (char-after (point)) 160) ; a Chinese character, 
			(memq (char-after (point)) whitespace)) ;  or a whitespace
		    (setq last-breakable (point))))
	       (t			; Chinese, all breakable on the right
		(forward-char spacing-plus-2)
		(setq last-breakable (point))))))

	  ;; insert the newline
	  (goto-char last-breakable)
	  (insert ?\n)
	
	  ;; now at the beginning of next line, we have to
          ;; indent, erase space, and check alignment for 1st ascii chunk
	  (or (eobp)
	      (progn
		(or (zerop (cemacs-current-left-margin)) ; indent
		    (indent-to-left-margin))
		(while (and (not (eobp)) ; erase 1st few spaces
			    (= (char-after (point)) ?\ ))
		  (delete-char 1))
		; check 1st ascii chunk aliagnment
 		(if (and (not (eobp))
			 (< (char-after (point)) 160))
		    (if (catch 'alignment-required
			  (while (and (not (eobp)) ; while always returns nil
				      (< (current-column)
					 (cemacs-current-fill-column)))
			    (forward-char 1)
			    (if (and (not (eobp))
				     (>= (char-after (point)) 160))
				(throw 'alignment-required 
				       (non-multiple (- (current-column)
							(cemacs-current-left-margin))
						     spacing-plus-2)))))
			(progn		; alignment here
			  (while (= (char-after (1- (point))) ?\ )
			    (backward-delete-char 1))
			  (while (non-multiple (- (current-column)
						  (cemacs-current-left-margin))
					       spacing-plus-2)
			    (insert ?\ )))))
		(cemacs-move-to-left-margin))))))))

(defun cemacs-fill-region (from to &optional spacing)
  "Fill each Chinese paragraph in a region.
If the optional prefix argument SPACING is given, there will be
SPACING spaces between adjacent Chinese characters."
  (interactive "r\nP")
  (or spacing (setq spacing 0))
  (let (beg end)
    (save-restriction
      (goto-char (max from to))
      (setq end (point))
      (goto-char (setq beg (min from to)))
      (beginning-of-line)
      (narrow-to-region (point) end)
      (while (not (eobp))
	(let ((initial (point))
	      end)
	  (forward-paragraph 1)
	  (setq end (point))
	  (forward-paragraph -1)
	  (if (< (point) beg)
	      (goto-char beg))
	  (if (>= (point) initial)
	      (cemacs-fill-region-as-paragraph (point) end spacing)
	    (goto-char end)))))))

(defun cemacs-fill-paragraph (&optional spacing)
  "Fill Chinese paragraph at or after point.
If the optional prefix argument SPACING is given, there will be
SPACING spaces between adjacent Chinese characters."
  (interactive "P")
  (or spacing (setq spacing 0))
  (let ((before (point)))
    (save-excursion
      (forward-paragraph)
      (let ((end (point))
	    (beg (progn (backward-paragraph) (point))))
	(goto-char before)
	(cemacs-fill-region-as-paragraph beg end spacing)
))))

;; you have to make a one-line patch to emacs source code for the 
;; following function (cemacs-auto-fill-mode) to work right. It
;; doesn't affect other functions if you do not have the patch.
(defun cemacs-auto-fill-mode (arg)
  "Toggle cemacs-auto-fill mode.
With ARG, turn `cemacs-auto-fill' mode on iff ARG is positive.
In `cemacs-auto-fill' mode, inserting anything at a column beyond
`fill-column' automatically breaks the line at a proper location."
  (interactive "P")
  (prog1 (setq auto-fill-function
	       (if (if (null arg)
		       (not auto-fill-function)
		     (> (prefix-numeric-value arg) 0))
		   'cemacs-do-auto-fill))
    (force-mode-line-update)))

(defun cemacs-do-auto-fill ()
  "Break line if non-white characters beyond fill-column."
  (if (> (current-column) fill-column)
      (progn
	(save-excursion
	 ;; move to the last breakable point before fill-column
	 (move-to-column fill-column)
	 (cemacs-set-ctype-list-region)
	 (while (zerop (car cemacs-ctype-list))
	   (backward-char 1)
	   (setq cemacs-ctype-list (cdr cemacs-ctype-list)))
	 (insert ?\n)
	 (or (/= (char-after (point)) ?\ )
	     (delete-char 1))
;; refresh the display seems helpful sometimes
;;	 (redraw-frame (selected-frame)) 
	 ))))

;; -------------------------------
;; input lookup related functions
;; -------------------------------
(defvar cemacs-keyprompt-alist nil
  "association list of key and corresponding prompt")

(defun cemacs-build-keyprompt-alist (buf)
  "builds the chinese key-prompt association list from buffer BUF.
Returns nil if there is no keyword KEYPROMPT in the file
cemacs-input-lookup-file, otherwise returns the keyprompt list."
  (let (alist)
    (save-excursion
      (set-buffer buf)
      (beginning-of-buffer)
      (while (search-forward "KEYPROMPT(" nil t 1)
	(setq alist
	      (cons (list
		     (buffer-substring	; the key
		      (point)
		      (1- (search-forward ")" nil t 1)))
		     (buffer-substring	; the prompt
		      (1- (re-search-forward "[^: \t]" nil t 1))
		      (1- (re-search-forward "[ \t\n]" nil t 1))))
		    alist))))
      alist))

(defun cemacs-input-lookup ()
  "displays the chinese input sequence for the character under cursor\n
The input method for lookup is a string specifying a .tit file, selected
by the variable cemasc-input-lookup-file, typically defined in the file
'~/.emacs'."
  (interactive)
  (cond
   ((not (boundp 'cemacs-input-lookup-file))
    (error "Error: cemacs-input-lookup-file is not defined, \
see cemacs.el for details."))
   ((eobp)
    (error "End of buffer"))
   ((< (char-after (point)) 160)
    (message "%c" (char-after (point))))
   (t
    (save-excursion
      (let ((chinese (buffer-substring (point) (+ (point) 2)))
	    (msg nil))
	(set-buffer cemacs-tit-buffer)
	(beginning-of-buffer)
	(search-forward "BEGINDICTIONARY" nil nil 1)
	(setq msg (concat chinese ":  "))
	(catch 'no-more-match
	  (while 1
	    ; find the correct character
	    (let ((on-2nd-byte t))
	      (while on-2nd-byte
		(let ((case-fold-search nil)) ; must be case-sensitive for big5
		  (or (search-forward chinese nil t 1)
		      (throw 'no-more-match nil)))
		(save-excursion		; make sure the boundary is right
		  (let ((column (current-column)))
		    (beginning-of-line)
		    (while (< (char-after (point)) 160)
		      (forward-char 1))
		    (if (= 0 (% (- column (current-column)) 2))
			(setq on-2nd-byte nil))))))
	    ; convert the sequence to prompt
	    (beginning-of-line)
	    (while (not (memq (char-after (point)) '(?\t ?\ )))
	      (let ((nchars 1))
		(or (/= ?\\ (char-after (point)))
		    (setq nchars 4))
		(setq msg
		      (concat msg
			      (let ((prompt (assoc (buffer-substring
						    (point)
						    (+ (point) nchars))
						   cemacs-keyprompt-alist)))
				(if prompt
				    (nth 1 prompt)
				  (char-to-string (char-after (point)))))))
		(forward-char nchars)))
	    (setq msg (concat msg "  "))
	    (next-line 1)))		; end of while and no more match
	; no more match now
	(message "%s" msg))))))

(defun cemacs-input-lookup-previous ()
  "displayss the chinese input sequence for the character before cursor
The input method for lookup is selected by the variable
cemasc-input-lookup-file, typically defined in the file '~/.emacs'."
  (interactive)
  (save-excursion
    (if (bobp)
	(message "beginning of buffer")
      (cemacs-backward-char 1)
      (cemacs-input-lookup)
      (cemacs-forward-char 1))))


;;-----------------
;; helper pages
;;-----------------
(defvar cemacs-helper nil
  "a list of strings containing help and keyboard layouts as its elements.
It is generated when emacs starts. Each element is a string
to be shown on the screen when cemacs-help-next is executed.")

(defvar cemacs-helper-pointer nil
  "pointer to the cemacs-helper element to be displayed")

(defun cemacs-help-next ()
  "displays the element pointed to by cemacs-helper-pointer,
and rotates to make cemacs-helper-pointer pointing to the next element
of cemacs-helper"
  (interactive)
  (let ((help-string (car cemacs-helper-pointer))
	(help-buf-name " '^' means Ctrl key, use ^c^l to turn on/off help"))
    (if help-string			; if you have something there
	(progn
	  (with-output-to-temp-buffer help-buf-name
	    (princ help-string))
	  (save-selected-window 
	    (select-window (get-buffer-window help-buf-name))
	    (shrink-window-if-larger-than-buffer)
	    (toggle-read-only)))
      ; else its the end of layout list, which is nil
      (delete-windows-on (get-buffer help-buf-name))
      (if (get-buffer help-buf-name)
	  (kill-buffer help-buf-name))))
  ;;  (redraw-display)			; if cxterm screws up display...
  ; pointing to the next element of cemacs-helper
  (let ((length (length cemacs-helper)))
    (setq cemacs-helper-pointer
	  (nthcdr (% (1+ (- length
			    (length cemacs-helper-pointer)))
		     (1+ length))
		  cemacs-helper))))

(defun cemacs-set-keyboard-layout (alist)
  "sets the keyboard layout from the keyprompt association list ALIST
It adds one more layout string to cemacs-helper. The keyprompt
association list is generated from cemacs-build-keyprompt-alist."
  (let ((keylist (list "1" "2" "3" "4" "5" "6" "7" "8" "9" "0" "-" "=" "\n "
		       "q" "w" "e" "r" "t" "y" "u" "i" "o" "p" "[" "]" "\n  "
		       "a" "s" "d" "f" "g" "h" "j" "k" "l" ";" "'" "\n   "
		       "z" "x" "c" "v" "b" "n" "m" "," "." "/"))
	key prompt help-string)
    (while (setq key (car keylist))
      (if (= (string-to-char key) ?\n)
	  (setq help-string (concat help-string key))
	(setq prompt (nth 1 (assoc key alist)))
	(setq help-string
	      (concat help-string
		      key (format "%-5s" (if prompt prompt " ")))))
      (setq keylist (cdr keylist)))
    (setq cemacs-helper
	  (cons help-string
		cemacs-helper))))

;;----------------------------------------------
;; put input lookup and keyboard helper to work
;;----------------------------------------------
;; determine the keyboard layout if cemacs-keyboard-list is defined
(defvar cemacs-enable-help-setup t
  "a switch to enable/disable the setup help information
If nil, no set help shows up at startup. Default is to enable.")

(if (boundp 'cemacs-keyboard-list)
    (let (tit)
      (while (setq tit (car cemacs-keyboard-list))
	(cemacs-set-keyboard-layout
	 (cemacs-build-keyprompt-alist
	  (find-file-read-only tit)))
	(kill-buffer (get-buffer tit))
	(setq cemacs-keyboard-list (cdr cemacs-keyboard-list))))
  ;; else add instruction for keyboard-list
  (if cemacs-enable-help-setup
      (setq cemacs-helper  (cons "\
For input lookup and keyboard helper, put lines like below in your ~/.emacs:
   (setq cemacs-input-lookup-file \"/your-directory-for/CangJie.tit\")
   (setq cemacs-keyboard-list (list \"/your-directory-for/ZOZY.tit\"
                                    \"/your-directory-for/CangJie.tit\"))"
				      cemacs-helper))))

; add the first page of help for dummy
(defvar cemacs-enable-help-dummy t
  "a switch to enable/disable the help information for dummy
If nil, no dummy help shows up at startup. Default is to enable.")

(if cemacs-enable-help-dummy
    (setq cemacs-helper  (cons "\
^f    forward    ^n next line     ^x^f open    ^k kill   ^_ undo    ^h    help
^b    backward   ^p prev line     ^x^s save    ^y yank   ^l redraw  ESC q fill
^v    PageDn     ^a beg of line   ^x^w write   ^c^i input lookup at cursor
ESC v PageUp     ^e end of line   ^x^c quit    ^c^p input lookup before cursor"
				    cemacs-helper)))

(setq cemacs-helper-pointer cemacs-helper) ; pointer to the first helper page

(cemacs-help-next)			; show the first page when startup


;; build the lookup table:
;;
;; the following part has to go after the previous keyboard helper portion,
;; otherwise the .tit buffer could get killed if the lookup file
;; is one the keyboard layout ---- cemacs-tit-buffer
;; should be kept for input lookup.
;;
;; build the input lookup table, unless
;;     1.  cemacs-input-lookup-file is not defined (in .emacs)
;;     2.  the file cemacs-input-lookup-file does not contain the string
;;         "ENCODE:" string, most likely it is not a .tit file
(if (boundp 'cemacs-input-lookup-file)
    (let ((old-buffer (current-buffer)))
      (save-excursion
	(defvar cemacs-tit-buffer
	  (find-file-read-only cemacs-input-lookup-file))
	(beginning-of-buffer)
	(if (search-forward "ENCODE:" nil t 1)
	    (progn
	     (rename-buffer " *cemacs-input-lookup-buffer*")
	     ; a space leading the buffer name make it less visible to users
	     (setq cemacs-keyprompt-alist
		   (cemacs-build-keyprompt-alist cemacs-tit-buffer)))
	  (message "wrong or non-existing tit file: %s"
		   cemacs-input-lookup-file)
	  (kill-buffer cemacs-tit-buffer)))
      ; switch back to prevent exposing the tit file to the user
      (switch-to-buffer old-buffer)))

;;; -------------
;;; Key bindings
;;; -------------
(global-set-key "\C-c\C-i" 'cemacs-input-lookup)
(global-set-key "\C-c\C-p" 'cemacs-input-lookup-previous)
(global-set-key "\C-c\C-l" 'cemacs-help-next)

(substitute-key-definition 'forward-char
			   'cemacs-forward-char global-map)
(substitute-key-definition 'backward-char
			   'cemacs-backward-char global-map)
(substitute-key-definition 'delete-char
			   'cemacs-delete-char global-map)
(substitute-key-definition 'delete-backward-char
			   'cemacs-backward-delete-char global-map)
(substitute-key-definition 'backward-delete-char
			   'cemacs-backward-delete-char global-map)
(substitute-key-definition 'backward-delete-char-untabify
			   'cemacs-backward-delete-char-untabify global-map)
(substitute-key-definition 'next-line
			   'cemacs-next-line global-map)
(substitute-key-definition 'previous-line
			   'cemacs-previous-line global-map)
(substitute-key-definition 'fill-paragraph
			   'cemacs-fill-paragraph global-map)
(substitute-key-definition 'forward-sentence
			   'cemacs-forward-sentence global-map)
(substitute-key-definition 'backward-sentence
			   'cemacs-backward-sentence global-map)
(substitute-key-definition 'kill-sentence
			   'cemacs-kill-sentence global-map)
(substitute-key-definition 'backward-kill-sentence
			   'cemacs-backward-kill-sentence global-map)

;; Because local keymaps override the global keymap,
;; it's sometimes hard to keep DEL bound to the command we want.
;; But maybe the following will help.
(add-hook 'text-mode-hook
	  (function (lambda ()
		      (define-key text-mode-map "\177"
			'cemacs-backward-delete-char-untabify)
		      t)))
(add-hook 'lisp-mode-hook
	  (function (lambda ()
		      (define-key lisp-mode-map "\177"
			'cemacs-backward-delete-char-untabify)
		      t)))
(add-hook 'c-mode-hook
	  (function (lambda ()
		      (define-key c-mode-map "\177"
			'cemacs-backward-delete-char-untabify)
		      t)))

(defun cemacs-rebind-del-key ()
  "rebind DEL to its appropriate cemacs command, if necessary.\n"
  (interactive)
  (if (not (current-local-map))		; if it's nil
      (local-set-key "\177" 'cemacs-backward-delete-char-untabify))
  (let ((cemacs-old-del (lookup-key (current-local-map) "\177")))
    (cond
     ((eq cemacs-old-del 'backward-delete-char-untabify)
      (local-set-key "\177" 'cemacs-backward-delete-char-untabify))
     ((or (eq cemacs-old-del 'backward-delete-char)
	  (eq cemacs-old-del 'delete-backward-char))
      (local-set-key "\177" 'cemacs-backward-delete-char)))))
(cemacs-rebind-del-key)

;; make C-l not only recenter the screen but also rebind DEL if necessary.
(defun cemacs-recenter (&optional arg)
  "recenter, and rebind DEL if necessary.\n"
  (interactive "P")
  (recenter arg)
  (cemacs-rebind-del-key))
(substitute-key-definition 'recenter
			   'cemacs-recenter global-map)

;;--------------------------------
;; Finally, provide this package.
(provide 'cemacs)

;;; cemacs.el ends here

;; Autoload and customize CCHELP.
;; note from the maintainer:
;;      cchelp helps you to find the PinYin pronounciation and English
;;      meaning of a Chinese character.  It's not useful for a native
;;      Chinese speaking user, but might help if you feel like a stranger
;;      to those Chinese characters.
;;
;;      If you don't have CCHELP, just omit this stuff.
;;
;(autoload 'cchelp "cchelp"
;  "Give helpful information on the Chinese character at point.\n" t)
;(define-key help-map "o" 'cchelp)
;(setq cchelp-pop-buffer t)

