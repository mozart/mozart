;; Major mode for editing Oz, and for running Oz under Emacs
;; Copyright (C) 1993 DFKI GmbH
;; Author: Ralf Scheidhauer and Michael Mehl ([scheidhr|mehl]@dfki.uni-sb.de)


;; TODO
;; - state only in initial Screens for GNU ?? Should we use the mode-line ???


(require 'comint)

;; ---------------------------------------------------------------------
;; global effects

(setq completion-ignored-extensions
      (append '(".load" ".sym")
	      completion-ignored-extensions))


;; ----------------------------------------------------------------------
;; support for different emacs versions: gnu19 and lucid


(defvar oz-emacs-version
  (cond ((string-match "Lucid" emacs-version) 'lucid)
	((string-match "19" emacs-version) 'gnu19))
  "use the right functions for fontifying etc")


;(byte-compiler-options (optimize t) (warnings (- free-vars))
;  (file-format emacs18))



(defmacro oz-version (lucid gnu)
  (` (cond (,(cons '(eq oz-emacs-version 'lucid) lucid))
	   (,(cons '(eq oz-emacs-version 'gnu19) gnu)))))

;;------------------------------------------------------------
;; Screen title bars
;;------------------------------------------------------------

(defvar oz-compiler-state "???")
(defvar oz-machine-state  "???")

(defvar oz-old-screen-title
  (oz-version
   (screen-title-format)
   ((cdr (assoc 'name (frame-parameters))))))


(defun oz-reset-state()
  (oz-version
   ((setq screen-title-format oz-old-screen-title))
   ((mapcar '(lambda(scr)
	       (modify-frame-parameters 
		scr
		(list (cons 'name oz-old-screen-title))))
	    (visible-screen-list)))))

(defun oz-set-screen-name(name)
  (oz-version
   ((setq screen-title-format name))
   ((mapcar '(lambda(scr)
	       (modify-frame-parameters 
		scr
		(list (cons 'name name))))
	    (visible-screen-list)))))

(defvar oz-title-format "Oz Console          C: %s  M: %s")

(defun oz-canon-status-string(s)
  (cond ((string-match "\\<idle\\>" s) s)
	((string-match "\\<running\\>" s) "running")
	((string-match "\\<halted\\>" s) "halted")
	((string-match "\\<booting\\>" s) "booting")
	( t "???")))


(defun oz-set-state(state string)
  (if (string= string "")
      t
    (setq string (oz-canon-status-string string))
    (set state 
	 (format "%-30s" 
		 (substring string 0 
			    (min 30 (length string)))))
    (oz-set-screen-name (format oz-title-format 
				oz-compiler-state 
				oz-machine-state))))




;;------------------------------------------------------------
;; Variables
;;------------------------------------------------------------

(defvar oz-indent-chars 3
"*Indentation of Oz statements with respect to containing block.")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)
(defvar oz-mode-map (make-sparse-keymap))

(defvar oz-compiler "oz.compiler")
(defvar oz-machine "oz.machine")

(defvar oz-machine-hook nil
  "Hook used if non nil for starting machine.
For example
  (setq oz-machine-hook 'gdb-machine)
")

(defvar oz-wait-for-compiler 5
  "Wait between startup of compiler and engine")

(defvar oz-home (concat (or (getenv "OZHOME") "/usr/share/gs/soft/oz") "/")
  "The directory where oz is installed")

(defvar oz-doc-dir (concat oz-home "doc/")
  "The default doc directory")

(defvar oz-doc-file "quick.dvi"
  "The default doc file")

(defvar oz-preview "xdvi"
  "The previewer for doc files")


(defvar oz-error-string (format "%c" 17)
  "how compiler and engine signal errors")

(defvar oz-warn-string (format "%c" 18)
  "how compiler and engine signal warnings")

(defvar oz-status-string (format "%c" 19)
  "How compiler and engine signal status changes")



;;------------------------------------------------------------
;; some wrappers for FSF Emacs
;;------------------------------------------------------------

(oz-version
 ()
 ((defalias 'screen-list 'frame-list)
  (defalias 'modify-screen-parameters 'modify-frame-parameters)
  (defalias 'visible-screen-list 'visible-frame-list)
  (defalias 'iconify-screen 'iconify-frame)
  (defalias 'set-screen-size 'set-frame-size)
  (defalias 'selected-screen 'selected-frame)
  (defalias 'select-screen 'select-frame)
  (defalias 'new-screen 'new-frame)
  (defalias 'delete-extent 'delete-overlay)
  (defalias 'make-extent 'make-overlay))
 )


(defun oz-make-screen-visible(scr)
  (oz-version
   ((make-screen-visible scr))
   ((if (eq (frame-visible-p scr) 'icon)
	(progn
	  (select-frame scr)
	  (iconify-or-deiconify-frame)))
    (raise-frame scr))))

 

(defun oz-display-buffer (buf bool scr)
  (oz-version 
   ((display-buffer buf bool scr))
   ((select-frame scr)
    (display-buffer buf bool))))


(if oz-mode-syntax-table
    ()
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?_ "w" table)
    (modify-syntax-entry ?\\ "." table)
    (modify-syntax-entry ?/ "." table)
    (modify-syntax-entry ?* "." table)
    (modify-syntax-entry ?+ "." table)
    (modify-syntax-entry ?- "." table)
    (modify-syntax-entry ?= "." table)
    (modify-syntax-entry ?% "<" table)
    (modify-syntax-entry ?< "." table)
    (modify-syntax-entry ?> "." table)
    (modify-syntax-entry ?\' "\"" table)
    (setq oz-mode-syntax-table table)))

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
  (make-local-variable 'after-change-function)
  (setq after-change-function 'oz-after-change-function)
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
  (define-key map "\C-c\C-e"    'oz-toggle-errors)
  (define-key map "\C-c\C-c"    'oz-toggle-compiler-window)

  (oz-version
   ((define-key map [(control button1)]       'oz-feed-region-browse))
   nil)
  (define-key map "\C-c\C-m"    'oz-toggle-machine-window)
  (define-key map "\C-c\C-h"    'halt-oz)
  (define-key map "\C-c\C-i"    'oz-include-file)
  (define-key map "\C-c\C-n"    'oz-new-buffer)
  (define-key map "\C-c\C-l"    'oz-prettyprint)
  (define-key map "\C-c\C-r"    'run-oz)
  (define-key map "\C-cc"    'oz-precompile-file)
  )

(oz-mode-commands oz-mode-map)

;;------------------------------------------------------------
;; Fonts
;;------------------------------------------------------------

(defvar oz-small-font      '("-adobe-courier-" . "-*-*-*-100-*-*-*-*-*-*"))
(defvar oz-default-font    '("-adobe-courier-" . "-*-*-*-120-*-*-*-*-*-*"))
(defvar oz-large-font      '("-adobe-courier-" . "-*-*-*-140-*-*-*-*-*-*"))
(defvar oz-very-large-font '("-adobe-courier-" . "-*-*-*-180-*-*-*-*-*-*"))

(defun oz-small-font()
  (interactive)
  (oz-set-default-font oz-small-font))

(defun oz-default-font()
  (interactive)
  (oz-set-default-font oz-default-font))

(defun oz-large-font()
  (interactive)
  (oz-set-default-font oz-large-font))

(defun oz-very-large-font()
  (interactive)
  (oz-set-default-font oz-very-large-font))

(defun oz-set-default-font(font)
  (let ((scr (selected-screen)))
    (oz-version
     ((set-face-font 'default (concat (car font) "medium-r" (cdr font)) scr))
     ((modify-screen-parameters
       scr
       (list (cons 'font  (concat (car font) "medium-r" (cdr font)))))))

    (set-face-font 'bold nil scr)
    (set-face-font 'bold (concat (car font) "bold-r" (cdr font)) scr)
    (set-face-font 'italic nil scr)
    (set-face-font 'italic (concat (car font) "medium-o" (cdr font)) scr)))


;;------------------------------------------------------------
;; Menus
;;------------------------------------------------------------

(defvar oz-menubar nil)

(defun oz-make-menu(list)
  (oz-version
   ((setq oz-menubar (oz-make-menu-lucid list)))
   ((oz-make-menu-gnu19 oz-mode-map
			(list (cons "menu-bar" list))))))

(defun oz-make-menu-lucid (list)
  (if (eq list nil)
      nil
    (cons
     (let* ((entry (car list))
	    (name (car entry))
	    (aname (intern name))
	    (rest (cdr entry)))
;;;      (message "entry: %s %s" name rest) (sleep-for 1)
       (if (atom rest)
	   (vector name rest t)
	 (cons name (oz-make-menu-lucid rest))))
     (oz-make-menu-lucid (cdr list)))))


;; for gnu19
(defun oz-make-menu-gnu19 (map list)
  (if (eq list nil)
      nil
    (let* ((entry (car list))
	   (name (car entry))
	   (aname (intern name))
	   (rest (cdr entry)))
;;;      (message "entry: %s %s" name rest) (sleep-for 1)
      (if (atom rest)
	  (define-key map (vector aname) entry)
	(let ((newmap (make-sparse-keymap name)))
	  (define-key map (vector aname)
	    (cons (concat "< " name " >")
		  newmap))
;;;	  (message "rest: %s" rest) (sleep-for 1)
	  (oz-make-menu-gnu19 newmap rest))))
    (oz-make-menu-gnu19 map (cdr list))))

;;; OZ MODE
(oz-make-menu
 '(("Oz"
    ("Feed buffer"            . oz-feed-buffer)
    ("Feed region"            . oz-feed-region)
    ("Feed line"              . oz-feed-line)
;;;     "-----"
    ("Include file"           . oz-include-file)
    ("Compile file"           . oz-precompile-file)
    ("Find"
     ("Demo file"              . oz-find-demo-file)
     ("Library file"           . oz-find-lib-file)
     ("Documentation"          . oz-find-docu-file)
     )
;;;     "-----"
    ("New Oz buffer"          . oz-new-buffer)
    ("Refresh buffer"         . oz-prettyprint)
    ("Print"
     ("buffer"      . oz-print-buffer)
     ("region"      . oz-print-region)
     )
    ("Core Syntax"
     ("buffer"      . oz-ks-buffer)
     ("region"      . oz-ks-region)
     ("line"        . oz-ks-line  )
     )
    ("Indent"
     ("line" . oz-indent-line)
     ("region" . oz-indent-region)
     ("buffer" . oz-indent-buffer)
     )
    ("Show/hide"
     ("errors"       . oz-toggle-errors)
     ("compiler"     . oz-toggle-compiler-window)
     ("machine"      . oz-toggle-machine-window)
     )
    ("Browse"   . oz-feed-region-browse)
;;;       "-----"
    ("Start Oz" . run-oz)
    ("Halt Oz"  . halt-oz)
    )
   ("Font"
    ("Small"      . oz-small-font     )
    ("Default"    . oz-default-font    )
    ("Large"      . oz-large-font     )
    ("Very Large" . oz-very-large-font)
    )
   ))

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
  (oz-version
   ((set-buffer-menubar (append current-menubar oz-menubar)))
   nil)
  (run-hooks 'oz-mode-hook))

(defun run-oz ()
  "Run an inferior Oz process, input and output via buffer *Oz Compiler*."
  (interactive)
  (oz-check-running)
  (if (get-process "Oz Compiler")
      (error "Oz already running")
    (start-oz-process)
    (if (not (eq major-mode 'oz-mode)) (oz-new-buffer))))


(defun ensure-oz-process ()
  (oz-check-running)
  (start-oz-process))


(defun oz-check-running()
  (if (and (get-process "Oz Compiler")
	   (not (get-process "Oz Machine")))
      (progn 
	(oz-set-state 'oz-machine-state "???")
	(error "Machine has died, for some unknown reason, try halting Oz")))
  (if (and (not (get-process "Oz Compiler"))
	   (get-process "Oz Machine"))
      (progn 
	(oz-set-state 'oz-compiler-state "???")
	(error "Compiler has died, for some unknown reason, try halting Oz"))))

(defvar oz-machine-visible nil "")

(defun start-oz-process()
  (or (get-process "Oz Compiler")
      (let ((file (oz-make-temp-name "/tmp/ozsock")))
	(setq oz-machine-visible (get-buffer-window "*Oz Machine*"))

	(oz-set-state 'oz-compiler-state "booting")
        (make-comint "Oz Compiler" oz-compiler nil "-S" file)
	(oz-create-buffer "*Oz Compiler*")
	(set-process-filter (get-process "Oz Compiler") 'oz-compiler-filter)
	(bury-buffer "*Oz Compiler*")

	(if oz-machine-hook
	    (funcall oz-machine-hook file)
	  (if oz-wait-for-compiler (sleep-for oz-wait-for-compiler))
	  (oz-set-state 'oz-machine-state "booting")
	  (make-comint "Oz Machine" oz-machine nil "-S" file)
	  (oz-create-buffer "*Oz Machine*")
	  (set-process-filter (get-process "Oz Machine")  'oz-machine-filter)
	  (bury-buffer "*Oz Machine*")
	  (save-excursion
	    (set-buffer (get-buffer "*Oz Machine*"))
	    (delete-region (point-min) (point-max))
	    )
	  )

	;; make sure buffers exist
	(oz-create-buffer "*Oz Errors*")

	(oz-version
	 ((setq screen-title-format
		'(("Oz Console           C:  "   (-30 . oz-compiler-state))
		  ("   M:  " (-30 . oz-machine-state)))))
	 nil))))


(defvar gdb-oz-machine "oz.machine.bin")

(defun gdb-machine (tmpfile)
  "Run gdb on oz-machine in buffer *Oz Machine*.
The directory containing FILE becomes the initial working directory
and source-file directory for GDB.  If you wish to change this, use
the GDB commands `cd DIR' and `directory'."
  (oz-set-state 'oz-machine-state "running under gdb")
  (let* ((path (expand-file-name gdb-oz-machine))
	(file (file-name-nondirectory path)))
    (setq default-directory (file-name-directory path))

    (make-comint "Oz Machine" gdb-command-name nil "-fullname"
		 "-cd" default-directory file)
    (save-excursion
      (set-buffer (get-buffer "*Oz Machine*"))
      (delete-region (point-min) (point-max))
      (gdb-mode)
      )
    (set-process-filter (get-process "Oz Machine") 'gdb-filter)
    (set-process-sentinel (get-process "Oz Machine") 'gdb-sentinel)
    (process-send-string (get-process "Oz Machine")
			 (concat "run -S " tmpfile "\n"))
    (setq current-gdb-buffer (get-buffer "*Oz Machine*"))
    )
  )


(defun oz-create-buffer (buf)
  (save-excursion
    (set-buffer (get-buffer-create buf))

;; enter oz-mode but no highlighting !
    (kill-all-local-variables)
    (use-local-map oz-mode-map)
    (setq mode-name "Oz-View")
    (setq major-mode 'oz-mode)
    (oz-version
     ((set-buffer-menubar (append current-menubar oz-menubar)))
     nil)

    (delete-region (point-min) (point-max))))

(defun oz-doc ()
  (interactive)
  (let ((name (read-file-name
	       (format "Preview File [%s]: " oz-doc-file)
	       oz-doc-dir (concat oz-doc-dir oz-doc-file) t)))
    (if (file-exists-p name)
	(start-process "OZ Doc" "*Preview*" oz-preview name)
      (error "file %s doesn't exists" name))))


;;------------------------------------------------------------
;;Compiling
;;------------------------------------------------------------


(defun oz-feed-buffer ()
  "Feeds the entire buffer."
  (interactive)
  (let ((file (buffer-file-name))
	(cur (current-buffer)))
    (if (or (not file) (buffer-modified-p))
	(oz-feed-region (point-min) (point-max))
      (oz-include-file file))
    (switch-to-buffer cur)))

(defun oz-feed-region (start end)
  "Consults the region."
   (interactive "r")
   (oz-hide-errors)
   (let ((contents (buffer-substring start end)))
    (oz-send-string (concat contents "\n"))))

(defun oz-feed-line ()
  "Consults one line."
   (interactive)
   (save-excursion
     (let ((line (oz-line-pos)))
       (oz-feed-region (car line) (cdr line)))))


(defun oz-line-pos()
  (let (beg end)
    (beginning-of-line)
    (setq beg (point))
    (end-of-line)
    (setq end (point))
    (cons beg end)))

(defvar oz-temp-counter 0)

(defun oz-make-temp-name(name)
  (setq oz-temp-counter (+ 1 oz-temp-counter))
  (format "%s%d" (make-temp-name name) oz-temp-counter))

(defvar oz-pretty-file (oz-make-temp-name "/tmp/ozpretty") "")

(defun oz-ks-buffer()
  (interactive)
  (oz-ks-region (point-min) (point-max)))

(defun oz-ks-line()
  (interactive)
  (let ((line (oz-line-pos)))
    (oz-ks-region (car line) (cdr line))))


(defun oz-ks-region (start end)
  "Consults the region."
   (interactive "r")
   (oz-hide-errors)
   (shell-command-on-region start end (concat "/bin/sh -c 'cat > " oz-pretty-file "'"))
   (message "")
   (oz-ks-file oz-pretty-file)
   (sleep-for 2)
   (let ((buf (get-buffer-create "*Oz Core Syntax*")))
     (save-excursion
       (set-buffer buf)
       (delete-region (point-min) (point-max))
       (insert-file-contents (concat oz-pretty-file ".i"))
       (display-buffer buf t)
       (oz-mode)
       (oz-fontify-buffer))))





(defun oz-feed-region-browse (start end)
  "Consults the region."
  (interactive "r")
  (oz-hide-errors)
  (let ((contents (buffer-substring start end)))
    (oz-send-string (concat "{Browse " contents "}\n"))))



(defun oz-include-file(file)
  (interactive "FInclude file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!include '" file "'\n"))) 

(defun oz-ks-file(file)
  (oz-hide-errors)
  (oz-send-string (concat "!pi '" file "'\n")))

(defun oz-load-file(file)
  (interactive "FLoad file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!load '" file "'\n"))) 

(defun oz-precompile-file(file)
  (interactive "FPrecompile file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!precompile '" file "'\n"))) 

(defun oz-find-demo-file()
  (interactive)
  (oz-find-file "Find demo file: " "demo/"))

(defun oz-find-lib-file()
  (interactive)
  (oz-find-file "Find library file: " "lib/"))

(defun oz-find-docu-file()
  (interactive)
  (oz-find-file "Find documentation [handbook.asc]: " "doc/"))


(defun oz-find-file(prompt file)
  (find-file (read-file-name prompt
			     (concat oz-home file)
			     nil
			     t
			     nil)))





(defvar oz-errors-found nil "")

(defun oz-hide-errors()
  (interactive)
  (setq oz-errors-found nil)
  (let ((show-machine (or (get-buffer-window "*Oz Machine*")
			  (get-buffer-window "*Oz Core Syntax*")
			  (get-buffer-window "*Oz Compiler*")
			  (get-buffer-window "*Oz Errors*"))))
    (if (get-buffer "*Oz Errors*") 
	(delete-windows-on "*Oz Errors*"))
    (if (get-buffer "*Oz Core Syntax*") 
	(delete-windows-on "*Oz Core Syntax*"))
    (if (and oz-machine-visible show-machine)
	(oz-show-buffer "*Oz Machine*"))))



(defun oz-toggle-compiler-window()
  (interactive)
  (if (get-buffer-window "*Oz Compiler*")
      (progn
	(delete-windows-on "*Oz Compiler*")
	(if oz-machine-visible
	    (oz-show-buffer "*Oz Machine*")))
    (oz-toggle-window "*Oz Compiler*")))

    




(defun oz-toggle-machine-window()
  (interactive)
  (oz-toggle-window "*Oz Machine*")
  (setq oz-machine-visible (get-buffer-window "*Oz Machine*")))


(defun oz-toggle-errors()
  (interactive)
  (if (get-buffer-window "*Oz Errors*")
      (oz-hide-errors)
    (oz-toggle-window "*Oz Errors*")))


(defun oz-toggle-window(buffername)
  (if (get-buffer buffername)
      (if (get-buffer-window buffername t)
	  (delete-windows-on buffername)
	(oz-show-buffer (get-buffer buffername)))))



(defun halt-oz()
  (interactive)
  
  (if (and (get-process "Oz Compiler")
	   (get-process "Oz Machine"))
      (oz-send-string "!halt \n"))

  (if (and (not (get-process "Oz Compiler"))
	   (not (get-process "Oz Machine")))
      (error "Oz not running"))

  (message "halting Oz...")
  (sleep-for 5)
  (if (get-process "Oz Compiler")
      (delete-process "*Oz Compiler*"))
  (if (get-process "Oz Machine")
      (delete-process "*Oz Machine*"))
  (message "")

  (oz-set-screen-name oz-old-screen-title))
    



(defun oz-send-string(string)
  (ensure-oz-process)
  (process-send-string "Oz Compiler" string)
  (process-send-eof "Oz Compiler"))

(defun oz-continue ()
  (interactive)
  (process-send-string "Oz Machine" "c\n"))

(defvar oz-other-buffer-percent 35 
  "
   How many percent of the actual screen will be occupied by the
   OZ compiler, machine and error window")

(defun oz-show-buffer (buffer)
  (save-excursion
    (let* ((edges (window-edges (selected-window)))
	   (win (or (get-buffer-window "*Oz Machine*")
		    (get-buffer-window "*Oz Compiler*")
		    (get-buffer-window "*Oz Errors*")
		    (split-window (selected-window)
				  (/ (* (- (nth 3 edges) (nth 1 edges))
					(- 100 oz-other-buffer-percent))
				     100)))))
      (set-window-buffer win buffer)
      )
    )

  (bury-buffer "*Oz Machine*")
  (bury-buffer "*Oz Compiler*")
  (bury-buffer "*Oz Errors*")
  (bury-buffer buffer))

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
      (if (eq major-mode 'oz-mode)
	  (progn (switch-to-buffer (car bufs))
		 (setq bool nil))
	  (setq bufs (cdr bufs))))
    (if (null bool)
	t
      (set-buffer cur)
      (error "No other oz-buffer"))))

  
(defun oz-make-keywords-for-match(args)
  (concat "\\<\\("
	  (mapconcat 'identity args "\\|")
	  "\\)\\>"))

(defconst oz-begin-pattern
      (oz-make-keywords-for-match 
	         '("local" "class" "meth" "create" "or" "if" "pred" "proc"
		   "fun"
		   "handle" "seq" "exists" "case" "begin" "process" "not"
		   )))

(defconst oz-end-pattern
      (oz-make-keywords-for-match '("end" "fi" "ro")))

(defconst oz-middle-pattern 
      (concat (oz-make-keywords-for-match
	       '("in" "then" "else" "elseif" "by" "of"))
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
		   (if (looking-at "exists")
		       (current-column)
		     (+ (current-column) oz-indent-chars)))
	       (re-search-forward "[^ \t]")
	       (1- (current-column))))
	    ((looking-at oz-abstr-pattern)
	     (beginning-of-line)
	     (skip-chars-forward " \t")
	     (+ (current-column) oz-indent-chars))
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
		((looking-at "\\<exists\\>")
		 (if (and search-exists (= n 0))
		     (setq s nil)
		   )
		 )
		((and (looking-at oz-abstr-pattern) (= n 0))
		 (setq s nil)
		 (beginning-of-line)
		 (skip-chars-forward " \t")
		 (current-column))
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
  (let* ((bool t)
	 (point (point))
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



(defun oz-delete-extents (from to)
  (if (< from to)
      (map-extents '(lambda(ext unused)
		      (delete-extent ext))
		   nil from to)))


(defun oz-change-match-face (face beg end)
  (oz-version
   ((set-extent-face (make-extent beg end) face))
   ((overlay-put (make-extent beg end) 'face face))))


(defconst ozKeywords
   (concat
    (oz-make-keywords-for-match
     '(
       "pred" "proc" "fun" "true" "false" "local" "begin" "end"
       "in" "not" "process" "det" "if" "then" "else" "elseif" 
       "fi" "or" "ro" "meth" "create" "class" "from" "with" 
       "exists" "case" "of"
       "wait" "div" "mod" "self"
       ))
    "\\|\\.\\|\\[\\]\\|#\\|!\\|\\^\\|:\\|\\@"
    ))



(oz-version
 nil
 ((defalias 'map-extents 'map-overlays)))


;; stolen from "cl-extra.el"
(defun map-overlays (cl-func &optional cl-buffer cl-start cl-end cl-arg)
  (or cl-buffer (setq cl-buffer (current-buffer)))
    ;; This alternate algorithm fails to find zero-length overlays.
    (let ((cl-mark (save-excursion (set-buffer cl-buffer)
                                   (copy-marker (or cl-start (point-min)))))
          (cl-mark2 (and cl-end (save-excursion (set-buffer cl-buffer)
                                                (copy-marker cl-end))))
          cl-pos cl-ovl)
      (while (save-excursion
               (and (setq cl-pos (marker-position cl-mark))
                    (< cl-pos (or cl-mark2 (point-max)))
                    (progn
                      (set-buffer cl-buffer)
                      (setq cl-ovl (overlays-at cl-pos))
                      (set-marker cl-mark (next-overlay-change cl-pos)))))
        (while (and cl-ovl
                    (or (/= (overlay-start (car cl-ovl)) cl-pos)
                        (not (and (funcall cl-func (car cl-ovl) cl-arg)
                                  (set-marker cl-mark nil)))))
          (setq cl-ovl (cdr cl-ovl))))
      (set-marker cl-mark nil) (if cl-mark2 (set-marker cl-mark2 nil))))


(defun oz-fontify-keywords(beg end)
  (save-excursion
    (goto-char beg)
    (while (re-search-forward ozKeywords end t)
      (oz-change-match-face 'bold (match-beginning 0) (match-end 0)))))

(defun oz-fontify-comments(beg end)
  (save-excursion
    (goto-char beg)
    (while  (condition-case () (re-search-forward "%\\|/\\*" end t) (error nil))
      (let ((beg (match-beginning 0))
	    (end))
	(if (= (preceding-char) 37)   ; == "%" ?
	    (re-search-forward "$")
	  (re-search-forward "\\*/" end t))
	(setq end (or (match-end 0) end))
      ;; delete extents within comment
	(oz-delete-extents beg end)
	(oz-change-match-face 'italic beg end)))))



(defun oz-fontify-buffer()
  (interactive)
  (oz-fontify-region (point-min) (point-max) t))


(defun oz-after-change-function(beg end len)
  (interactive)
  (save-match-data
    (oz-fontify-line)))


(defun oz-fontify-line()
  (interactive)
  (save-excursion
    (beginning-of-line)
    (let ((start (point)))
      (end-of-line)
      (oz-fontify-region start (point)))))


(defun oz-fontify-region(beg end &optional verbose)
  (if (null beg) (setq beg (mark)))
  (if (null end) (setq beg (point)))
  (if (eq major-mode 'oz-mode)
      (let ((old-case case-fold-search))
	;; search case dependent:
	(setq case-fold-search nil)
	(unwind-protect
	    (save-excursion
	      (if verbose (message "fontifying... (cleaning)"))
	      (oz-delete-extents beg end)
	      (if verbose (message "fontifying... (keywords)"))
	      (oz-fontify-keywords beg end)
	      (if verbose (message "fontifying... (comments)"))
	      (oz-fontify-comments beg end))
	  (setq case-fold-search old-case))
	(message ""))))
   

(defun oz-prettyprint(&optional arg)
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
  (concat oz-warn-string "\\|" oz-error-string)
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


(add-hook 'find-file-hooks 'oz-fontify-buffer)


(defun oz-print-buffer()
  "Print buffer."
  (interactive)
  (oz-print-region (point-min) (point-max)))


(defun oz-print-region(start end)
  "Print region."
  (interactive "r")
  (shell-command-on-region start end "oz2lpr -"))
