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
      (append '(".load" ".sym")
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

(defvar oz-indent-chars 3
  "*Indentation of Oz statements with respect to containing block.")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)
(defvar oz-mode-map (make-sparse-keymap))

(defvar oz-machine (concat (getenv "HOME") "/Oz/AM/oz.machine.bin")
  "The machine for gdb mode and for [oz-other]")

(defvar oz-machine-buffer "*Oz Machine*"
  "The buffername of the Oz Machine output")

(defvar oz-machine-hook nil
  "Hook used if non nil for starting the Oz Machine.
For example
  (setq oz-machine-hook 'oz-start-gdb-machine)
starts the machine under gdb")

(defvar oz-wait-for-compiler 5
  "Wait between startup of compiler and engine")

(defvar oz-home (concat (or (getenv "OZHOME") "/usr/share/gs/soft/oz") "/")
  "The directory where oz is installed")

(defvar oz-doc-dir (concat oz-home "doc/")
  "The default doc directory")

(defvar oz-preview "xdvi"
  "The previewer for doc files")


(defvar oz-error-string (format "%c" 17)
  "how compiler and engine signal errors")

(defvar oz-warn-string (format "%c" 18)
  "how compiler and engine signal warnings")

(defvar oz-status-string (format "%c" 19)
  "How compiler and engine signal status changes")

(defvar oz-see-compiler-input t
  "*If non-nil means every input to the compiler is echoed in the 
Compiler buffer")

(defvar oz-temp-counter 0
  "gensym counter")

(defvar oz-compiler-state "???")
(defvar oz-machine-state  "???")

(defvar oz-title-format nil
  "The format string for the window title" )
(if oz-gnu19
 (setq oz-title-format "Oz Console          C: %s  M: %s")
 )
(if oz-lucid
 (setq oz-title-format
       '(("Oz Console: %b   C: "    oz-compiler-state)
	 ("   M: " oz-machine-state))))
;       '(("Oz Console: %b   C:  "   (-30 . oz-compiler-state))
;	 ("   M:  " (-30 . oz-machine-state)))))


(defvar oz-old-screen-title
  (if oz-lucid
      (setq oz-old-screen-title
	    screen-title-format)
    (if oz-gnu19
	(setq oz-old-screen-title
	      (cdr (assoc 'name (frame-parameters))))))
  "The saved window title")


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
;; Screen title
;;------------------------------------------------------------

;; lucid supports screen-title as format string (is better ...)
;;  see function mode-line-format
;; gnu19 supports frame-title as constant string

(defun oz-canon-status-string(s)
  (cond ((string-match "\\<idle\\>" s) s)
	((string-match "\\<gdb\\>" s) "gdb mode")
	((string-match "\\<running\\>" s) "running")
	((string-match "\\<halted\\>" s) "halted")
	((string-match "\\<booting\\>" s) "booting")
	( t "???")))

(defun oz-set-state(state string)
  "change compiler or machine state and adjust the window titles"
  (if (string= string "")
      t
    (setq string (oz-canon-status-string string))
    (set state 
	 (format "%s" 
		 (substring string 0 
			    (min 30 (length string)))))
    (if oz-gnu19
     (let ((name (format oz-title-format 
			 oz-compiler-state 
			 oz-machine-state)))
       (mapcar '(lambda(scr)
		  (modify-frame-parameters 
		   scr
		   (list (cons 'name name))))
	       (visible-screen-list))))))


(defun oz-reset-title()
  "reset to the initial window title"
  (if oz-lucid
   (setq screen-title-format oz-old-screen-title))
  (if oz-gnu19
   (mapcar '(lambda(scr)
	      (modify-frame-parameters 
	       scr
	       (list (cons 'name oz-old-screen-title))))
	   (visible-screen-list))))

;;------------------------------------------------------------
;; Fonts
;;------------------------------------------------------------

(defvar oz-small-font      '("-adobe-courier-" . "-*-*-*-100-*-*-*-*-*-*"))
(defvar oz-default-font    '("-adobe-courier-" . "-*-*-*-120-*-*-*-*-*-*"))
(defvar oz-large-font      '("-adobe-courier-" . "-*-*-*-140-*-*-*-*-*-*"))
(defvar oz-very-large-font '("-adobe-courier-" . "-*-*-*-180-*-*-*-*-*-*"))

(make-face 'oz-bold)
(make-face 'oz-italic)

(defun oz-small-font()
  (interactive)
  (oz-set-font oz-small-font))

(defun oz-default-font()
  (interactive)
  (oz-set-font oz-default-font))

(defun oz-large-font()
  (interactive)
  (oz-set-font oz-large-font))

(defun oz-very-large-font()
  (interactive)
  (oz-set-font oz-very-large-font))

(defun oz-set-font(font)
  (let ((scr (selected-screen)))
    (if	oz-gnu19
	(progn
	  (modify-frame-parameters
	   scr
	   (list (cons 'font  (concat (car font) "medium-r" (cdr font)))))
          (set-face-font 'oz-bold nil scr)
	  (set-face-font 'oz-italic nil scr)
	  (set-face-font 'oz-bold (concat (car font) "bold-r" (cdr font)) scr)
	  (set-face-font 'oz-italic (concat (car font) "medium-o" (cdr font)) scr)))
    (if oz-lucid
	(progn
	  (set-face-font 'default (concat (car font) "medium-r" (cdr font)) scr)
	  (set-face-font 'oz-bold (concat (car font) "bold-r" (cdr font)) scr)
	  (set-face-font 'oz-italic (concat (car font) "medium-o" (cdr font)) scr)
	  ))))

(oz-default-font)

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
    ("Feed buffer"            . oz-feed-buffer)
    ("Feed region"            . oz-feed-region)
    ("Feed line"              . oz-feed-line)
    ("Feed file"              . oz-feed-file)
    ("Compile file"           . oz-precompile-file)
    ("-----")
    ("Find"
     ("Demo file"              . oz-find-demo-file)
     ("Library file"           . oz-find-lib-file)
;     ("Documentation (Text)"          . oz-find-docu-file)
;     ("Documentation (DVI)"           . oz-find-dvi-file)
     )
    ("Print"
     ("region"      . oz-print-region)
     ("buffer (portrait)"	. oz-print-buffer)
     ("buffer (landscape)"	. oz-print-buffer-landscape)
     )
    ("Core Syntax"
     ("buffer"      . oz-to-coresyntax-buffer)
     ("region"      . oz-to-coresyntax-region)
     ("line"        . oz-to-coresyntax-line  )
     )
    ("Machine Code"
     ("buffer"      . oz-to-machinecode-buffer)
     ("region"      . oz-to-machinecode-region)
     ("line"        . oz-to-machinecode-line  )
     )
    ("Indent"
     ("line"   . oz-indent-line)
     ("region" . oz-indent-region)
     ("buffer" . oz-indent-buffer)
     )
    ("Browse"
     ("browse" . oz-feed-region-browse)
     ("memory" . oz-feed-region-browse-memory)
     )
;    ("Panel"   . oz-feed-panel)
    ("-----")
    ("Next Oz buffer"         . oz-next-buffer)
    ("Previous Oz buffer"     . oz-previous-buffer)
    ("New Oz buffer"          . oz-new-buffer)
    ("Fontify buffer"         . oz-fontify)
    ("Show/hide"
     ("errors"       . oz-toggle-errors)
     ("compiler"     . oz-toggle-compiler)
     ("machine"      . oz-toggle-machine)
     )
    ("-----")
    ("Start Oz" . run-oz)
    ("Halt Oz"  . oz-halt)
    )
   ("Font"
    ("Small"      . oz-small-font     )
    ("Default"    . oz-default-font    )
    ("Large"      . oz-large-font     )
    ("Very Large" . oz-very-large-font)
    )
   ))


;;------------------------------------------------------------
;; oz-mode
;;------------------------------------------------------------

(if oz-mode-syntax-table
    ()
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?_ "w" table)
    (modify-syntax-entry ?\\ "." table)
    (modify-syntax-entry ?+ "." table)
    (modify-syntax-entry ?- "." table)
    (modify-syntax-entry ?= "." table)
    (modify-syntax-entry ?< "." table)
    (modify-syntax-entry ?> "." table)
;    (modify-syntax-entry ?\' "\"" table)
    (modify-syntax-entry ?\n ">   " table)
    (modify-syntax-entry ?\f ">   " table)
    (modify-syntax-entry ?\% "<   " table)
    (modify-syntax-entry ?/ ". 14" table)
    (modify-syntax-entry ?* ". 23" table)
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
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'oz-indent-line)
  (make-local-variable 'comment-start)
  (setq comment-start "%")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "%+ *")
;  (make-local-variable 'after-change-function)
;  (setq after-change-function 'oz-after-change-function)
;  (make-local-variable 'comment-column)
;  (setq comment-column 48)
;  (make-local-variable 'comment-indent-hook)
;  (setq comment-indent-hook 'oz-comment-indent)
)

(defun oz-mode-commands (map)
  (define-key map "\t"      'oz-indent-line)
  (define-key map "\M-\C-m" 'oz-feed-buffer)
  (define-key map "\M-r"    'oz-feed-region)
  (define-key map "\M-l"    'oz-feed-line)
  (define-key map "\M-n"   'oz-next-buffer)
  (define-key map "\M-p"   'oz-previous-buffer)
  (define-key map "\C-c\C-e"    'oz-toggle-errors)
  (define-key map "\C-c\C-c"    'oz-toggle-compiler)
  (if oz-lucid
      (progn
	(define-key map [(control button1)]       'oz-feed-region-browse)
	(define-key map [(control button3)]       'oz-feed-region-browse-memory)))
  (if oz-gnu19
      (progn
	(define-key map [C-down-mouse-1]        'oz-feed-region-browse)
	(define-key map [C-down-mouse-3]        'oz-feed-region-browse-memory)))
  
  (if oz-lucid
      (progn
	;; otherwise this looks in the menubar like "C-TAB" "C-BS" "C_RET"
	(define-key map [(control c) (control i)] 'oz-feed-file)
	(define-key map [(control c) (control h)] 'oz-halt)
	(define-key map [(control c) (control m)]   'oz-toggle-machine))
    (define-key map "\C-c\C-h"    'oz-halt)
    (define-key map "\C-c\C-i"    'oz-feed-file)
    (define-key map "\C-c\C-m"    'oz-toggle-machine))

  (define-key map "\C-c\C-f"    'oz-feed-file)
  (define-key map "\C-c\C-n"    'oz-new-buffer)
  (define-key map "\C-c\C-l"    'oz-fontify)
  (define-key map "\C-c\C-r"    'run-oz)
  (define-key map "\C-cc"       'oz-precompile-file)
  (define-key map "\C-cm"       'oz-set-machine)
  (define-key map "\C-co"       'oz-other)
  (define-key map "\C-cd"       'oz-gdb)

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
   (set-buffer-menubar (append current-menubar oz-menubar)))

  ; font lock stuff
  (make-local-variable 'comment-start)
  (make-local-variable 'comment-end)
  (setq comment-start "%")
  (setq comment-start "/* ")
; (setq comment-end "")
  (setq comment-end " */")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "/\\*+ *\\|% *")

  (setq font-lock-keywords (list oz-keywords))

  (font-lock-mode t)
  (font-lock-fontify-buffer)
  (run-hooks 'oz-mode-hook))

;;------------------------------------------------------------
;; Start/Stop oz
;;------------------------------------------------------------

(defvar oz-machine-visible nil 
  "???")
(defvar oz-errors-found nil
  "??? show *Oz Errors* if necessary")


(defun run-oz ()
  "Run the Oz Compiler and Oz Machine.
Input and output via buffers *Oz Compiler* and *Oz Machine*."
  (interactive)
  (oz-check-running)
  (if (or (get-process "Oz Compiler") (get-buffer-process oz-machine-buffer))
      (error "Oz already running, try halting Oz"))
  (start-oz-process)
  (if (not (equal mode-name "Oz"))
      (oz-new-buffer)))

(defvar oz-halt-timeout 15
  "How long to wait in oz-halt after sending the directive halt")

(defun oz-halt()
  (interactive)

  (message "halting Oz...")
  (if (and (get-process "Oz Compiler")
	   (get-buffer-process oz-machine-buffer))
      (let ((i oz-halt-timeout))
	(oz-send-string "!halt \n")
	(while (and (or (get-process "Oz Compiler")
			(get-buffer-process oz-machine-buffer))
		    (> i 0))
	  (sit-for 1)
	  (sleep-for 1)
	  (setq i (1- i)))))

  (if (get-process "Oz Compiler")
      (delete-process "*Oz Compiler*"))
  (if (get-buffer-process oz-machine-buffer)
      (delete-process oz-machine-buffer))
  (message "")
  (oz-reset-title))



(defun oz-check-running()
  (if (and (get-process "Oz Compiler")
	   (not (get-buffer-process oz-machine-buffer)))
      (progn 
	(oz-set-state 'oz-machine-state "???")
	(error "Machine has died, for some unknown reason, try halting Oz")))
  (if (and (not (get-process "Oz Compiler"))
	   (get-buffer-process oz-machine-buffer))
      (progn 
	(oz-set-state 'oz-compiler-state "???")
	(error "Compiler has died, for some unknown reason, try halting Oz"))))

(defun start-oz-process()
  (let ((file (oz-make-temp-name "/tmp/ozsock")))
    (setq oz-errors-found nil)
    (setq oz-machine-visible (get-buffer-window oz-machine-buffer))

    (oz-set-state 'oz-compiler-state "booting")
    (make-comint "Oz Compiler" "oz.compiler" nil "-emacs" "-S" file)
    (oz-create-buffer "*Oz Compiler*")
    (set-process-filter (get-process "Oz Compiler") 'oz-compiler-filter)
    (bury-buffer "*Oz Compiler*")
    (if oz-wait-for-compiler (sleep-for oz-wait-for-compiler))

    (if oz-machine-hook
	(funcall oz-machine-hook file)
      (oz-set-state 'oz-machine-state "booting")
      (setq oz-machine-buffer "*Oz Machine*")
      (make-comint "Oz Machine" "oz.machine" nil "-emacs" "-S" file)
      (set-process-filter (get-buffer-process oz-machine-buffer)
			  'oz-machine-filter)
      (oz-create-buffer oz-machine-buffer)
;;      (save-excursion
;;	(set-buffer oz-machine-buffer)
;;	(define-key (current-local-map) "\C-m" 'comint-send-input))
   )

    (bury-buffer oz-machine-buffer)

    ;; make sure buffers exist
    (oz-create-buffer "*Oz Errors*")

    (if oz-lucid
	(setq screen-title-format oz-title-format))))



;;------------------------------------------------------------
;; GDB support
;;------------------------------------------------------------

(defun oz-set-machine()
  (interactive)
  (setq oz-machine 
	(expand-file-name 
	 (read-file-name "Choose Machine: "
			 nil
			 nil
			 t
			 nil))))

(defun oz-gdb()
  (interactive)
  (if (getenv "OZ_PI")
      t
    (setenv "OZ_PI" "1")
    (setenv "OZPLATFORM" "sunos-sparc")
    (setenv "OZHOME" (or (getenv "OZHOME") "/usr/share/gs/soft/oz"))
    (setenv "OZPATH" 
	    (concat (or (getenv "OZPATH") ".") ":"
		    (getenv "OZHOME") "/lib:"
		    (getenv "OZHOME") "/platform/sunos-sparc:"
		    (getenv "OZHOME") "/demo"))
    (setenv "PATH"
	    (concat (getenv "PATH") ":" (getenv "OZHOME") "/bin")))


  (if oz-machine-hook
      (setq oz-machine-hook nil)
    (setq oz-machine-hook 'oz-start-gdb-machine)
    )

  (if oz-machine-hook
      (message "gdb enabled: %s" oz-machine)
    (message "gdb disabled")))


(defun oz-other()
  (interactive)
  (if (getenv "OZMACHINE")
      (setenv "OZMACHINE" nil)
    (setenv "OZMACHINE" oz-machine))

  (if (getenv "OZMACHINE")
      (message "Oz Machine: %s" oz-machine)
    (message "Oz Machine: global")))


(defun oz-start-gdb-machine (tmpfile)
  "Run gdb on oz-machine
The directory containing FILE becomes the initial working directory
and source-file directory for GDB.  If you wish to change this, use
the GDB commands `cd DIR' and `directory'."
  (let ((old-buffer (current-buffer)))
    (oz-set-state 'oz-machine-state "gdb")
    (if oz-gnu19 (gdb (concat "gdb " oz-machine)))
    (if oz-lucid (gdb oz-machine))
    (setq oz-machine-buffer (buffer-name (current-buffer)))
    (comint-send-string (get-buffer-process oz-machine-buffer)
			(concat "run -S " tmpfile "\n"))
    (switch-to-buffer old-buffer)))

;;------------------------------------------------------------
;; Feeding the compiler
;;------------------------------------------------------------

(defun oz-feed-buffer ()
  "Feeds the entire buffer."
  (interactive)
  (let ((file (buffer-file-name))
	(cur (current-buffer)))
    (if (or (not file) (buffer-modified-p))
	(oz-feed-region (point-min) (point-max))
      (oz-feed-file file))
    (switch-to-buffer cur))
  (if oz-lucid (setq zmacs-region-stays t)))



(defun oz-feed-region (start end)
  "Consults the region."
   (interactive "r")   
   (oz-hide-errors)
   (let ((contents (buffer-substring start end)))
     (oz-send-string (concat contents "\n")))
   (if oz-lucid (setq zmacs-region-stays t)))
     

(defun oz-feed-line ()
  "Consults one line."
   (interactive)
   (let* ((line (oz-line-pos)))
     (oz-feed-region (car line) (cdr line)))
   (if oz-lucid (setq zmacs-region-stays t)))

(defun oz-send-string(string)
  (oz-check-running)
  (or (get-process "Oz Compiler") (start-oz-process))
  (comint-send-string "Oz Compiler" string)
  (if oz-see-compiler-input
      (oz-compiler-filter (get-process "Oz Compiler") string))
  (process-send-eof "Oz Compiler"))


;;------------------------------------------------------------
;; Feeding the machine
;;------------------------------------------------------------

(defun oz-continue()
  "continue the Oz Machine after an error"
  (interactive)
  (comint-send-string (get-buffer-process oz-machine-buffer) "c\n"))

;;------------------------------------------------------------
;;Indent
;;------------------------------------------------------------

(defun oz-make-keywords-for-match(args)
  (concat "\\<\\("
	  (mapconcat 'identity args "\\|")
	  "\\)\\>"))

(defconst oz-begin-pattern
      (oz-make-keywords-for-match 
	         '(
		   "pred" "proc" "fun"
		   "local" "declare" 
		   "if" "or" "case"
		   "class" "create" "meth" "extern"
		   "not" "process"
		   )))

(defconst oz-end-pattern
      (oz-make-keywords-for-match '("end" "fi" "ro")))

(defconst oz-middle-pattern 
      (concat (oz-make-keywords-for-match
	       '("in" "then" "else" "elseif" "of" "elseof"))
	      "\\|" "\\[\\]"))

(defconst oz-key-pattern
      (concat oz-begin-pattern "\\|" oz-middle-pattern "\\|" oz-end-pattern))

(defconst oz-left-pattern "[[({]")
(defconst oz-right-pattern "[])}]")

(defvar oz-abstr-pattern "\\<proc\\>\\|\\<pred\\>\\|\\<fun\\>")
(defvar oz-meth-pattern "\\<meth\\>\\|\\<class\\>\\|\\<create\\>")

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

(defun oz-indent-line(&optional no-empty-line)
  (interactive)
  (let ((search-mode case-fold-search))
    (setq case-fold-search nil)
    (unwind-protect
	(save-excursion
	  (beginning-of-line)
	  (skip-chars-forward " \t")
	  (let ((col (save-excursion (oz-calc-indent no-empty-line))))
	    (cond ((< col 0) t)
		  ((not (= col (current-column)))
		   (delete-horizontal-space)
		   (indent-to col)))))
      (if (is-first-in-row)
	  (skip-chars-forward " \t"))
      (setq case-fold-search search-mode))))


(defun oz-calc-indent(no-empty-line)
  (cond ((and no-empty-line (is-first-in-row) (is-last-in-row))
	  ;; empty lines are not changed
	 -1)
	((and no-empty-line (looking-at "%"))
	 -1)
	((looking-at "\\<in\\>")
	 (oz-search-matching-begin t)
	 )
	((or (looking-at oz-end-pattern) (looking-at oz-middle-pattern))
	 ;; we must indent to the same column as the matching begin
	 (oz-search-matching-begin))
	((looking-at oz-right-pattern)
	 (oz-search-matching-paren)
	 )
	(t (oz-calc-indent1))
	)
  )


(defun oz-calc-indent1 ()
  ;; the heavy case
  ;; backward search for
  ;;    the next oz-key-pattern
  ;; or for a line without an oz-pattern
  ;; or for an paren
  (if (re-search-backward
       (concat oz-key-pattern 
	       "\\|" "^[ \t]*[^ \t\n]"
	       "\\|" oz-left-pattern "\\|" oz-right-pattern
	       )
       0 t)
      (cond ((or (oz-comment-start) (oz-middle-of-string))
	     (oz-calc-indent1))
	    ((looking-at "\\<in\\>")
	     ;; we are the first token after an 'in'
	     (goto-char (match-end 0)) ; skip 'in'
	     (if (is-last-in-row)
		 (progn
		   (search-backward "in")
		   (oz-search-matching-begin t)
		   (if (looking-at "\\<exists\\>\\|\\<declare\\>")
		       (current-column)
		     (+ (current-column) oz-indent-chars)))
	       (re-search-forward "[^ \t]")
	       (1- (current-column))))
	    ((looking-at oz-abstr-pattern)
	     ;; the arguments of pred are behind
	     ;; so indent the body to the start of 'pred'
	     (+ (current-column) oz-indent-chars)
	     )
;	    ((looking-at oz-abstr-pattern)
;	     (beginning-of-line)
;	     (skip-chars-forward " \t")
;	     (+ (current-column) oz-indent-chars))
	    ((looking-at oz-meth-pattern)
	     ;; the arguments of pred are behind
	     ;; so indent the body to the start of 'pred'
	     (+ (current-column) oz-indent-chars)
	     )
	    ((or (looking-at oz-begin-pattern)
		 (looking-at "\\<else\\>\\|\\[\\]"))
	     ;; we are the first token after 'if' 'pred' ...
	     (let ((col (current-column)))
	       (goto-char (match-end 0))
	       (if (is-last-in-row)
		   (+ col oz-indent-chars)
		 (re-search-forward "[^ \t]")
		 (1- (current-column)))
	       )
	     )
	    ((looking-at oz-middle-pattern)
	     ;; we are the first token after 'then of in'
	     (+ (oz-search-matching-begin) oz-indent-chars))
	    ((looking-at oz-end-pattern)
	     ;; we are the first token after an 'fi end'
	     (oz-search-matching-begin))
	    ((looking-at oz-right-pattern)
	     ;; we are the first token after {..} [..] (..)
	     (oz-search-matching-paren)
	     (oz-calc-indent1)
	     )
	    ((looking-at oz-left-pattern)
	     ;; we are the first token after open paren
	     (forward-char)
	     (if (is-last-in-row)
		 (current-column)
	       (re-search-forward "[^ \t]")
	       (1- (current-column))
	       )
	     )
	    ((is-first-in-row)
	     ;; we are a token of an list (found a line without an oz-pattern)
	     (re-search-forward "[^ \t]")
	     (1- (current-column)))
	    (t
	     ;; else impossible
	     (error "mm2"))
	    )
    ;; else: nothing found: beginning of file
    0
    )
  )

(defun oz-comment-start ()
  (let ((p (point)))
    (re-search-backward "%\\|^" (point-min) t)
    (if (looking-at "%")
	t
      (goto-char p)
      nil)))

(defun oz-middle-of-string ()
  (let ((p (point))
	c)
    (re-search-forward "['\"`]\\|$" (point-max) t)
    (setq c (char-to-string (preceding-char)))
    (if (looking-at "$")
	(progn
	  (goto-char p) nil)
      (goto-char p)
      (re-search-backward (concat c "\\|$") 0 t)
      (if (looking-at c)
	  t
	(goto-char p)
	nil))))

(defun is-first-in-row()
  (save-excursion
    (skip-chars-backward " \t")
    (if (= (current-column) 0)
	t
      nil)))

(defun is-last-in-row()
  (if (looking-at "[ \t]*$")
      t
    nil))

(defun oz-search-matching-paren()
  (let ((s t)
	(n 0))
    (while s
      (if (re-search-backward
	   (concat oz-left-pattern "\\|" oz-right-pattern)
	   0 t) 
	  (cond ((or (oz-comment-start) (oz-middle-of-string))
		 t)
		((looking-at oz-left-pattern)
		 (if (= n 0)
		     (setq s nil)
		   (setq n (- n 1))
		   ))
		(t (setq n (+ n 1))))
	(error "no matching left paren"))))
  (current-column))

(defun oz-search-matching-begin(&optional search-exists)
  (let ((s t)
	(n 0))
    (while s
      (if (re-search-backward oz-key-pattern 0 t) 
	  (cond ((or (oz-comment-start) (oz-middle-of-string))
		 t)
		((looking-at "\\<exists\\>\\|\\<declare\\>")
		 (if (and search-exists (= n 0))
		     (setq s nil)
		   )
		 )
;		((and (looking-at oz-abstr-pattern) (= n 0))
;		 (setq s nil)
;		 (beginning-of-line)
;		 (skip-chars-forward " \t")
;		 (current-column))
		((looking-at oz-begin-pattern)
		 ;; 'if'
		 (if (= n 0)
		     (setq s nil)
		   (setq n (- n 1))))
		((and (looking-at "\\<else\\>\\|\\[\\]") (= n 0))
		 (setq s nil))
		((looking-at oz-middle-pattern)
		 ;; 'then' '[]'
		 t)
		(t
		 ;; 'end'
		 (setq n (+ n 1)))
		)
	(error "no matching begin token"))
      (setq search-exists nil)))
  (current-column))

(defun oz-find-paren()
  (interactive)
  (beginning-of-line)
  (let* ((point (point))
	 (middle-pat (looking-at oz-middle-pattern))
	 (counter 0))

    (while (and (>= counter 0) 
		(re-search-backward 
		    (concat oz-begin-pattern "\\|" oz-end-pattern) 0 t))
      (if (looking-at oz-begin-pattern) 
	  (setq counter (1- counter))
	(setq counter (1+ counter))))
    (if (not middle-pat)
	(skip-chars-forward " \t"))
    (if (< counter 0)
	(if middle-pat
	    t
	  (let ((now (point))
		(is-paren (looking-at "(")))
	    (goto-char point)
	    (forward-line -1)
	    (beginning-of-line)
	    (skip-chars-forward " \t")
	    (if (and (not is-paren)
		     (not (looking-at oz-middle-pattern))
		     (< now (point)))
		t
	      (goto-char now)
	      (re-search-forward oz-begin-pattern (point-max) t)
	      (skip-chars-forward " \t"))))
      (beginning-of-line))
    (current-column)))




;;------------------------------------------------------------
;; Fontification
;;------------------------------------------------------------

(defconst oz-keywords
   (concat
    (oz-make-keywords-for-match
     '(
       "pred" "proc" "fun"
       "local" "declare"
       "if" "or" "case" "then" "else" "elseif" "of" "elseof" "end" "fi" "ro"
       "class" "create" "meth" "extern" "from" "with" "attr" "feat" "self"
       "true" "false"
       "div" "mod"
       "not" "process" "in"
       ))
    "\\|\\.\\|\\[\\]\\|#\\|!\\|\\^\\|:\\|\\@"
    ))


(defun oz-fontify-buffer()
  (interactive)
  (font-lock-fontify-buffer))


(defun oz-fontify-region(beg end)
  (font-lock-fontify-region beg end))


(defun oz-fontify(&optional arg)
  (interactive "P")
  (oz-hide-errors)
  (recenter arg)
  (oz-fontify-buffer))


;;------------------------------------------------------------
;; Filtering process output
;;------------------------------------------------------------


(defun oz-machine-filter (proc string)
  (oz-filter proc string 'oz-machine-state))


(defun oz-compiler-filter (proc string)
  (oz-filter proc string 'oz-compiler-state))


(defvar oz-escape-chars
  (concat oz-status-string "\\|" oz-warn-string "\\|" oz-error-string)
  "")

(defvar oz-error-chars
  (concat oz-error-string)
  "")


(defun oz-filter (proc string state-string)
  (let ((old-buffer (current-buffer)))
    (unwind-protect
	(let ((newbuf (process-buffer proc))
	      help-string old-point
	      match-start match-end
	      moving)
	  (set-buffer newbuf)
	  (setq moving (= (point) (process-mark proc)))

	  (save-excursion
	    ;; Insert the text, moving the process-marker.
	    (goto-char (process-mark proc))
	    (setq old-point (point))
	    (insert-before-markers string)
	    (set-marker (process-mark proc) (point))

	    ;; show status messages of compiler in mini buffer
	    (setq help-string string)

      ;; only display the last status message
	    (while (setq match-start
			 (string-match oz-status-string help-string))
	      (setq help-string (substring help-string (+ 1 match-start))))

	    (if (string= string help-string)
		t
	      (setq match-end (string-match "\n" help-string))
	      (oz-set-state state-string (substring help-string 0 match-end)))
            
	    ;; remove escape characters
	    (goto-char old-point)
	    (while (search-forward-regexp oz-escape-chars nil t)
	      (replace-match "" nil t))
	    (goto-char (point-max)))
	  (if moving (goto-char (process-mark proc))))
      (set-buffer old-buffer)
      )
    ;; error output
    (if (or oz-errors-found (string-match oz-error-chars string))   ; contains errors ?
	(progn
	  (setq oz-errors-found t)
	  (oz-show-error string)))

    ;; reset error output if we have another message prefix than error
    (if (and (not (string-match oz-error-chars string))
	     (string-match oz-escape-chars string))
	(setq oz-errors-found nil))))

;;------------------------------------------------------------
;; buffers
;;------------------------------------------------------------

(defvar oz-other-buffer-percent 35 
  "
How many percent of the actual screen will be occupied by the
OZ compiler, machine and error window")

(defun oz-show-buffer (buffer)
  (save-excursion
    (let* ((edges (window-edges (selected-window)))
	   (win (or (get-buffer-window oz-machine-buffer)
		    (get-buffer-window "*Oz Compiler*")
		    (get-buffer-window "*Oz Errors*")
		    (split-window (selected-window)
				  (/ (* (- (nth 3 edges) (nth 1 edges))
					(- 100 oz-other-buffer-percent))
				     100)))))
      (set-window-buffer win buffer)
      )
    )

  (bury-buffer oz-machine-buffer)
  (bury-buffer "*Oz Compiler*")
  (bury-buffer "*Oz Errors*")
  (bury-buffer buffer))


(defun oz-create-buffer (buf)
  (save-excursion
    (set-buffer (get-buffer-create buf))

;; enter oz-mode but no highlighting !
    (kill-all-local-variables)
    (use-local-map oz-mode-map)
    (setq mode-name "Oz-Output")
    (setq major-mode 'oz-mode)
    (if oz-lucid
     (set-buffer-menubar (append current-menubar oz-menubar)))
    (delete-region (point-min) (point-max))))


(defun oz-hide-errors()
  (interactive)
  (setq oz-errors-found nil)
  (let ((show-machine (or (get-buffer-window oz-machine-buffer)
			  (get-buffer-window "*Oz Temp*")
			  (get-buffer-window "*Oz Compiler*")
			  (get-buffer-window "*Oz Errors*"))))
    (if (get-buffer "*Oz Errors*") 
	(delete-windows-on "*Oz Errors*"))
    (if (get-buffer "*Oz Temp*") 
	(delete-windows-on "*Oz Temp*"))
    (if (and oz-machine-visible show-machine)
	(oz-show-buffer oz-machine-buffer))))


(defun oz-show-error(string)
  (let ((buf (get-buffer-create "*Oz Errors*"))
	old-point)
    (save-excursion
      (set-buffer buf)
      ; if buffer is not visible then clear it out
      (if (not (get-buffer-window buf))
	  (delete-region (point-min) (point-max)))
      (setq old-point (point-max))
      (goto-char old-point)
      (insert-before-markers string)
    
      ;; remove other than error messages
      (goto-char old-point)
      (while (search-forward-regexp 
	      (concat oz-status-string ".*\n") nil t)
	(replace-match "" nil t))

      ;; remove escape characters
      (goto-char old-point)
      (while (search-forward-regexp oz-escape-chars nil t)
	(replace-match "" nil t))
      (goto-char (point-max)))

    (oz-show-buffer buf)))

(defun oz-toggle-compiler()
  (interactive)
  (setq oz-errors-found nil)
  (if (get-buffer-window "*Oz Compiler*")
      (progn
	(delete-windows-on "*Oz Compiler*")
	(if oz-machine-visible
	    (oz-show-buffer oz-machine-buffer)))
    (oz-toggle-window "*Oz Compiler*")))


(defun oz-toggle-machine()
  (interactive)
  (setq oz-errors-found nil)
  (oz-toggle-window oz-machine-buffer)
  (setq oz-machine-visible (get-buffer-window oz-machine-buffer)))


(defun oz-toggle-errors()
  (interactive)
  (setq oz-errors-found nil)
  (if (get-buffer-window "*Oz Errors*")
      (oz-hide-errors)
    (oz-toggle-window "*Oz Errors*")))


(defun oz-toggle-window(buffername)
  (if (get-buffer buffername)
      (if (get-buffer-window buffername t)
	  (delete-windows-on buffername)
	(oz-show-buffer (get-buffer buffername)))))


(defun oz-new-buffer()
  (interactive)
  (oz-hide-errors)
  (switch-to-buffer (generate-new-buffer "Oz"))
  (oz-mode))


(defun oz-previous-buffer()
  (interactive)
  (oz-hide-errors)
  (bury-buffer)
  (oz-walk-trough-buffers (buffer-list)))


(defun oz-next-buffer()
  (interactive)
  (oz-hide-errors)
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
   (oz-directive-on-region start end "!pi" ".i" t))


(defun oz-to-machinecode-buffer()
  (interactive)
  (oz-to-machinecode-region (point-min) (point-max)))

(defun oz-to-machinecode-line()
  (interactive)
  (let ((line (oz-line-pos)))
    (oz-to-machinecode-region (car line) (cdr line))))

(defun oz-to-machinecode-region (start end)
   (interactive "r")
   (oz-directive-on-region start end "!compile" ".ham" nil))




(defun oz-directive-on-region (start end directive suffix mode)
  "Applies a directive to the region."
   (oz-hide-errors)
   (let ((file-1 (concat oz-temp-file ".oz"))
	 (file-2 (concat oz-temp-file suffix)))
     (if (file-exists-p file-2)
	 (shell-command (concat "rm -f " file-2)))
     (write-region start end file-1)
     (message "")
     (oz-hide-errors)
     (oz-send-string (concat directive " '" file-1 "'\n"))
     (sleep-for 2)
     (while (not (file-exists-p file-2))
       (sleep-for 2)
       )
     (if (not (get-buffer-window "*Oz Errors*"))
	 (let ((buf (get-buffer-create "*Oz Temp*")))
	   (save-excursion
	     (set-buffer buf)
	     (delete-region (point-min) (point-max))
	     (insert-file-contents file-2)
	     (shell-command (concat "rm -f " file-1 " " file-2))
	     (display-buffer buf t)
	     (oz-mode)
	     (if mode (oz-fontify-buffer)))))))


(defun oz-feed-region-browse (start end)
  "Feed the current region into the Oz Compiler"
  (interactive "r")
  (oz-hide-errors)
  (let ((contents (buffer-substring start end)))
    (oz-send-string (concat "{Browse " contents "}\n"))))


(defun oz-feed-region-browse-memory (start end)
  "Feed the current region into the Oz Compiler"
  (interactive "r")
  (oz-hide-errors)
  (let ((contents (buffer-substring start end)))
    (oz-send-string (concat "{Browse.memory " contents "}\n"))))


(defun oz-feed-panel ()
  "Feed {Panel popup} into the Oz Compiler"
  (interactive)
  (oz-hide-errors)
  (oz-send-string "{Panel popup}\n"))

(defun oz-feed-file(file)
  "Feed an file into the Oz Compiler"
  (interactive "FFeed file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!include '" file "'\n"))) 

(defun oz-precompile-file(file)
  "precompile an Oz file"
  (interactive "FPrecompile file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!precompile '" file "'\n"))) 



(defun oz-find-dvi-file ()
  "preview a file from  Oz doc directory"
  (interactive)
  (let ((name (read-file-name
	       "Preview File: "
	       (concat oz-home "doc/")
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

(defun oz-find-lib-file()
  "find a Oz file in the lib directory"
  (interactive)
  (oz-find-file "Find library file: " "lib/"))

(defun oz-find-file(prompt file)
  (find-file (read-file-name prompt
			     (concat oz-home file)
			     nil
			     t
			     nil)))


