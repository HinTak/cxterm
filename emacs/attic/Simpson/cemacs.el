;;;; cemacs.el
;;;; by Stephen G. Simpson <simpson@math.psu.edu>
;;;; original version created February 1991
;;;; last revised 22 June 1994

;;
;; DESCRIPTION
;; This code enables Emacs to edit Chinese files within cxterm
;; or other Chinese terminal emulators.
;;
;; SOFTWARE REQUIREMENTS
;; It is assumed that you are running GNU Emacs Release 19.
;;
;; BUGS
;; This code is very primitive.  If you make any improvements, please
;; send them to Steve Simpson <simpson@math.psu.edu>.
;;
;; AVAILABILITY
;; I will try to keep the latest version of this file available for
;; anonymous FTP in math.psu.edu:/pub/simpson/chinese/cemacs/emacs-19.
;;

;;
;; HOW TO USE THIS FILE
;; 
;; Go into your cxterm or other Chinese terminal emulator, start
;; Emacs, and load this file by typing, for instance,
;; 
;;   ESC x load-file RET cemacs.el RET
;; 
;; Your Emacs will now be able to display and edit Chinese files.
;;
;; If you want to automate this procedure, you should place cemacs.el
;; in your site-lisp directory with other Emacs Lisp code, so that
;; your Emacs will be able to load it.  (If you wish, you may
;; byte-compile cemacs.el.)  You may then put the following line into
;; your ~/.cshrc
;; 
;;   alias cemacs 'emacs -nw -l cemacs'
;; 
;; You will then be able to bring up Emacs in Chinese mode by simply
;; typing "cemacs" (inside cxterm, of course).
;;

;; Put Emacs 19 into 8-bit mode.

(require 'disp-table)
(standard-display-8bit 160 255)
(set-input-mode
  (car (current-input-mode)) (car (cdr (current-input-mode))) 0)

;; Redefine some keys.

(substitute-key-definition 'forward-char 'cemacs-forward-char global-map)
(substitute-key-definition 'backward-char 'cemacs-backward-char global-map)
(substitute-key-definition 'delete-char 'cemacs-delete-char global-map)
(substitute-key-definition 'delete-backward-char
  'cemacs-backward-delete-char global-map)
(substitute-key-definition 'backward-delete-char
  'cemacs-backward-delete-char global-map)
(substitute-key-definition 'backward-delete-char-untabify
  'cemacs-backward-delete-char-untabify global-map)

;; Because local keymaps override the global keymap,
;; it's sometimes hard to keep DEL bound to the command we want.
;; But maybe the following will help.
(add-hook 'text-mode-hook (function (lambda ()	  
  (define-key text-mode-map "\177" 'cemacs-backward-delete-char-untabify)
  t)))
(add-hook 'lisp-mode-hook (function (lambda ()
  (define-key lisp-mode-map "\177" 'cemacs-backward-delete-char-untabify)
  t)))
; (add-hook 'c-mode-hook (function (lambda ()
;   (define-key c-mode-map "\177" 'cemacs-backward-delete-char-untabify)
;   t)))
;; Commented out, because something about c-mode-map doesn't work
;; in Emacs 19.25.
(defun cemacs-rebind-del-key ()
  "rebind DEL to its appropriate cemacs command, if necessary.\n"
  (interactive)
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
  (interactive "P") (recenter arg) (cemacs-rebind-del-key))
(substitute-key-definition 'recenter 'cemacs-recenter global-map)

;; Define our Cemacs commands.

;; NOTE: The code below does not work perfectly with Big5 in mixed
;; Chinese/ASCII files.  The problem is that, in Big5, the second byte
;; of a Chinese character can be an ASCII byte, so it is harder to
;; distinguish Chinese from ASCII.  Can someone suggest a way to get
;; around this problem?

;; replacement for forward-char
(defun cemacs-forward-char (&optional arg)
  "cemacs replacement for forward-char\n
Move point right ARG characters.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (- arg 1))
    (cond
      ((= (point) (point-max))
       (message "End of buffer")
       (setq arg 0))
      ((>= (char-after (point)) 160)
       (forward-char 1)
       (forward-char 1))
      (t (forward-char 1)))
))

;; replacement for backward-char
(defun cemacs-backward-char (&optional arg)
  "cemacs replacement for backward-char\n
Move point left ARG characters.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (- arg 1))
    (cond
      ((= (point) (point-min))
       (message "Beginning of buffer")
       (setq arg 0))
      ((>= (char-after (- (point) 1)) 160)
       (backward-char 1)
       (backward-char 1))
      (t (backward-char 1)))
))

;; replacement for delete-char
(defun cemacs-delete-char (&optional arg)
  "cemacs replacement for delete-char\n
Delete the following ARG characters.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (- arg 1))
    (cond
      ((= (point) (point-max))
       (message "End of buffer")
       (setq arg 0))
      ((>= (char-after (point)) 160)
       (delete-char 1)
       (delete-char 1))
      (t (delete-char 1)))
))

;; replacement for backward-delete-char and delete-backward-char
(defun cemacs-backward-delete-char (&optional arg)
  "cemacs replacement for backward-delete-char and delete-backward-char\n
Delete ARG characters backward, changing tabs into spaces.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (- arg 1))
    (cond
      ((= (point) (point-min))
       (message "Beginning of buffer")
       (setq arg 0))
      ((>= (char-after (- (point) 1)) 160)
       (backward-delete-char 1)
       (backward-delete-char 1))
      (t (backward-delete-char 1)))
))

;; replacement for backward-delete-char-untabify
(defun cemacs-backward-delete-char-untabify (&optional arg)
  "cemacs replacement for backward-delete-char-untabify\n
Delete ARG characters backward, changing tabs into spaces.
On reaching end of buffer, stop and signal error.
ARG defaults to 1.\n
arguments: (&optional n)\n"
  (interactive "p")
  (while (> arg 0)
    (setq arg (- arg 1))
    (cond
      ((= (point) (point-min))
       (message "Beginning of buffer")
       (setq arg 0))
      ((>= (char-after (- (point) 1)) 160)
       (backward-delete-char-untabify 1)
       (backward-delete-char-untabify 1))
      (t (backward-delete-char-untabify 1)))
))

;; Finally, provide this package.
(provide 'cemacs)

;;
;; end of cemacs.el
;;

;; Autoload and customize CCHELP.
;; If you don't have CCHELP, omit this stuff.
; (autoload 'cchelp "cchelp"
;  "Give helpful information on the Chinese character at point.\n" t)
; (define-key help-map "o" 'cchelp)
; (setq cchelp-pop-buffer t)
