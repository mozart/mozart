;; Major mode for editing Oz, and for running Oz under Emacs
;; Copyright (C) 1993 DFKI GmbH
;; Author: Ralf Scheidhauer and Michael Mehl ([scheidhr|mehl]@dfki.uni-sb.de)
;; $Id$

;; TODO
;; - state message: Should we use the mode-line ???

(require 'comint)
(require 'compile)

;; ---------------------------------------------------------------------
;; global effects
;; ---------------------------------------------------------------------

(or (member ".ozo" completion-ignored-extensions)
    (setq completion-ignored-extensions
	  (append '(".ozo" ".ozs")
		  completion-ignored-extensions)))

;; automatically switch into Oz-Mode when loading
;; files ending in ".oz"
(or (assoc "\\.oz$" auto-mode-alist)
    (setq auto-mode-alist (cons '("\\.oz$" . oz-mode)
				auto-mode-alist)))

;; ---------------------------------------------------------------------
;; lemacs and gnu19 support
;; ---------------------------------------------------------------------

(defvar oz-lucid nil)
(defvar oz-gnu19 nil)

(cond ((string-match "Lucid" emacs-version)
       (setq oz-lucid t))
      ((string-match "19" emacs-version)
       (setq oz-gnu19 t)))

(if oz-gnu19
    (progn
      (require 'lucid)
      (defalias 'delete-extent 'delete-overlay)
      (defalias 'make-extent 'make-overlay)))

;; ---------------------------------------------------------------------
;; win32 support
;;    primary change: emulator is started by compiler, 
;;    so its output goes into the compiler buffer
;; ---------------------------------------------------------------------

(defvar oz-win32 nil)
(setq oz-win32 (eq system-type 'windows-nt))



;;------------------------------------------------------------
;; Variables/Initialization
;;------------------------------------------------------------

(defvar oz-other-map nil
  "choose alternate keybinding

t:   avoids redefinition of already used keys
nil: use gerts key bindings
")

(defvar oz-gdb-autostart t
  "*In gdb mode: start emulator immediately or not.

non-nil: Start emulator immediately.
nil:     Don't start emulator (use command 'run').
         This is useful when you want to use breakpoints.
")

(defvar oz-auto-indent t
  "*Determines whether automatic indenting is active.")

(defvar oz-indent-chars 3
  "*Indentation of Oz statements with respect to containing block.")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)
(defvar oz-mode-map (make-sparse-keymap))

(defvar oz-emulator (concat (getenv "HOME") "/Oz/Emulator/oz.emulator.bin")
  "The emulator for gdb mode and for [oz-other]")

(defvar oz-boot (concat (getenv "HOME") "/Oz/Compiler/backend/ozboot.ql")
  "The compiler for tel mode and for [oz-other]")

(defvar oz-emulator-buffer "*Oz Emulator*"
  "The buffername of the Oz Emulator output")

(defvar oz-compiler-buffer "*Oz Compiler*"
  "The buffername of the Oz Compiler output")

(defvar oz-emulator-hook nil
  "Hook used if non nil for starting the Oz Emulator.
For example
  (setq oz-emulator-hook 'oz-start-gdb-emulator)
starts the emulator under gdb")

(defvar OZ-HOME "/project/ps/oz"
  "The directory where oz is installed")

(defun oz-home ()
  (let ((ret (getenv "OZHOME")))
    (if ret
	ret
      (message "OZHOME not set using fallback: %s" OZ-HOME)
      (setenv "OZHOME" OZ-HOME)
      OZ-HOME)))

(defvar oz-doc-dir (concat (oz-home) "/doc/")
  "The default doc directory")

(defvar oz-preview "xdvi"
  "The previewer for doc files")

(defvar oz-error-string (format "%c" 17)
  "how compiler and engine signal errors")

(defconst oz-remove-pattern
  (concat oz-error-string 
	  "\\|" (char-to-string 18) "\\|" (char-to-string 19) "\\|
")
  "")


(defvar oz-want-font-lock t
  "*If t means that font-lock mode is switched on")

(defvar oz-temp-counter 0
  "gensym counter")

(defvar oz-popup-on-error t
  "*popup compiler/emulator buffer if error occured")


;;------------------------------------------------------------
;; for error location (jd)
;;------------------------------------------------------------

(defvar oz-last-fed-region-start nil
  "marker on last fed region; used to locate errors")

(defvar oz-compiler-output-start nil
  "where output of last compiler run began")

(defvar oz-next-error-marker nil 
  "remembers place of last error msg")

(defvar oz-error-intro-pattern "\\(error\\|warning\\) \\*\\*\\*\\*\\*"
  "pattern for error location")

(defvar oz-compiler-buffer-map nil)
(defvar oz-emulator-buffer-map nil)




;;------------------------------------------------------------
;; Frame title
;;------------------------------------------------------------

;; lucid supports frame-title as format string (is better ...)
;;  see function mode-line-format
;; gnu19 supports frame-title as constant string

(defvar oz-old-frame-title
  (if oz-lucid
      (setq oz-old-frame-title
	    frame-title-format)
    (if oz-gnu19
	(setq oz-old-frame-title
	      (cdr (assoc 'name (frame-parameters))))))
  "The saved window title")

(defun oz-get-title ()
  (if oz-gnu19
      (cdr (assoc 'name (frame-parameters (car (visible-frame-list)))))
      (if oz-lucid
	frame-title-format
	"")))

(defvar oz-title-format
  (concat "Oz Programming Interface ("
	  (oz-get-title) ")")
  "The format string for the window title")

(defvar oz-change-title t
  "If non-nil means change the title of the Emacs window")

(defun oz-set-title ()
  (if oz-change-title
      (if oz-gnu19
	  (mapcar '(lambda(scr)
		     (modify-frame-parameters 
		      scr
		      (list (cons 'name oz-title-format))))
		  (visible-frame-list)))
    
    (if oz-lucid
	(setq frame-title-format oz-title-format))))

(defun oz-reset-title ()
  "reset to the initial window title"
  (if oz-change-title
      (if oz-lucid
	  (setq frame-title-format oz-old-frame-title))
    (if oz-gnu19
	(mapcar '(lambda(scr)
		   (modify-frame-parameters 
		    scr
		    (list (cons 'name oz-old-frame-title))))
		(visible-frame-list)))))

(defun oz-window-system ()
  "Non-nil iff we are running under X"
  window-system)

;;------------------------------------------------------------
;; Utilities
;;------------------------------------------------------------

(defun oz-make-temp-name (name)
  "gensym implementation"
  (setq oz-temp-counter (+ 1 oz-temp-counter))
  (format "%s%d" (make-temp-name name) oz-temp-counter))

(defun oz-line-pos ()
  "get the position of line start and end (changes point!)"
  (save-excursion
    (let (beg end)
      (beginning-of-line)
      (setq beg (point))
      (end-of-line)
      (setq end (point))
      (cons beg end))))

;;------------------------------------------------------------
;; Fonts
;;------------------------------------------------------------

(defun oz-set-fontlock-keywords ()
  (setq font-lock-keywords (list oz-keywords)))

;;------------------------------------------------------------
;; Menus
;;------------------------------------------------------------
;; lucid: a menubar is a new datastructure (see function set-buffer-menubar)
;; GNU19: a menubar is a usial keysequence with prefix "menu-bar"

(defvar oz-menubar nil
  "The Oz Menubar for Lucid Emacs")

(defun oz-make-menu (list)
  (if oz-lucid
   (setq oz-menubar (oz-make-menu-lucid list)))
  (if oz-gnu19
   (oz-make-menu-gnu19 oz-mode-map
		       (list (cons "menu-bar" list)))))

(defun oz-make-menu-lucid (list)
  (if (eq list nil)
      nil
    (cons
     (let* ((entry (car list))
	    (name (car entry))
	    (rest (cdr entry)))
       (if (null rest)
	   (vector name nil nil)
	 (if (atom rest)
	     (vector name rest t)
	   (cons name (oz-make-menu-lucid rest)))))
     (oz-make-menu-lucid (cdr list)))))

(defun oz-dup-list (l)
  (if (or (null l) 
	  (not (listp l)))
      l
    (cons (oz-dup-list (car l)) (oz-dup-list (cdr l)))))

(defun oz-make-menu-gnu19 (map list)
  ;; for some unknown reason Emacs corrupts the input list
  (oz-make-menu-gnu19-1 map (oz-dup-list list)))

(defun oz-make-menu-gnu19-1 (map list)
  (if (eq list nil)
      nil
    (let* ((entry (car list))
	   (name (car entry))
	   (aname (intern name))
	   (rest (cdr entry)))
      (if (null rest)
	  (define-key map (vector (intern (oz-make-temp-name name))) entry)
	(if (atom rest)
	    (define-key map (vector aname) entry)
	  (let ((newmap (make-sparse-keymap name)))
	    (define-key map (vector aname)
	      (cons name
		    newmap))
	    (oz-make-menu-gnu19-1 newmap (reverse rest))))))
    (oz-make-menu-gnu19-1 map (cdr list))))

(defvar oz-menu
 '(("Oz"
    ("Feed Buffer"            . oz-feed-buffer)
    ("Feed Region"            . oz-feed-region)
    ("Feed Line"              . oz-feed-line)
    ("Feed Paragraph"         . oz-feed-paragraph)
    ("Feed File"              . oz-feed-file)
    ("Compile File"           . oz-precompile-file)
    ("-----")
    ("Find"
     ("Documentation Demo"      . oz-find-docdemo-file)
     ("Other Demo"              . oz-find-demo-file)
     ("Modules File"            . oz-find-modules-file)
     )
    ("Print"
     ("Region"                  . ps-print-region-with-faces)
     ("Buffer"			. ps-print-buffer-with-faces)
     )
    ("Core Syntax"
     ("Buffer"      . oz-to-coresyntax-buffer)
     ("Region"      . oz-to-coresyntax-region)
     ("Line"        . oz-to-coresyntax-line  )
     )
    ("Emulator Code"
     ("Buffer"      . oz-to-emulatorcode-buffer)
     ("Region"      . oz-to-emulatorcode-region)
     ("Line"        . oz-to-emulatorcode-line  )
     )
    ("Indent"
     ("Line"   . oz-indent-line)
     ("Region" . oz-indent-region)
     ("Buffer" . oz-indent-buffer)
     )
    ("Comment"
     ("Comment Region"   . oz-comment-region)
     ("Uncomment Region" . oz-un-comment-region)
     )
    ("Browse" 
     ("Region" . oz-feed-region-browse)
     ("Line" . oz-feed-line-browse))
    ("Panel"   . oz-view-panel)
    ("-----")
    ("Next Oz Buffer"         . oz-next-buffer)
    ("Previous Oz Buffer"     . oz-previous-buffer)
    ("New Oz Buffer"          . oz-new-buffer)
    ("Fontify Buffer"         . oz-fontify)
    ("Show/Hide"
     ("Compiler"      . oz-toggle-compiler)
     ("Emulator"      . oz-toggle-emulator)
     )
    ("-----")    
    ("Start Oz" . run-oz)
    ("Halt Oz"  . oz-halt)
    ("-----")
    ("Debugger" . oz-debug-start)
    ))

  "The contents of the Oz menu")

(autoload 'oz-emacs-connect "$OZHOME/tools/oztoemacs/oztoemacs"
  "Load the definitions for communication from Oz to Emacs" t)

(oz-make-menu oz-menu)

;;------------------------------------------------------------
;; Debugger stuff
;;------------------------------------------------------------

(defun oz-debug-start()
  "Start the debugger."
  (interactive)
  (oz-send-string "{Ozcar unhide}"))

(defun oz-debug-stop()
  "Stop the debugger."
  (interactive)
  (oz-send-string "{Ozcar hide}"))
  

;;------------------------------------------------------------
;; Start/Stop oz
;;------------------------------------------------------------

(defun run-oz ()
  "Run the Oz Compiler and Oz Emulator.
Input and output via buffers *Oz Compiler* and *Oz Emulator*."
  (interactive)
  (oz-check-running t)
  (if (not (equal mode-name "Oz"))
      (oz-new-buffer))
  (oz-show-buffer (get-buffer oz-compiler-buffer)))

(defvar oz-halt-timeout 15
  "How long to wait in oz-halt after sending the directive halt")

(defun oz-halt (force)
  (interactive "P")

  (message "halting Oz...")
  (if (get-buffer "*Oz Temp*") (kill-buffer "*Oz Temp*"))
  (if (and (get-buffer-process oz-compiler-buffer)
	   (or oz-win32 (get-buffer-process oz-emulator-buffer)))
      (if (null force)
	  (let* ((i (* 2 oz-halt-timeout))
		 (cproc (get-buffer-process oz-compiler-buffer))
		 (eproc (if oz-win32
			    cproc
			  (get-buffer-process oz-emulator-buffer))))
	    (oz-send-string "\\halt ")
	    (while (and (or (eq (process-status eproc) 'run)
			    (eq (process-status cproc) 'run))
			(> i 0))
	      (sleep-for 1)
	      (setq i (1- i))))))
  (if (get-buffer-process oz-compiler-buffer)
      (delete-process oz-compiler-buffer))
  (if (get-buffer-process oz-emulator-buffer)
      (delete-process oz-emulator-buffer))
  (message "")
  (oz-reset-title))

(defun oz-check-running (start-flag)
  (if (and (not oz-win32)
	   (get-buffer-process oz-compiler-buffer)
	   (not (get-buffer-process oz-emulator-buffer)))
      (progn
	(message "Emulator died.")
	(delete-process oz-compiler-buffer)))
  (if (and (not (get-buffer-process oz-compiler-buffer))
	   (get-buffer-process oz-emulator-buffer))
      (progn
	(message "Compiler died.")
	(delete-process oz-emulator-buffer)))
  (if (get-buffer-process oz-compiler-buffer)
      t
    (let ((file (concat (oz-make-temp-name "/tmp/ozpipeout")
			":"
			(oz-make-temp-name "/tmp/ozpipein"))))
      (if (not start-flag) (message "Oz died. Restarting ..."))
      (if oz-win32
	  (make-comint "Oz Compiler" "ozcompiler" nil "+E")
	(make-comint "Oz Compiler" "oz.compiler" nil "-emacs" "-S" file))
      (setq oz-compiler-buffer "*Oz Compiler*") 
      (oz-create-buffer oz-compiler-buffer 'compiler)
      (save-excursion
	(set-buffer oz-compiler-buffer)
	(set (make-local-variable 
	      'compilation-error-regexp-alist)
	     '(
	       ("at line \\([0-9]+\\) in file \"\\([^ \n]+[^. \n]\\)\\.?\""
		2 1)
	       ("at line \\([0-9]+\\)" 1 1)))
	(set (make-local-variable 'compilation-parsing-end)
	     (point))
	(set (make-local-variable 'compilation-error-list)
	     nil)
	(set (make-local-variable 'compilation-last-buffer)
	     (current-buffer)))
      (set-process-filter (get-buffer-process oz-compiler-buffer)
			  'oz-compiler-filter)
      (bury-buffer oz-compiler-buffer)

      (if oz-emulator-hook
	  (funcall oz-emulator-hook file)
	(setq oz-emulator-buffer "*Oz Emulator*")
	(if (not oz-win32)
	    (make-comint "Oz Emulator" "oz.emulator" nil "-emacs" "-S" file))
	(oz-create-buffer oz-emulator-buffer 'emulator)
	(if (not oz-win32)
	    (set-process-filter (get-buffer-process oz-emulator-buffer)
				'oz-emulator-filter))
	)

      (bury-buffer oz-emulator-buffer)

      (oz-set-title)
      (message "Oz started.")
      ;(sleep-for 10)
      )))




;;------------------------------------------------------------
;; GDB support
;;------------------------------------------------------------

(defun oz-set-other (comp)
  (interactive "P")
  (if comp
      (oz-set-compiler)
    (oz-set-emulator)))


(defun oz-set-emulator ()
  (interactive)
  (setq oz-emulator 
	(expand-file-name 
	 (read-file-name "Choose Emulator: "
			 nil
			 nil
			 t
			 nil)))
  (if (getenv "OZEMULATOR")
      (setenv "OZEMULATOR" oz-emulator)))

(defun oz-set-compiler ()
  (interactive)
  (setq oz-boot
	(expand-file-name 
	 (read-file-name "Choose Compiler Boot File: "
			 nil
			 nil
			 t
			 nil)))
  (if (getenv "OZBOOT")
      (setenv "OZBOOT" oz-boot)))

(defun oz-gdb ()
  (interactive)
  (if (getenv "OZ_PI")
      t
    (setenv "OZ_PI" "1")
    (if (null (getenv "OZPLATFORM"))
      (setenv "OZPLATFORM" "sunos-sparc"))
    (if (getenv "OZHOME")
	t
      (message "no OZHOME using fallback: %s" OZ-HOME)
      (setenv "OZHOME" OZ-HOME))
    (setenv "OZPATH" 
	    (concat (or (getenv "OZPATH") ".") ":"
		    (getenv "OZHOME") "/lib:"
		    (getenv "OZHOME") "/platform/" (getenv "OZPLATFORM") ":"
		    (getenv "OZHOME") "/demo"))
    (setenv "PATH"
	    (concat (getenv "PATH") ":" (getenv "OZHOME") "/bin")))

  (if oz-emulator-hook
      (setq oz-emulator-hook nil)
    (setq oz-emulator-hook 'oz-start-gdb-emulator)
    )

  (if oz-emulator-hook
      (message "gdb enabled: %s" oz-emulator)
    (message "gdb disabled")))


(defun oz-other (comp)
  (interactive "P")
  (if comp
      (oz-other-compiler)
    (oz-other-emulator)))

(defun oz-other-emulator ()
  (interactive)
  (if (getenv "OZEMULATOR")
      (setenv "OZEMULATOR" nil)
    (setenv "OZEMULATOR" oz-emulator))

  (if (getenv "OZEMULATOR")
      (message "Oz Emulator: %s" oz-emulator)
    (message "Oz Emulator: global")))

(defun oz-other-compiler ()
  (interactive)
  (if (getenv "OZBOOT")
      (setenv "OZBOOT" nil)
    (setenv "OZBOOT" oz-boot))

  (if (getenv "OZBOOT")
      (message "Oz Compiler: %s" oz-boot)
    (message "Oz Compiler: global")))

(defun oz-start-gdb-emulator (tmpfile)
  "Run gdb on oz-emulator
The directory containing FILE becomes the initial working directory
and source-file directory for GDB.  If you wish to change this, use
the GDB commands `cd DIR' and `directory'."
  (let ((old-buffer (current-buffer))
	(init-str (concat "set args -S " tmpfile "\n")))
    (if oz-gnu19 (gdb (concat "gdb " oz-emulator)))
    (if oz-lucid (gdb oz-emulator))
    (setq oz-emulator-buffer (buffer-name (current-buffer)))
    (comint-send-string
     (get-buffer-process oz-emulator-buffer)
     init-str)
    (if oz-gdb-autostart
	(comint-send-string
	 (get-buffer-process oz-emulator-buffer)
	 "run\n"))
    (switch-to-buffer old-buffer)))

;;------------------------------------------------------------
;; Feeding the compiler
;;------------------------------------------------------------

(defun oz-zmacs-stuff ()
  (if oz-lucid (setq zmacs-region-stays t)))

(defun oz-feed-buffer ()
  "Feeds the entire buffer."
  (interactive)
  (let ((file (buffer-file-name))
	(cur (current-buffer)))
    (if (and file (buffer-modified-p)
	     (y-or-n-p (format "Save buffer %s first? " (buffer-name))))
	(save-buffer))
    (if (and file (not (buffer-modified-p)))
	(oz-insert-file file)
      (oz-feed-region (point-min) (point-max)))
    (switch-to-buffer cur))
  (oz-zmacs-stuff))


(defun oz-feed-region (start end)
  "Feeds the region."
  (interactive "r")
  (oz-send-string (buffer-substring start end))
  (setq oz-last-fed-region-start (copy-marker start))
  (oz-zmacs-stuff))


(defun oz-feed-line ()
  "Feeds one line."
  (interactive)
   (let* ((line (oz-line-pos)))
     (oz-feed-region (car line) (cdr line)))
   (oz-zmacs-stuff))


(defun oz-feed-paragraph ()
  "Feeds the current paragraph."
  (interactive)
  (save-excursion
	(backward-paragraph 1)
	(let ((start (point)))
	  (forward-paragraph 1)
	  (oz-feed-region start (point))))
  )

(defun oz-send-string (string)
  (oz-check-running nil)
  (let ((proc (get-buffer-process oz-compiler-buffer)))
    (comint-send-string proc string)
    (comint-send-string proc "\n")
    (save-excursion
       (set-buffer oz-compiler-buffer)
       (setq oz-compiler-output-start (point-max))
;       (comint-send-eof)
       (comint-send-string proc "") ;; works under win32 as well
       (setq oz-next-error-marker nil)
;       (setq compilation-parsing-end (point))
       )
    ))

;;------------------------------------------------------------
;; Feeding the emulator
;;------------------------------------------------------------

(defun oz-continue ()
  "continue the Oz Emulator after an error"
  (interactive)
  (comint-send-string (get-buffer-process oz-emulator-buffer) "c\n"))

;;------------------------------------------------------------
;; electric
;;------------------------------------------------------------

(defun oz-electric-terminate-line ()
  "Terminate line and indent next line."
  (interactive)
  (cond (oz-auto-indent (oz-indent-line)))
  (delete-horizontal-space) ; Removes trailing whitespaces
  (newline)
  (cond (oz-auto-indent (oz-indent-line)))
)

;;------------------------------------------------------------
;;Indent
;;------------------------------------------------------------

(defun oz-make-keywords-for-match (args)
  (concat "\\<\\("
	  (mapconcat 'identity args "\\|")
	  "\\)\\>"))

(defconst oz-keywords
   (concat
    (oz-make-keywords-for-match
     '(
       "proc" "fun" "local" "declare" "in" "end"
       "if" "or" "dis" "choice" "case" "then" "andthen" "orelse" 
       "else" "elseif" "of" "elseof" "elsecase"
       "class" "from" "with" "self"
       "attr" "feat" "prop" "meth" 
       "true" "false" "unit"
       "div" "mod" 
       "condis" "not"
       "thread" "try" "catch" "raise" "lock" "finally" "skip" "fail"
       ))
    "\\|\\.\\|\\[\\]\\|#\\|!\\|:\\|\\@\\|\\,"
    ))

(defconst oz-declare-pattern (oz-make-keywords-for-match '("declare")))

(defconst oz-begin-pattern
      (oz-make-keywords-for-match 
	         '(
		   "proc" "fun" "try"
		   "local"
		   "if" "or" "dis" "choice" "case"
		   "class" "meth"
		   "not" "thread" "lock"
		   "condis" "thread" "raise"
		   )))

(defconst oz-left-pattern "[[({]")
(defconst oz-right-pattern "[])}]")

(defconst oz-end-pattern
      (oz-make-keywords-for-match '("end")))

(defconst oz-between-pattern 
      (concat (oz-make-keywords-for-match
	       '("from" "attr" "feat" "prop" "with"
		 ))))

(defconst oz-middle-pattern 
  (concat (oz-make-keywords-for-match
	   '(
	     "in" "then" "else" "elseif" "of" "elseof" "elsecase"
	     "catch" "finally"
	     ))
	  "\\|" "\\[\\]"))

(defconst oz-feat-end-pattern
  ":[ \t]*")

(defconst oz-key-pattern
      (concat oz-declare-pattern "\\|" oz-begin-pattern "\\|"
	      oz-between-pattern "\\|"
	      oz-left-pattern "\\|" oz-right-pattern "\\|"
	      oz-middle-pattern "\\|" oz-end-pattern
	      ))

(defun oz-indent-buffer ()
  (interactive)
  (goto-char 0)
  (while (< (point) (point-max))
    (oz-indent-line t)
    (forward-line 1)))

(defun oz-indent-region (b e)
  (interactive "r")
  (let ((end (copy-marker e)))
    (goto-char b)
    (while (< (point) end)
      (oz-indent-line t)
      (forward-line 1))))

;; indent one line
;;  no-empty-line arg is delegated to oz-calc-indent
;; algm:
;;   
;;   call oz-call-indent with point at first no-blank character
;;   if output-column is
;;     <0 -> do nothing
;;     unchanged -> do nothing
;;     >0 -> insert this number of blanks
;;   set position of point at end of initial blanks, if not yet behind them
(defun oz-indent-line (&optional no-empty-line)
  (interactive)
  (let ((old-cc case-fold-search))
    (setq case-fold-search nil) ;;; respect case
    (unwind-protect
	(save-excursion
	  (beginning-of-line)
	  (skip-chars-forward " \t")
	  (let ((col (save-excursion (oz-calc-indent no-empty-line))))
	    (cond ((< col 0) t)
		  ((= col (current-column)) t)
		  (t (delete-horizontal-space)
		     (indent-to col)))))
      (if (oz-is-left)
	  (skip-chars-forward " \t"))
      (setq case-fold-search old-cc))))

;; calculate the indent column (<0 means: don't change)
;; pre: point is at beginning of text
;; arg: no-empty-line: t = do not indent empty lines and comment lines
(defun oz-calc-indent (no-empty-line)
  (cond ((and no-empty-line (oz-is-empty))
	  ;; empty lines are not changed
	 -1)
	((looking-at oz-declare-pattern)
	 0)
	((looking-at "\\\\")
	 0)
	((looking-at "%%%")
	 0)
	((looking-at "%[^%]")
	 -1)
	((oz-is-field-value)
	 (oz-search-matching-paren)
	 (+ (oz-indent-after-paren) oz-indent-chars))
	((or (looking-at oz-end-pattern) (looking-at oz-middle-pattern))
	 ;; we must indent to the same column as the matching begin
	 (oz-search-matching-begin nil))
	((looking-at oz-right-pattern)
	 (oz-search-matching-paren))
	((looking-at oz-between-pattern)
	 (let ((col (oz-search-matching-begin nil)))
	   (if (< col 0)
	       -1
	     (+ col oz-indent-chars))))
	((or (looking-at oz-begin-pattern)
	     (looking-at oz-left-pattern)
	     (looking-at "$")
	     (looking-at "%"))
	 (let (col (ret nil))
	   (while (not ret)
	     (setq col (oz-search-matching-begin t))
	     (if (< col 0)
		 (cond ((= col -1)
			(setq ret col))
		       ((= col -3) ;; start of file
			(setq ret 0))
		       ;; previous block begin found:
		       ;; check if is first in column
		       ((= col -2)
			(let ((found-col (current-column))
			      first-col)
			  (beginning-of-line)
			  (skip-chars-forward " \t")
			  (setq first-col (current-column))
			  (if (= first-col found-col)
			      (setq ret first-col))))
		       (t (error "mm2: special hack")))
	       (cond ((looking-at oz-declare-pattern)
		      (setq ret (current-column)))
		     ((looking-at oz-left-pattern)
		      (setq ret (oz-indent-after-paren)))
		     ((looking-at oz-begin-pattern)
		      (setq ret (+ (current-column) oz-indent-chars)))
		     ((= (point) 1)
		      (setq ret 0))
		     (t
		      (error "mm2: never be here")
		      (setq ret (+ (current-column) oz-indent-chars))))))
	   ret))
	((looking-at ",")
	 (search-backward "," nil t)
	 (current-column))
	(t (oz-calc-indent1))
	)
  )

;; the heavy case
;; backward search for the next oz-key-pattern
(defun oz-calc-indent1 ()
  (if (re-search-backward
       (concat oz-key-pattern 
;mm2	       "\\|" "^[ \t]*[^ \t\n]"
	       )
       0 t)
      (cond ((oz-comment-start)
	     (oz-calc-indent1))
	    ((looking-at oz-declare-pattern)
	     (current-column))
	    ((looking-at "\\<in\\>")
	     ;; we are the first token after an 'in'
	     (oz-search-matching-begin nil)
	     (if (looking-at oz-declare-pattern)
		 (current-column)
	       (+ (current-column) oz-indent-chars)))
	    ((looking-at oz-begin-pattern)
	     ;; we are the first token after 'if' 'proc' ...
	     (+ (current-column) oz-indent-chars))
	    ((looking-at oz-left-pattern)
	     ;; we are the first token after '(' '{' ...
	     (let ((col (current-column)))
	       (goto-char (match-end 0))
	       (if (oz-is-right)
		   (+ col 2)
		 (re-search-forward "[^ \t]")
		 (1- (current-column)))))
	    ((looking-at oz-middle-pattern)
	     ;; we are the first token after 'then of'
	     (let ((col (oz-search-matching-begin nil)))
	       (if (< col 0)
		   -1
		 (+ col oz-indent-chars))))
	    ((looking-at oz-between-pattern)
	     ;; we are the first token after 'attr feat'
	     (+ (current-column) oz-indent-chars))
	    ((looking-at oz-end-pattern)
	     ;; we are the first token after an 'fi' 'end'
	     (oz-search-matching-begin nil)
	     (oz-calc-indent1)
	     ;; (current-column)
	     )
	    ((looking-at oz-right-pattern)
	     ;; we are the first token after an ')' '}'
	     (oz-search-matching-paren)
	     (oz-calc-indent1)
	     )
	    (t
	     ;; else impossible
	     (error "mm2: here"))
	    )
    ;; else: nothing found: beginning of file
    0
    )
  )

;; check for record fields with seperate line for feature and value
;;  f( arity:
;;        myarity)
(defun oz-is-field-value ()
  (let ((old (point)))
    (skip-chars-backward " \t\n")
    (if (= (point) 1)
	t
      (backward-char))
    (if (and (looking-at ":") (null (oz-comment-start)))
	t
      (goto-char old)
      nil)))

;; calculate the start position for features after '('
(defun oz-indent-after-paren ()
  (let ((col (current-column)))
    (forward-char)
    (if (oz-is-right)
	(+ col 2)
      (re-search-forward "[^ \t]")
      (1- (current-column)))))

(defun oz-comment-start ()
  (let ((p (point)))
    (re-search-backward "%\\|^" (point-min) t)
    (if (looking-at "%")
	t
      (goto-char p)
      nil)))

(defun oz-is-empty ()
  (and (oz-is-left) (oz-is-right)))

(defun oz-is-left ()
  (save-excursion
    (skip-chars-backward " \t")
    (if (= (current-column) 0)
	t
      nil)))

(defun oz-is-right ()
  (if (looking-at "[ \t]*$")
      t
    nil))

(defun oz-search-matching-begin (search-paren)
  (let ((ret nil)
	(nesting 0)
	(second-nesting nil))
    (while (not ret)
      (if (re-search-backward oz-key-pattern 0 t) 
	  (cond ((oz-comment-start)
		 t)
		((looking-at oz-declare-pattern)
		 (setq ret (current-column)))
		((looking-at oz-begin-pattern)
		 ;; 'if'
		 (if (= nesting 0)
		     (setq ret (current-column))
		   (if (and search-paren second-nesting (= nesting 1))
		       (setq ret -2)
		     (setq nesting (- nesting 1)))))
		((looking-at oz-left-pattern)
		 ;; '('
		 (if (and search-paren (= nesting 0))
		     (setq ret (current-column))
		   (message "unbalanced open paren.")
		   (setq ret -1)))
		((looking-at oz-middle-pattern)
		 ;; 'then' '[]'
		 t)
		((looking-at oz-between-pattern)
		 ;; 'attr' 'feat'
		 t)
		((looking-at oz-end-pattern)
		 ;; 'end'
		 (setq second-nesting t)
		 (setq nesting (+ nesting 1)))
		((looking-at oz-right-pattern)
		 (oz-search-matching-paren))
		(t (error "mm2: beg"))
		)
	(goto-char 1)
	(if (= nesting 0)
	    (setq ret -3)
	  (message "no matching begin token")
	  (setq ret -1))))
    ret))

(defun oz-search-matching-paren ()
  (let ((do-loop t)
	(nesting 0))
    (while do-loop
      (if (re-search-backward (concat oz-left-pattern "\\|" oz-right-pattern)
			      0 t) 
	  (cond ((oz-comment-start)
		 t)
		((looking-at oz-left-pattern)
		 ;; '('
		 (if (= nesting 0)
		     (setq do-loop nil)
		   (setq nesting (- nesting 1))))
		((looking-at oz-right-pattern)
		 ;; ')'
		 (setq nesting (+ nesting 1)))
		(t (error "mm2: paren"))
		)
	(message "no matching open paren")
	(setq do-loop nil))))
  (current-column))

;;------------------------------------------------------------
;; oz-mode
;;------------------------------------------------------------

;(setq oz-mode-syntax-table nil)
(if oz-mode-syntax-table
    ()
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?_ "w" table)
    (modify-syntax-entry ?\\ "\\" table)
    (modify-syntax-entry ?+ "." table)
    (modify-syntax-entry ?- "." table)
    (modify-syntax-entry ?= "." table)
    (modify-syntax-entry ?< "." table)
    (modify-syntax-entry ?> "." table)
    (modify-syntax-entry ?\" "\"" table)
    (modify-syntax-entry ?\' "\"" table)
    (modify-syntax-entry ?\` "\"" table)
    (modify-syntax-entry ?%  "<" table)
    (modify-syntax-entry ?\n ">" table)
    (modify-syntax-entry ?/ ". 14" table)
    (modify-syntax-entry ?* ". 23b" table)
    (modify-syntax-entry ?. "_" table)
    (setq oz-mode-syntax-table table)
    (set-syntax-table oz-mode-syntax-table)))

(define-abbrev-table 'oz-mode-abbrev-table ())

(defun oz-mode-variables ()
  (set-syntax-table oz-mode-syntax-table)
  (setq local-abbrev-table oz-mode-abbrev-table)
  (make-local-variable 'paragraph-start)
  (setq paragraph-start (concat "^$\\|" page-delimiter))
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'paragraph-ignore-fill-prefix)
  (setq paragraph-ignore-fill-prefix t)
  (make-local-variable 'fill-paragraph-function)
  (setq fill-paragraph-function 'oz-fill-paragraph)
;  (make-local-variable 'auto-fill-function)
;  (setq auto-fill-function 'oz-auto-fill)
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'oz-indent-line)
  (make-local-variable 'comment-start)
  (setq comment-start "%")
  (make-local-variable 'comment-end)
  (setq comment-end "")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "/\\*+ *\\|% *")
  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)
  (set (make-local-variable 'compilation-last-buffer)
       (get-buffer-create oz-compiler-buffer))
)

(defun oz-mode-commands (map)
  (define-key map "\t" 'oz-indent-line)
  (if oz-other-map
      (progn
	(define-key map "\C-c\C-f\C-b" 	'oz-feed-buffer)
	(define-key map "\C-c\C-f\C-r"	'oz-feed-region)
	(define-key map "\C-c\C-f\C-l"	'oz-feed-line)
	(define-key map "\C-c\C-f\C-f"	'oz-feed-file)
	(define-key map "\C-c\C-i"      'oz-feed-file)
	(define-key map "\C-c\C-f\C-p"	'oz-feed-paragraph)
	
	(define-key map "\C-c\C-p\C-f"	'oz-precompile-file)

	(define-key map "\C-c\C-b\C-l"	'oz-feed-line-browse)
	(define-key map "\C-c\C-b\C-r"  'oz-feed-region-browse)
	)
    (define-key map "\C-c\C-f"     'oz-feed-file)
    (define-key map "\M-\C-m"      'oz-feed-buffer)
    (define-key map "\M-r"         'oz-feed-region)
    (define-key map "\M-l"         'oz-feed-line)
    (define-key map "\C-c\C-p"     'oz-feed-paragraph)
    (define-key map "\C-cb"        'oz-feed-line-browse)
    (define-key map "\C-c\C-b"     'oz-feed-region-browse)
    (define-key map "\M-n"         'oz-next-buffer)
    (define-key map "\M-p"         'oz-previous-buffer)

    (define-key map "\C-c\C-d\C-r" 'oz-debug-start)
    (define-key map "\C-c\C-d\C-d" 'oz-debug-devel-start)
    (define-key map "\C-c\C-d\C-h" 'oz-debug-stop)
    )

  (define-key map "\M-\C-x"	'oz-feed-paragraph)
  (define-key map "\C-c\C-c"    'oz-toggle-compiler)

  (define-key map [(control c) (control h)] 'oz-halt)
  (define-key map "\C-c\C-h"    'oz-halt)

  (define-key map "\C-c\C-e"    'oz-toggle-emulator)
  (define-key map "\C-c\C-n"    'oz-new-buffer)
  (define-key map "\C-c\C-l"    'oz-fontify)
  (define-key map "\C-c\C-r"    'run-oz)
  (define-key map "\C-cc"       'oz-precompile-file)
  (define-key map "\C-cm"       'oz-set-other)
  (define-key map "\C-co"       'oz-other)
  (define-key map "\C-cd"       'oz-gdb)
  (define-key map "\r"		'oz-electric-terminate-line)
  (define-key map "\C-cp"       'oz-view-panel)

  ;; by j.doerre
  (define-key map "\M-\C-f" 'forward-oz-expr)
  (define-key map "\M-\C-b" 'backward-oz-expr)
  (define-key map "\M-\C-k" 'kill-oz-expr)
  (define-key map "\M-\C-@" 'mark-oz-expr)
  (define-key map [(meta control space)] 'mark-oz-expr)
  (define-key map [(meta control backspace)] 'backward-kill-oz-expr)
  (define-key map [(meta control delete)] 'backward-kill-oz-expr)
  (define-key map "\M-\C-t" 'transpose-oz-exprs)

  ;; error location
  (define-key oz-mode-map "\C-x`" 'oz-goto-next-error)
  )

(oz-mode-commands oz-mode-map)

(defun oz-mode ()
  "Major mode for editing Oz code.
Commands:
\\{oz-mode-map}
Entry to this mode calls the value of oz-mode-hook
if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map oz-mode-map)
  (setq major-mode 'oz-mode)
  (setq mode-name "Oz")
  (oz-mode-variables)
  (if (and oz-lucid (not (assoc "Oz" current-menubar)))
   (set-buffer-menubar (oz-insert-menu oz-menubar current-menubar)))

  ; font lock stuff
  (oz-set-fontlock-keywords)
  (if (and oz-want-font-lock (oz-window-system))
      (font-lock-mode 1))
  (run-hooks 'oz-mode-hook))

;; do not put Oz menu too much to the right
(defun oz-insert-menu (menu list)
  (if (eq nil list)
      menu
    (if (eq (car list) nil)
	(cons (car menu) list)
      (cons (car list)
	    (oz-insert-menu menu (cdr list))))))

;;;; Lisp paragraph filling commands.

(defun oz-fill-paragraph (&optional justify)
  "Like \\[fill-paragraph], but handle Oz comments.
If any of the current line is a comment, fill the comment or the
paragraph of it that point is in, preserving the comment's indentation
and initial semicolons."
  (interactive "P")
  (let (
	;; Non-nil if the current line contains a comment.
	has-comment

	;; If has-comment, the appropriate fill-prefix for the comment.
	comment-fill-prefix
	)

    ;; Figure out what kind of comment we are looking at.
    (save-excursion
      (beginning-of-line)
      (cond

       ;; A line with nothing but a comment on it?
       ((looking-at "[ \t]*%[% \t]*")
	(setq has-comment t
	      comment-fill-prefix (buffer-substring (match-beginning 0)
						    (match-end 0))))

       ;; A line with some code, followed by a comment?  Remember that the
       ;; semi which starts the comment shouldn't be part of a string or
       ;; character.
       ((progn
	  (while (not (looking-at "%\\|$"))
	    (skip-chars-forward "^%\n\\\\")
	    (cond
	     ((eq (char-after (point)) ?\\) (forward-char 2))))
	  (looking-at "%+[\t ]*"))
	(setq has-comment t)
	(setq comment-fill-prefix
	      (concat (make-string (current-column) ? )
		      (buffer-substring (match-beginning 0) (match-end 0)))))))

    (if (not has-comment)
	(fill-paragraph justify)

      ;; Narrow to include only the comment, and then fill the region.
      (save-restriction
	(narrow-to-region
	 ;; Find the first line we should include in the region to fill.
	 (save-excursion
	   (while (and (zerop (forward-line -1))
		       (looking-at "^[ \t]*%")))
	   ;; We may have gone to far.  Go forward again.
	   (or (looking-at "^[ \t]*%")
	       (forward-line 1))
	   (point))
	 ;; Find the beginning of the first line past the region to fill.
	 (save-excursion
	   (while (progn (forward-line 1)
			 (looking-at "^[ \t]*%")))
	   (point)))

	;; Lines with only semicolons on them can be paragraph boundaries.
	(let ((paragraph-start (concat paragraph-start "\\|^[ \t%]*$"))
	      (paragraph-separate (concat paragraph-start "\\|^[ \t%]*$"))
	      (fill-prefix comment-fill-prefix))
	  (fill-paragraph justify))))
    t))

;; oz auto fill not impl (mm)
;(defun oz-auto-fill ()
;  (let ((start (oz-comment-start)))
;  (message "Oz auto fill: not implemented"))

;;------------------------------------------------------------
;; Fontification
;;------------------------------------------------------------

(if (oz-window-system) (require 'font-lock))

(defun oz-fontify-buffer ()
  (interactive)
  (if (oz-window-system) (font-lock-fontify-buffer)))

(defun oz-fontify-region (beg end)
  (if (oz-window-system) (font-lock-fontify-region beg end)))

(defun oz-fontify (&optional arg)
  (interactive "P")
  (recenter arg)
  (oz-fontify-buffer))

;;------------------------------------------------------------
;; Filtering process output
;;------------------------------------------------------------

(defun oz-emulator-filter (proc string)
  (oz-filter proc string (process-buffer proc)))


;; 
;; Under Win32 emulator is started by compiler, 
;; so its output goes into the compiler buffer
;; 

;; if you ever change these constants
;; adapt Compiler/frontend/cFunctions.cc
(defvar oz-emulator-output-start nil)
(defvar oz-emulator-output-end nil)

(setq oz-emulator-output-start (char-to-string 5))
(setq oz-emulator-output-end   (char-to-string 6))

(defvar oz-read-emulator-output nil)

(defun oz-current-switch ()
  (if oz-read-emulator-output 
      oz-emulator-output-end
    oz-emulator-output-start))

(defun oz-current-outbuffer ()
  (if oz-read-emulator-output 
      oz-emulator-buffer
    oz-compiler-buffer))

(defun oz-compiler-filter (proc string)
  (if (not oz-win32)
      (oz-filter proc string (process-buffer proc))

    (let ((switch (string-match (oz-current-switch) string)))
      (if (null switch)
	  (oz-filter proc string (oz-current-outbuffer))
	(oz-filter proc (substring string 0 switch) (oz-current-outbuffer))
	(setq oz-read-emulator-output (not oz-read-emulator-output))
	(oz-compiler-filter proc (substring string (1+ switch)))
))))


(defun oz-filter (proc string newbuf)
;; see elisp manual: "Filter Functions"
  (let ((old-buffer (current-buffer))
	(errs-found (and oz-popup-on-error (string-match oz-error-string string))))
    (unwind-protect
	(let (moving old-point index)
	  (set-buffer newbuf)
	  (setq moving (= (point) (process-mark proc)))
	  (save-excursion

	    ;; Insert the text, moving the process-marker.
	    (goto-char (process-mark proc))

	    (setq old-point (point))
	    (goto-char (point-max))

	    ;; Irix outputs garbage, when sending EOF
	    (setq index (string-match "\\^D" string))
	    (if index
		(setq string (concat (substring string 0 index)
				     (substring string (+ 4 index)))))

	    (insert-before-markers string)
	    (set-marker (process-mark proc) (point))

	    ;; remove escape characters
	    (goto-char old-point)
	    (while (search-forward-regexp oz-remove-pattern nil t)
	      (replace-match "" nil t)))
	  (if (or moving errs-found) (goto-char (process-mark proc))))
      (set-buffer old-buffer))

    (if errs-found (oz-show-buffer newbuf))))


;;------------------------------------------------------------
;; buffers
;;------------------------------------------------------------

(defvar oz-other-buffer-percent 35 
  "
How many percent of the actual screen will be occupied by the
OZ compiler, emulator and error window")

(defun oz-show-buffer (buffer)
  (if (get-buffer-window buffer)
      t
    (save-excursion
      (let* ((win (or (get-buffer-window oz-emulator-buffer)
		      (get-buffer-window oz-compiler-buffer)
		      (split-window (get-largest-window)
				    (/ (* (window-height (get-largest-window))
					  (- 100 oz-other-buffer-percent))
				       100)))))
	(set-window-buffer win buffer)

	;; set point in window to end
	(set-buffer buffer) (set-window-point win (point-max))

	(bury-buffer buffer)))))

(defun oz-create-buffer (buf which)
  (save-excursion
    (set-buffer (get-buffer-create buf))

    (cond ((eq which 'compiler)
	   ;; enter oz-mode but no highlighting; use own map, inherit
	   ;; from oz-mode-map
	   (kill-all-local-variables)
    (if (null oz-compiler-buffer-map)
	      (setq oz-compiler-buffer-map (copy-keymap oz-mode-map)))
	   (use-local-map oz-compiler-buffer-map)
	   (setq mode-name "Oz-Output")
	   (setq major-mode 'oz-mode))

	  ((eq which 'emulator)
	   (if (null oz-emulator-buffer-map)
	       (setq oz-emulator-buffer-map (copy-keymap comint-mode-map)))
	   (use-local-map oz-emulator-buffer-map)))

    (oz-set-mouse-error-key)   ;; set mouse-2 key

    (if oz-lucid
     (set-buffer-menubar (append current-menubar oz-menubar)))
    (delete-region (point-min) (point-max))))


(defun oz-toggle-compiler ()
  (interactive)
  (oz-toggle-window oz-compiler-buffer))


(defun oz-toggle-emulator ()
  (interactive)
  (oz-toggle-window oz-emulator-buffer))



(defun oz-toggle-window (buffername)
  (let ((buffer (get-buffer buffername)))
    (if buffer
	(let ((win (get-buffer-window buffername nil)))
	  (if win
	      (save-excursion
		(set-buffer buffer)
		(if (= (window-point win) (point-max))
		  (if oz-gnu19
		      (delete-windows-on buffername t)
		    (delete-windows-on buffername))
		  (set-window-point win (point-max))))
	    (oz-show-buffer (get-buffer buffername)))))))

(defun oz-new-buffer ()
  (interactive)
  (switch-to-buffer (generate-new-buffer "Oz"))
  (oz-mode))

(defun oz-previous-buffer ()
  (interactive)
  (bury-buffer)
  (oz-walk-trough-buffers (buffer-list)))

(defun oz-next-buffer ()
  (interactive)
  (oz-walk-trough-buffers (reverse (buffer-list))))

(defun oz-walk-trough-buffers (bufs)
  (let ((bool t)
	(cur (current-buffer)))
    (while (and bufs bool)
      (set-buffer (car bufs))
      (if (equal mode-name "Oz")
	  (progn (switch-to-buffer (car bufs))
		 (setq bool nil))
	  (setq bufs (cdr bufs))))
    (if (null bool)
	t
      (set-buffer cur)
      (error "No other oz-buffer"))))

;;------------------------------------------------------------
;; Misc Goodies
;;------------------------------------------------------------

(defun oz-comment-region (beg end arg)
  (interactive "r\np")
  (comment-region beg end arg))

(defun oz-un-comment-region (beg end arg)
  (interactive "r\np")
  (comment-region beg end (if (= arg 0) -1 (- 0 arg))))

(defvar oz-temp-file (oz-make-temp-name "/tmp/ozemacs") "")

(defun oz-to-coresyntax-buffer ()
  (interactive)
  (oz-to-coresyntax-region (point-min) (point-max)))

(defun oz-to-coresyntax-line ()
  (interactive)
  (let ((line (oz-line-pos)))
    (oz-to-coresyntax-region (car line) (cdr line))))

(defun oz-to-coresyntax-region (start end)
   (interactive "r")
   (oz-directive-on-region start end "\\core" ".ozc" t))

(defun oz-to-emulatorcode-buffer ()
  (interactive)
  (oz-to-emulatorcode-region (point-min) (point-max)))

(defun oz-to-emulatorcode-line ()
  (interactive)
  (let ((line (oz-line-pos)))
    (oz-to-emulatorcode-region (car line) (cdr line))))

(defun oz-to-emulatorcode-region (start end)
   (interactive "r")
   (oz-directive-on-region start end "\\machine" ".ozm" nil))

(defun oz-directive-on-region (start end directive suffix mode)
  "Applies a directive to the region."
   (let ((file-1 (concat oz-temp-file ".oz"))
	 (file-2 (concat oz-temp-file suffix)))
     (if (file-exists-p file-2)
	 (delete-file file-2))
     (write-region start end file-1)
     (message "")
     (shell-command (concat "touch " file-2))
     (if (get-buffer "*Oz Temp*") 
	 (progn (delete-windows-on "*Oz Temp*")
		(kill-buffer "*Oz Temp*")))
     (start-process "Oz Temp" "*Oz Temp*" "tail" "+1f" file-2)
     (message "")
     (oz-send-string (concat directive " '" file-1 "'"))
     (let ((buf (get-buffer "*Oz Temp*")))
       (oz-show-buffer buf)
       (if mode
	   (save-excursion
	     (set-buffer buf)
	     (oz-mode)
	     (oz-fontify-buffer))))))

(defun oz-feed-region-browse (start end)
  "Feed the current region into the Oz Compiler"
  (interactive "r")
  (let ((contents (buffer-substring start end)))
    (oz-send-string (concat "{Browse " contents "}"))
    (setq oz-last-fed-region-start (copy-marker start))))

(defun oz-feed-line-browse ()
  "Feed the current line into the Oz Compiler"
  (interactive)
   (let ((line (oz-line-pos)))
     (oz-feed-region-browse (car line) (cdr line))))

(defun oz-view-panel ()
  "Feed {Panel open} into the Oz Compiler"
  (interactive)
  (oz-send-string "{Panel open}"))

(defun oz-feed-file (file)
  "Feed an file into the Oz Compiler"
  (interactive "FFeed file: ")
  (oz-send-string (concat "\\feed '" file "'"))) 

;;(defun oz-insert-file (file)
;;  "Insert an file into the Oz Compiler"
;;  (if oz-step-mode
;;      (oz-send-string (concat 
;;            "local T = {Thread.this} thread {Ozcar add(T)} end "
;;            "{Thread.suspend T} in \n\\insert '" file "'\nend"))
;;    (oz-send-string (concat "\\threadedfeed '" file "'"))))

(defun oz-insert-file (file)
  "Insert an file into the Oz Compiler"
    (oz-send-string (concat "\\threadedfeed '" file "'")))

(defun oz-precompile-file (file)
  "precompile an Oz file"
  (interactive "FPrecompile file: ")
  (oz-send-string (concat "\\precompile '" file "'"))) 

(defun oz-find-dvi-file ()
  "preview a file from  Oz doc directory"
  (interactive)
  (let ((name (read-file-name
	       "Preview File: "
	       oz-doc-dir
	       nil t)))
    (if (file-exists-p name)
	(start-process "OZ Doc" "*Preview*" oz-preview name)
      (error "file %s doesn't exist" name))))

(defun oz-find-docu-file ()
  "find a text in the doc directory"
  (interactive)
  (oz-find-file "Find doc file: " "doc/"))

(defun oz-find-demo-file ()
  "find a Oz file in the demo directory"
  (interactive)
  (oz-find-file "Find demo file: " "demo/"))

(defun oz-find-docdemo-file ()
  "find a Oz file in the demo/documentation directory"
  (interactive)
  (oz-find-file "Find demo file: " "demo/documentation/"))

(defun oz-find-modules-file ()
  "find a Oz file in the lib directory"
  (interactive)
  (oz-find-file "Find modules file: " "lib/"))

(defun oz-find-file (prompt file)
  (find-file (read-file-name prompt
			     (concat (oz-home) "/" file)
			     nil
			     t
			     nil)))



;;; OZ expressions hopping

;;; simulation of the -sexp functions for OZ expressions (i.e. support
;;; for moving over complete proc (fun, meth etc.) ... end blocks)

;;; Method: We use scan-sexp and count keywords delimiting OZ
;;; expressions appropriately. When an infix keyword (like in, then,
;;; but also attr) is encountered, this is treated like white space.

;;; Limitations: 
;;; 1) The method used means that we might happily hop over balanced 
;;; s-expressions (i.e. delimited with parens, braces ...) in which OZ
;;; keywords are not properly balanced. 
;;; 2) When encountering an infix expression, it is ambiguous which
;;; expression to move over, the sub-expression or the whole. We
;;; choose to always move over the smallest balanced expression
;;; (i.e. the first subexpression).
;;; 3) transpose-oz-expr is not very useful, since it does not know,
;;; when space must be inserted.


;;; unfortunately one more pattern, since those used for indenting
;;; are not exactly what is required; but they are used to define the
;;; new one here


(defconst oz-expr-between-pattern
  (concat oz-declare-pattern "\\|" oz-between-pattern "\\|"
	  oz-middle-pattern)) 


(defun forward-oz-expr (&optional arg)
  "Move forward one balanced expression (OZ expression).
With argument, do it that many times. Negative ARG means backwards."
  (interactive "p")
  (or arg (setq arg 1))
  (if (< arg 0) (backward-oz-expr (- arg))
    (while (> arg 0)
      (let ((pos (scan-sexps (point) 1))
	    keyword-kind)
	(if (equal pos nil)
	    (progn (end-of-buffer) (setq arg 0))
	  (goto-char pos)
	  (and (= (char-syntax (preceding-char)) ?w)
	       (save-excursion
		 (forward-word -1)
		 (cond ((looking-at oz-begin-pattern)
			(setq keyword-kind 'begin))
		       ((looking-at oz-expr-between-pattern)
			(setq keyword-kind 'between))
		       ((looking-at oz-end-pattern)
			(setq keyword-kind 'end))))
	       (cond ((eq keyword-kind 'begin)
		      (oz-goto-matching-end 1))
		     ((eq keyword-kind 'end)
		      (error "Containing expression ends"))
		     (t ;; (eq keyword-kind 'between)
		      (setq arg (1+ arg)))))
	  (setq arg (1- arg)))))))
      


(defun oz-goto-matching-end (nest-level)
  ;; move point to NEST-LEVELth unbalanced "end"
  (let ((loop t)
	found
	(pos (point)))
    (while loop
      (setq pos (scan-sexps pos 1))
      (if (equal pos nil)
	  (setq loop nil
		pos (point-max))
	(goto-char pos)
	(if (= (char-syntax (preceding-char)) ?w)
	    (and (forward-word -1)
		 (cond ((looking-at oz-end-pattern)
			(setq nest-level (1- nest-level)))
		       ((looking-at oz-begin-pattern)
			(setq nest-level (1+ nest-level)))
		       (t t))
		 (goto-char pos)))
	(if (< nest-level 1)
	    (setq loop nil
		  found t))))
    (goto-char pos)
    (if found
	t
      (error "Unbalanced OZ expression"))))


(defun backward-oz-expr (&optional arg)
  "Move backward one balanced expression (OZ expression).
With argument, do it that many times. Argument must be positive."
  (interactive "p")
  (or arg (setq arg 1))
  (while (> arg 0)
    (let ((pos (scan-sexps (point) -1)))
      (if (equal pos nil)
	  (progn (beginning-of-buffer) (setq arg 0))
	(goto-char pos)
	(cond ((looking-at oz-end-pattern)
	       (oz-goto-matching-begin 1))
	      ((looking-at oz-expr-between-pattern)
	       (setq arg (1+ arg)))
	      ((looking-at oz-begin-pattern)
	       (error "Containing expression ends")))
	(setq arg (1- arg))))))


(defun oz-goto-matching-begin (nest-level)
  ;; move point to NEST-LEVELth unbalanced begin of block
  (let ((loop t)
	found
	(pos (point)))
    (while loop
      (setq pos (scan-sexps pos -1))
      (if (equal pos nil)
	  (setq loop nil
		pos (point-min))
	(goto-char pos)
	(if (= (char-syntax (following-char)) ?w)
	    (cond ((looking-at oz-end-pattern)
		   (setq nest-level (1+ nest-level)))
		  ((looking-at oz-begin-pattern)
		   (setq nest-level (1- nest-level)))
		  (t t)))
	(if (< nest-level 1)
	    (setq loop nil
		  found t))))
    (goto-char pos)
    (if found
	t
      (error "Unbalanced OZ expression"))))



;; the other functions (mark-, transpose-, kill-(bw)-oz-expr are 
;; straightforward adaptions of their "sexpr"-counterparts


(defun mark-oz-expr (arg)
  "Set mark ARG balanced OZ expressions from point.
The place mark goes is the same place \\[forward-oz-expr] would
move to with the same argument."
  (interactive "p")
  (push-mark
    (save-excursion
      (forward-oz-expr arg)
      (point))
    nil t))


(defun transpose-oz-exprs (arg)
  "Like \\[transpose-words] but applies to balanced OZ expressions.
Does not work in all cases."
  (interactive "*p")
  (transpose-subr 'forward-oz-expr arg))


(defun kill-oz-expr (arg)
  "Kill the balanced  OZ expression following the cursor.
With argument, kill that many OZ expressions after the cursor.
Negative arg -N means kill N OZ expressions before the cursor."
  (interactive "p")
  (let ((pos (point)))
    (forward-oz-expr arg)
    (kill-region pos (point))))

(defun backward-kill-oz-expr (arg)
  "Kill the balanced OZ expression preceding the cursor.
With argument, kill that many OZ expressions before the cursor.
Negative arg -N means kill N OZ expressions after the cursor."
  (interactive "p")
  (let ((pos (point)))
    (forward-oz-expr (- arg))
    (kill-region pos (point))))


;;------------------------------------------------------------
;; utilities for error location
;; author: Jochen Doerre

;; The compile.el stuff is used only a little bit; it cannot be made
;; working, if error-msg does not contain file info, as with
;; feed-region. 

;; Functionality: 
;; oz-goto-next-error (C-x ` in .oz, compiler and emulator buffer)
;; Visit next compilation error message and corresponding source code.
;; Applies to most recent compilation, started with one of the feed
;; commands. However, if called in compiler or emulator buffer, it
;; visits the next error message following point (no matter whether
;; that came from the latest compilation or not).
;;------------------------------------------------------------

;; setting keys in compiler/emulator buffer
(defun oz-set-mouse-error-key ()
  (let ((map (current-local-map)))
    (if map
	(if oz-lucid
	    (define-key map [(shift button2)] 'oz-mouse-goto-error)
	  (define-key map [(shift mouse-2)] 'oz-mouse-goto-error)))))


(defun fetch-next-error-data ()
  (let (infoline posx posy lineno file limit error-marker column)
    (if (and (setq posx (re-search-forward oz-error-intro-pattern nil t))
	     (setq posy (search-forward "\tat line " nil t)))
	(progn
	  (goto-char posx)
	  (beginning-of-line)
	  (setq error-marker (point-marker))
	  (forward-line 2)
	  (goto-char posy)
	  (end-of-line)
	  (setq limit (point))
	  (goto-char posy)
	  (if (looking-at "[0-9]+")
	      (setq lineno (car (read-from-string 
				 (buffer-substring
				  posy (match-end 0)))))
	    (error "error format not recognized"))
	  (if (setq posx (search-forward "in file" limit t))
	      (setq file (car (read-from-string 
			       (buffer-substring posx limit)))))
	  (setq posx (re-search-forward "^$" nil t)) ;; matches also a
						     ;; final \n
	  (if (setq posy (search-backward "^-- *** here" limit t))
	      (setq column (- posy
			      (progn (beginning-of-line) (point)) 
			      3)))
	  (goto-char posx)
	  (setq oz-next-error-marker (point-marker))
	  (list error-marker file lineno column))
      (goto-char (point-max))
      (message "no next error")
      (sit-for 1)
      nil)))

(defun fetch-next-callst-data ()
  (let (posx posy lineno file limit error-marker)
    (beginning-of-line)
    (if (setq posy (search-forward "File:" nil t))
	(progn
	  (beginning-of-line)
	  (setq posx (point))
	  (end-of-line)
	  (setq limit (point))
	  (search-backward "\n*** *")
	  (forward-char 1)
	  (setq error-marker (point-marker))
	  (goto-char posy)
	  (setq posy (re-search-forward "[ \t]*" limit t)) ;; skip blanks
	  (setq posx (search-forward "Line:" limit t))
	  (setq file (buffer-substring posy (- posx 6)))
	  (setq lineno (car (read-from-string 
			     (buffer-substring posx limit))))
	  (forward-line 1)
	  (list error-marker file lineno))
      (message "no file/line info found")
      (sit-for 1)
      nil)))

(defun oz-goto-next-error ()
  "Visit next compilation error message and corresponding source
code. Applies to most recent compilation, started with one of the feed
commands. 
When in compiler buffer, visit next error message following point.
When in emulator buffer, visit place indicated in next callstack
line." 
  (interactive)
  (let ((old-buffer (current-buffer))
	(comp-buffer (get-buffer oz-compiler-buffer))
	(emu-buffer (get-buffer oz-emulator-buffer))
	error-data)
    (cond 
     ((eq old-buffer emu-buffer)
      (setq error-data (fetch-next-callst-data))
      (oz-err-moveto-other old-buffer))
     ((eq old-buffer comp-buffer)
      (oz-goto-error-start)
      (setq error-data (fetch-next-error-data))
      (oz-err-moveto-other old-buffer))
     ((bufferp comp-buffer)
      (switch-to-buffer-other-window comp-buffer)
      (cond 
       ((and oz-next-error-marker
	     (eq (marker-buffer oz-next-error-marker) comp-buffer))
	(goto-char oz-next-error-marker))
       ;; else new compilation
       ((and (<= oz-compiler-output-start (point-max))
	     (<= (point-min) oz-compiler-output-start))
	(goto-char oz-compiler-output-start)
	(setq oz-next-error-marker (point-marker)))
       (t (error "No compilation found")))
      (setq error-data (fetch-next-error-data))
      (switch-to-buffer-other-window old-buffer))
     (t (error "no Oz compiler buffer found")))
    (and error-data
	 (let ((errfile (car (cdr error-data)))
	       (line (nth 1 (cdr error-data)))
	       (column (nth 2 (cdr error-data)));; if at all
	       errfile-buffer)
;	   (message (concat "errfile: " errfile))
	   (if (not errfile)
	       (if (not oz-last-fed-region-start)
		   (error "No source buffer found")
		 (set-buffer (marker-buffer oz-last-fed-region-start))
		 (save-excursion
		   (goto-char oz-last-fed-region-start)
		   (if (> line 1) (forward-line (1- line)))
		   (if (and column (> column 0))
		       ;; Columns in error msgs are 1-origin.
		       (if (= line 1)
			   (move-to-column 
			    (+ (current-column) (1- column)))
			 (move-to-column (1- column)))
		     (beginning-of-line))
		   (setcdr error-data (point-marker))))
	     ;; else
	     (set-buffer 
	      (compilation-find-file (car error-data) errfile nil))
	     (save-excursion
	       (save-restriction
		 (widen)
		 (goto-line line)
		 (if (and column (> column 0))
		     ;; Columns in error msgs are 1-origin.
		     (move-to-column (1- column))
		   (beginning-of-line))
		 (setcdr error-data (point-marker)))))
	   (compilation-goto-locus error-data)))))


;; when in compiler buffer in the middle of an error msg, we need to
;; find its first line
(defun oz-goto-error-start ()
  (let ((errstart 
	(save-excursion
	  (beginning-of-line)
	  (if (looking-at "%\\*\\*")
	      (re-search-backward oz-error-intro-pattern nil t)))))
    (if errstart (goto-char errstart))))

;; Move to another window, so that next-error's window changes
;; result in the desired setup.
(defun oz-err-moveto-other (buffer)
  (or (one-window-p)
      (progn
	(other-window -1)
	;; other-window changed the selected buffer,
	;; but we didn't want to do that.
	(set-buffer buffer))))


(defun oz-mouse-goto-error (event)
  (interactive "e")
  (let ((buf (if oz-lucid
		 (event-buffer event) 
	       (window-buffer (posn-window (event-end event))))))
    (or (eq buf (current-buffer))
	;; click not in current buffer -> need other window, so that 
	;; window switching in oz-goto-next-error comes out right
	(switch-to-buffer-other-window buf)))
  (goto-char (if oz-lucid 
		 (event-closest-point event)
	       (posn-point (event-end event))))
  (or (eq (current-buffer) (get-buffer oz-compiler-buffer))
      (eq (current-buffer) (get-buffer oz-emulator-buffer))
      (error "Not in compiler nor in emulator buffer"))
  (oz-goto-next-error))



(provide 'oz)
