;; Major mode for editing Oz, and for running Oz under Emacs
;; Copyright (C) 1993 DFKI GmbH
;; Author: Ralf Scheidhauer and Michael Mehl ([scheidhr|mehl]@dfki.uni-sb.de)

;; TODO
;; - state message: Should we use the mode-line ???

(require 'comint)


;; ---------------------------------------------------------------------
;; global effects
;; ---------------------------------------------------------------------

(setq completion-ignored-extensions
      (append '(".ozo" ".ozs")
	      completion-ignored-extensions))

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

;;------------------------------------------------------------
;; Variables/Initialization
;;------------------------------------------------------------

(defvar oz-gdb-autostart t
  "In gdb mode: start emulator immediately or not.

non-nil: Start emulator immediately.
nil:     Don't start emulator (use command 'run').
         This is useful when you want to use breakpoints.
")

(defvar oz-indent-chars 3
  "*Indentation of Oz statements with respect to containing block.")

(defvar oz-wait-time nil
  "wait for compiler startup (on linux use 15 sec)")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)
(defvar oz-mode-map (make-sparse-keymap))

(defvar oz-emulator (concat (getenv "HOME") "/Oz/Emulator/oz.emulator.bin")
  "The emulator for gdb mode and for [oz-other]")

(defvar oz-emulator-buffer "*Oz Emulator*"
  "The buffername of the Oz Emulator output")

(defvar oz-compiler-buffer "*Oz Compiler*"
  "The buffername of the Oz Compiler output")

(defvar oz-emulator-hook nil
  "Hook used if non nil for starting the Oz Emulator.
For example
  (setq oz-emulator-hook 'oz-start-gdb-emulator)
starts the emulator under gdb")

(defvar OZ-HOME "/usr/share/gs/oz"
  "The directory where oz is installed")

(defun oz-home ()
  (let ((ret (getenv "OZHOME")))
    (if ret
	ret
      (message "OZHOME not set using fallback: %s" OZ-HOME)
      OZ-HOME)))

(defvar oz-doc-dir (concat (oz-home) "/doc/")
  "The default doc directory")

(defvar oz-preview "xdvi"
  "The previewer for doc files")


(defvar oz-error-string (format "%c" 17)
  "how compiler and engine signal errors")

(defconst oz-remove-pattern
  (concat oz-error-string "\\|" (format "%c" 18) "\\|" (format "%c" 19))
  "")

(defvar oz-want-font-lock t
  "*If t means that font-lock mode is switched on")

(defvar oz-temp-counter 0
  "gensym counter")

(defvar oz-popup-on-error t
  "*popup compiler/emulator buffer if error occured")

;;------------------------------------------------------------
;; Screen title
;;------------------------------------------------------------

;; lucid supports screen-title as format string (is better ...)
;;  see function mode-line-format
;; gnu19 supports frame-title as constant string

(defvar oz-old-screen-title
  (if oz-lucid
      (setq oz-old-screen-title
	    screen-title-format)
    (if oz-gnu19
	(setq oz-old-screen-title
	      (cdr (assoc 'name (frame-parameters))))))
  "The saved window title")


(defun oz-get-title()
  (if oz-gnu19
      (cdr (assoc 'name (frame-parameters (car (visible-frame-list)))))
      (if oz-lucid
	screen-title-format
	"")))

(defvar oz-title-format
  (concat "Oz Programming Interface ("
	  (oz-get-title) ")")
  "The format string for the window title")


(defun oz-set-title ()
  (if oz-gnu19
      (mapcar '(lambda(scr)
		 (modify-frame-parameters 
		  scr
		  (list (cons 'name oz-title-format))))
	      (visible-screen-list)))

  (if oz-lucid
      (setq screen-title-format oz-title-format)))



(defun oz-reset-title ()
  "reset to the initial window title"
  (if oz-lucid
   (setq screen-title-format oz-old-screen-title))
  (if oz-gnu19
   (mapcar '(lambda(scr)
	      (modify-frame-parameters 
	       scr
	       (list (cons 'name oz-old-screen-title))))
	   (visible-screen-list))))

(defun oz-window-system()
  "Non-nil iff we are running under X"
  window-system)


;;------------------------------------------------------------
;; Utilities
;;------------------------------------------------------------

(defun oz-make-temp-name(name)
  "gensym implementation"
  (setq oz-temp-counter (+ 1 oz-temp-counter))
  (format "%s%d" (make-temp-name name) oz-temp-counter))


(defun oz-line-pos()
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


(defun oz-set-fontlock-keywords()
  (setq font-lock-keywords (list oz-keywords)))

;;------------------------------------------------------------
;; Menus
;;------------------------------------------------------------
;; lucid: a menubar is a new datastructure (see function set-buffer-menubar)
;; GNU19: a menubar is a usial keysequence with prefix "menu-bar"

(defvar oz-menubar nil
  "The Oz Menubar for Lucid Emacs")

(defun oz-make-menu(list)
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


(defun oz-make-menu-gnu19 (map list)
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
	    (oz-make-menu-gnu19 newmap (reverse rest))))))
    (oz-make-menu-gnu19 map (cdr list))))

(oz-make-menu
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
     ("Region"                  . oz-print-region)
     ("Buffer (Portrait)"	. oz-print-buffer)
     ("Buffer (Landscape)"	. oz-print-buffer-landscape)
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
    )))

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
	   (get-buffer-process oz-emulator-buffer))
      (if (null force)
	  (let ((i (* 2 oz-halt-timeout))
		(eproc (get-buffer-process oz-emulator-buffer))
		(cproc (get-buffer-process oz-compiler-buffer)))
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


(defun oz-check-running(start-flag)
  (if (and (get-buffer-process oz-compiler-buffer)
	   (not (get-buffer-process oz-emulator-buffer)))
      (progn
	(message "Emulator died for some reason")
	(delete-process oz-compiler-buffer)))
  (if (and (not (get-buffer-process oz-compiler-buffer))
	   (get-buffer-process oz-emulator-buffer))
      (progn
	(message "Compiler died for some reason")
	(delete-process oz-emulator-buffer)))
  (if (get-buffer-process oz-compiler-buffer)
      t
    (let ((file (oz-make-temp-name "/tmp/ozsock")))
      (if (not start-flag) (message "Oz died for some reason. Restarting ..."))
      (make-comint "Oz Compiler" "oz.compiler" nil "-emacs" "-S" file)
      (setq oz-compiler-buffer "*Oz Compiler*")
      (oz-create-buffer oz-compiler-buffer t)
      (set-process-filter (get-buffer-process oz-compiler-buffer)
			  'oz-compiler-filter)
      (bury-buffer oz-compiler-buffer)

      (if oz-emulator-hook
	  (funcall oz-emulator-hook file)
	(setq oz-emulator-buffer "*Oz Emulator*")
	(if oz-wait-time (sleep-for oz-wait-time))
	(make-comint "Oz Emulator" "oz.emulator" nil "-emacs" "-S" file)
	(oz-create-buffer oz-emulator-buffer nil)
	(set-process-filter (get-buffer-process oz-emulator-buffer)
			    'oz-emulator-filter)
	)

      (bury-buffer oz-emulator-buffer)

      (oz-set-title)
      (message "Oz started")
      ;(sleep-for 10)
      )))




;;------------------------------------------------------------
;; GDB support
;;------------------------------------------------------------

(defun oz-set-emulator()
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

(defun oz-gdb()
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


(defun oz-other()
  (interactive)
  (if (getenv "OZEMULATOR")
      (setenv "OZEMULATOR" nil)
    (setenv "OZEMULATOR" oz-emulator))

  (if (getenv "OZEMULATOR")
      (message "Oz Emulator: %s" oz-emulator)
    (message "Oz Emulator: global")))


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
    (process-send-string
     (get-buffer-process oz-emulator-buffer)
     init-str)
    (if oz-gdb-autostart
	(process-send-string
	 (get-buffer-process oz-emulator-buffer)
	 "run\n"))
    (switch-to-buffer old-buffer)))

;;------------------------------------------------------------
;; Feeding the compiler
;;------------------------------------------------------------

(defun oz-zmacs-stuff()
  (if oz-lucid (setq zmacs-region-stays t)))

(defun oz-feed-buffer ()
  "Feeds the entire buffer."
  (interactive)
  (let ((file (buffer-file-name))
	(cur (current-buffer)))
    (if (or (not file) (buffer-modified-p))
	(oz-feed-region (point-min) (point-max))
      (oz-feed-file file))
    (switch-to-buffer cur))
  (oz-zmacs-stuff))


(defun oz-feed-region (start end)
  "Feeds the region."
   (interactive "r")   
   (oz-send-string (buffer-substring start end))
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
	(forward-paragraph 1)
	(let ((end (point)))
	  (backward-paragraph 1)
	  (oz-feed-region (point) end)))
  (oz-zmacs-stuff))


(defun oz-send-string(string)
  (oz-check-running nil)
  (let ((proc (get-buffer-process oz-compiler-buffer)))
;  (comint-send-string  proc string)
;  (comint-send-string proc "\n")
    (process-send-string proc string)
    (process-send-string proc "\n")
    (process-send-eof proc)))


;;------------------------------------------------------------
;; Feeding the emulator
;;------------------------------------------------------------

(defun oz-continue()
  "continue the Oz Emulator after an error"
  (interactive)
  (comint-send-string (get-buffer-process oz-emulator-buffer) "c\n"))


;;------------------------------------------------------------
;; electric
;;------------------------------------------------------------

(defun oz-electric-terminate-line ()
  "Terminate line and indent next line."
  (interactive)
  (oz-indent-line)
  (delete-horizontal-space) ; Removes trailing whitespaces
  (newline)
  (oz-indent-line)
)

;;------------------------------------------------------------
;;Indent
;;------------------------------------------------------------

(defun oz-make-keywords-for-match(args)
  (concat "\\<\\("
	  (mapconcat 'identity args "\\|")
	  "\\)\\>"))

(defconst oz-keywords
   (concat
    (oz-make-keywords-for-match
     '(
       "sproc" "proc" "fun"
       "local" "declare"
       "if" "or" "OR" "case" "then" "else" "elseif" "of" "elseof" "elsecase"
       "end" "fi" "ro" "RO"
       "class" "create" "meth" "extern" "from" "with" "attr" "feat" "self"
       "true" "false"
       "div" "mod" "andthen" "orelse"
       "not" "thread" "task" "in"
       "dis"
       ))
    "\\|\\.\\|\\[\\]\\|#\\|!\\|:\\|\\@"
    ))


(defconst oz-declare-pattern (oz-make-keywords-for-match '("declare")))

(defconst oz-begin-pattern
      (oz-make-keywords-for-match 
	         '(
		   "sproc" "proc" "fun"
		   "local"
		   "if" "or" "OR" "case"
		   "class" "create" "meth" "extern"
		   "not" "thread" "task"
		   "dis"
		   )))

(defconst oz-left-pattern "[[({]")
(defconst oz-right-pattern "[])}]")

(defconst oz-end-pattern
      (oz-make-keywords-for-match '("end" "fi" "ro" "RO")))


(defconst oz-between-pattern 
      (concat (oz-make-keywords-for-match
	       '("from" "attr" "feat" "with"))))

(defconst oz-middle-pattern 
  (concat (oz-make-keywords-for-match
	   '("in" "then" "else" "elseif" "of" "elseof" "elsecase"))
	  "\\|" "\\[\\]"))

(defconst oz-feat-end-pattern
  ":[ \t]*")

(defconst oz-key-pattern
      (concat oz-declare-pattern "\\|" oz-begin-pattern "\\|"
	      oz-between-pattern "\\|"
	      oz-left-pattern "\\|" oz-right-pattern "\\|"
	      oz-middle-pattern "\\|" oz-end-pattern
	      ))

(defun oz-indent-buffer()
  (interactive)
  (goto-char 0)
  (while (< (point) (point-max))
    (oz-indent-line t)
    (forward-line 1)))


(defun oz-indent-region(b e)
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
(defun oz-calc-indent(no-empty-line)
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

(defun oz-is-right()
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
		   (message "unbalanced open paren")
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

(defun oz-search-matching-paren()
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
;; emacs does not support two comment formats !!!
;    (modify-syntax-entry ?/ ". 14" table)
;    (modify-syntax-entry ?* ". 23" table)
    (setq oz-mode-syntax-table table)
    (set-syntax-table oz-mode-syntax-table)))

(define-abbrev-table 'oz-mode-abbrev-table ())

(defun oz-mode-variables ()
  (set-syntax-table oz-mode-syntax-table)
  (setq local-abbrev-table oz-mode-abbrev-table)
  (make-local-variable 'paragraph-start)
  (setq paragraph-start (concat "^%%\\|^$\\|" page-delimiter)) ;'%%..'
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'paragraph-ignore-fill-prefix)
  (setq paragraph-ignore-fill-prefix t)
  (make-local-variable 'fill-paragraph-function)
  (setq fill-paragraph-function 'oz-fill-paragraph)
  (make-local-variable 'auto-fill-function)
  (setq auto-fill-function 'oz-auto-fill)
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'oz-indent-line)
  (make-local-variable 'comment-start)
  (setq comment-start "%")
  (make-local-variable 'comment-end)
  (setq comment-end "")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "/\\*+ *\\|% *")
)

(defun oz-mode-commands (map)
  (define-key map "\t"       'oz-indent-line)
  (define-key map "\M-\C-m"  'oz-feed-buffer)
  (define-key map "\M-r"     'oz-feed-region)
  (define-key map "\M-l"     'oz-feed-line)
  (define-key map "\C-c\C-p" 'oz-feed-paragraph)
  (define-key map "\C-cb" 'oz-feed-line-browse)
  (define-key map "\C-c\C-b"    'oz-feed-region-browse)
  (define-key map "\M-n"   'oz-next-buffer)
  (define-key map "\M-p"   'oz-previous-buffer)
  (define-key map "\C-c\C-c"    'oz-toggle-compiler)
  (if oz-lucid
      (progn
	;; otherwise this looks in the menubar like "C-TAB" "C-BS" "C_RET"
	(define-key map [(control c) (control i)] 'oz-feed-file)
	(define-key map [(control c) (control h)] 'oz-halt)
	(define-key map [(control c) (control m)]   'oz-toggle-emulator))
    (define-key map "\C-c\C-h"    'oz-halt)
    (define-key map "\C-c\C-i"    'oz-feed-file)
    (define-key map "\C-c\C-m"    'oz-toggle-emulator))

  (define-key map "\C-c\C-e"    'oz-toggle-emulator)
  (define-key map "\C-c\C-f"    'oz-feed-file)
  (define-key map "\C-c\C-n"    'oz-new-buffer)
  (define-key map "\C-c\C-l"    'oz-fontify)
  (define-key map "\C-c\C-r"    'run-oz)
  (define-key map "\C-cc"       'oz-precompile-file)
  (define-key map "\C-cm"       'oz-set-emulator)
  (define-key map "\C-co"       'oz-other)
  (define-key map "\C-cd"       'oz-gdb)
  (define-key map "\r"		'oz-electric-terminate-line)
  (define-key map "\C-cp"       'oz-view-panel)
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
  (if oz-lucid
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
(defun oz-auto-fill ()
;  (let ((start (oz-comment-start)))
  (message "Oz auto fill: not implemented"))

;;------------------------------------------------------------
;; Fontification
;;------------------------------------------------------------

(if (oz-window-system) (require 'font-lock))


(defun oz-fontify-buffer()
  (interactive)
  (if (oz-window-system) (font-lock-fontify-buffer)))


(defun oz-fontify-region(beg end)
  (if (oz-window-system) (font-lock-fontify-region beg end)))


(defun oz-fontify(&optional arg)
  (interactive "P")
  (recenter arg)
  (oz-fontify-buffer))


;;------------------------------------------------------------
;; Filtering process output
;;------------------------------------------------------------


(defun oz-emulator-filter (proc string)
  (oz-filter proc string))


(defun oz-compiler-filter (proc string)
  (oz-filter proc string))

(defun oz-filter (proc string)
;; see elisp manual: "Filter Functions"
  (let ((old-buffer (current-buffer))
	(newbuf (process-buffer proc))
	(errs-found (and oz-popup-on-error (string-match oz-error-string string))))
    (unwind-protect
	(let (moving ol-point index)
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
	(bury-buffer buffer)))))


(defun oz-create-buffer (buf ozmode)
  (save-excursion
    (set-buffer (get-buffer-create buf))

;; enter oz-mode but no highlighting !
    (if ozmode
	(progn
	  (kill-all-local-variables)
	  (use-local-map oz-mode-map)
	  (setq mode-name "Oz-Output")
	  (setq major-mode 'oz-mode)))
    (if oz-lucid
     (set-buffer-menubar (append current-menubar oz-menubar)))
    (delete-region (point-min) (point-max))))


(defun oz-toggle-compiler()
  (interactive)
  (oz-toggle-window oz-compiler-buffer))


(defun oz-toggle-emulator()
  (interactive)
  (oz-toggle-window oz-emulator-buffer))



(defun oz-toggle-window(buffername)
  (if (get-buffer buffername)
      (if (get-buffer-window buffername nil)
	  (if oz-gnu19
	      (delete-windows-on buffername t)
	    (delete-windows-on buffername))
	(oz-show-buffer (get-buffer buffername)))))


(defun oz-new-buffer()
  (interactive)
  (switch-to-buffer (generate-new-buffer "Oz"))
  (oz-mode))


(defun oz-previous-buffer()
  (interactive)
  (bury-buffer)
  (oz-walk-trough-buffers (buffer-list)))


(defun oz-next-buffer()
  (interactive)
  (oz-walk-trough-buffers (reverse (buffer-list))))


(defun oz-walk-trough-buffers(bufs)
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


(defvar oz-lpr "oz2lpr -"
  "pretty printer for oz code")

(defvar oz-lpr-landscape "oz2lpr -landscape -"
  "pretty printer in landscape for oz code")

(defun oz-print-buffer()
  "Print buffer."
  (interactive)
  (oz-print-region (point-min) (point-max)))

(defun oz-print-buffer-landscape()
  "Print buffer."
  (interactive)
  (oz-print-region-landscape (point-min) (point-max)))

(defun oz-print-region(start end)
  "Print region."
  (interactive "r")
  (shell-command-on-region start end oz-lpr))

(defun oz-print-region-landscape(start end)
  "Print region."
  (interactive "r")
  (shell-command-on-region start end oz-lpr-landscape))


(defvar oz-temp-file (oz-make-temp-name "/tmp/ozemacs") "")


(defun oz-to-coresyntax-buffer()
  (interactive)
  (oz-to-coresyntax-region (point-min) (point-max)))

(defun oz-to-coresyntax-line()
  (interactive)
  (let ((line (oz-line-pos)))
    (oz-to-coresyntax-region (car line) (cdr line))))

(defun oz-to-coresyntax-region (start end)
   (interactive "r")
   (oz-directive-on-region start end "\\core" ".ozc" t))


(defun oz-to-emulatorcode-buffer()
  (interactive)
  (oz-to-emulatorcode-region (point-min) (point-max)))

(defun oz-to-emulatorcode-line()
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
    (oz-send-string (concat "{Browse " contents "}"))))


(defun oz-feed-line-browse()
  "Feed the current line into the Oz Compiler"
  (interactive)
   (let ((line (oz-line-pos)))
     (oz-feed-region-browse (car line) (cdr line))))


(defun oz-view-panel ()
  "Feed {System.panel popup} into the Oz Compiler"
  (interactive)
  (oz-send-string "{System.panel popup}"))

(defun oz-feed-file(file)
  "Feed an file into the Oz Compiler"
  (interactive "FFeed file: ")
  (oz-send-string (concat "\\feed '" file "'"))) 

(defun oz-precompile-file(file)
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

(defun oz-find-docu-file()
  "find a text in the doc directory"
  (interactive)
  (oz-find-file "Find doc file: " "doc/"))

(defun oz-find-demo-file()
  "find a Oz file in the demo directory"
  (interactive)
  (oz-find-file "Find demo file: " "demo/"))

(defun oz-find-docdemo-file()
  "find a Oz file in the demo/documentation directory"
  (interactive)
  (oz-find-file "Find demo file: " "demo/documentation/"))

(defun oz-find-modules-file()
  "find a Oz file in the lib directory"
  (interactive)
  (oz-find-file "Find modules file: " "lib/"))

(defun oz-find-file(prompt file)
  (find-file (read-file-name prompt
			     (concat (oz-home) "/" file)
			     nil
			     t
			     nil)))


(provide 'oz)

