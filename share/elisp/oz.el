;; PLEASE USE CVS TO CHANGE THIS FILE:
;;  cvs get Oz/elisp
;;  ... edit ...
;;  make oz
;;  cvs commit
;; ----------------------------------------------------------------------
;; OZ-Mode
;; credits to mm and rs
;; ----------------------------------------------------------------------
;; list of changes:

;; 11.1.93 mm 
;;  "exists"-keyword
;; 12.1.93 mm
;;  "OZC/OZM"-removed
;; 20.1.93 mm
;;  (random t) included
;; 16.2.93 r2
;;  "seq", "wait", "on"-keywords
;; 17.3.93 cs
;;  "case", "of" keywords
;; 7.4.93 mm
;;  "seq" "wait" removed
;;  "proc" added
;; 15.4.93 mm
;;  add xdvi - Browser
;; 23.4.93 mm
;;  reimpl. oz-indent-line
;; 4.3.93 cs
;;  deleted some keywords for fontifying, fixed error
;; 15.3.93 rs
;; added "oz-print-region" and "oz-print-buffer"
;; 8.9.93 rs
;; random removed
;; call compiler directly, since it can now reliably bind the socket
;; --------------------------------------------------------------------------


(if (string-match "^18" emacs-version)
    (error "This version of 'oz.el' does not support Emacs 18"))

(defvar lucid-emacs 
  (string-match "Lucid" emacs-version)
  "Use Lucid-Emacs functions for fontifying for example")


;(byte-compiler-options (optimize t) (warnings (- free-vars)) (file-format emacs18))
(setq debug-on-error nil)
;(setq debug-on-error t)


(require 'comint)


;;------------------------------------------------------------
;; Screen title bars
;;------------------------------------------------------------

(defvar oz-compiler-state "dead")
(defvar oz-machine-state  "dead")


;; only for FSF Emacs
(defun oz-set-screen-name(scr name)
  (modify-frame-parameters 
       scr
       (list (cons 'name name))))

(defvar oz-title-format "C: %s  M: %s")


(defun oz-set-state(state string)
  (set state string)
  (if (not lucid-emacs)
      (mapcar '(lambda(scr)
		      (oz-set-screen-name scr
					  (format oz-title-format 
						  oz-compiler-state 
						  oz-machine-state)))
	      (visible-screen-list))))



;; only for Lucid
(setq screen-title-format
      '((" C:  "  (40 . oz-compiler-state))
	("   M:  " (-25 . oz-machine-state))))



;;------------------------------------------------------------
;; Variables
;;------------------------------------------------------------



(if lucid-emacs
    (load "blink-paren"))

(defvar oz-indent-chars 3
"*Indentation of Oz statements with respect to containing block.")

(defvar oz-input-to-oz nil 
"Defines wheter direkt input to oz is allowed")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)
(defvar oz-mode-map (make-sparse-keymap))
(defvar oz-input-mode-map nil)

(defvar oz-compiler "oz.compiler"
  "Oz system used by run-oz")

(defvar oz-machine "oz.machine"
  "Oz machine used by run-oz")

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


(if lucid-emacs
    t
  (defalias 'screen-list 'frame-list)
  (defalias 'modify-screen-parameters 'modify-frame-parameters)
  (defalias 'visible-screen-list 'visible-frame-list)
  (defalias 'iconify-screen 'iconify-frame)
  (defalias 'set-screen-size 'set-frame-size)
  (defalias 'selected-screen 'selected-frame)
  (defalias 'select-screen 'select-frame)
  (defalias 'new-screen 'new-frame)
  (defalias 'delete-extent 'delete-overlay)
  (defalias 'make-extent 'make-overlay))


(defun oz-make-screen-visible(scr)
  (if lucid-emacs
      (make-screen-visible scr)
    (if (eq (frame-visible-p scr) 'icon)
	(progn
	  (select-frame scr)
	  (iconify-or-deiconify-frame)))
    (raise-frame scr)))

 

(defun oz-display-buffer (buf bool scr)
  (if lucid-emacs
      (display-buffer buf bool scr)
    (select-frame scr)
    (display-buffer buf bool)))


(setq completion-ignored-extensions
      (append '(".load" ".sym")
	      completion-ignored-extensions))



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
  (if lucid-emacs
      (progn
	(define-key map [(hyper b)] 'oz-indent-buffer)
	(define-key map [(hyper return)] 'oz-indent-buffer)
	(define-key map [(hyper r)] 'oz-indent-region)
	(define-key map [(hyper l)] 'oz-indent-line)
	)
    )
  (define-key map "\M-\C-m" 'oz-feed-buffer)
  (define-key map "\M-r"    'oz-feed-region)
  (define-key map "\M-i"    'oz-feed-file)
  (define-key map "\M-l"    'oz-feed-line)
  (define-key map "\M-m"    'oz-toggle-machine-window)
  (define-key map "\M-c"    'oz-toggle-compiler-window)
  (define-key map "\M-e"    'oz-toggle-errors)
  (define-key map "\M-z"    'oz-prettyprint))

(oz-mode-commands oz-mode-map)

;;------------------------------------------------------------
;; Fonts
;;------------------------------------------------------------

(defvar oz-small-font    '("-*-courier-" . "-*-*-*-100-*-*-*-*-iso8859-*"))
(defvar oz-default-font   '("-*-courier-" . "-*-*-*-120-*-*-*-*-iso8859-*"))
(defvar oz-large-font    '("-*-courier-" . "-*-*-*-140-*-*-*-*-iso8859-*"))
(defvar oz-very-large-font '("-*-courier-" . "-*-*-*-180-*-*-*-*-iso8859-*"))

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
    (if lucid-emacs
	(set-face-font 'default (concat (car font) "medium-r" (cdr font)) scr)
      (modify-screen-parameters scr
			       (list (cons 'font  (concat (car font) "medium-r" (cdr font))))))

    (set-face-font 'bold nil scr)
    (set-face-font 'bold (concat (car font) "bold-r" (cdr font)) scr)
    (set-face-font 'italic nil scr)
    (set-face-font 'italic (concat (car font) "medium-o" (cdr font)) scr)))


;;------------------------------------------------------------
;; Menus
;;------------------------------------------------------------

(defun oz-make-menu (name list)
  (global-set-key (vector 'menu-bar name)
       (cons (symbol-name name) (make-sparse-keymap (symbol-name name))))
  (mapcar '(lambda(entry)
	     (global-set-key 
	          (vector 'menu-bar name (car entry))
		  (cons (symbol-name (car entry)) (cdr entry))))
	  (reverse list))
  (setq menu-bar-final-items (list name)))
 
;;; OZ MODE
(if lucid-emacs
    (defvar oz-menubar 
      ;;(setq oz-menubar 
      (append default-menubar
	  '(("Oz"     
	     ["Feed buffer"            oz-feed-buffer t]
	     ["Feed region"            oz-feed-region t]
	     ["Feed line"              oz-feed-line t]
              "-----"
	     ["Next Oz buffer"         oz-next-buffer t]
	     ["Previous Oz buffer"     oz-previous-buffer t]
	     ["New Oz buffer"          oz-new-buffer t]
              "-----"
	     ["Refresh buffer"         oz-prettyprint t]
	     ["Region to printer"      oz-print-region t]
	     ["Buffer to printer"      oz-print-buffer t]
              "-----"
	     ["Include file"           oz-feed-file t]
	     ["Compile file"           oz-precompile-file t]
              "-----"
	     ["Show/hide compiler"     oz-toggle-compiler-window t]
	     ["Show/hide machine"      oz-toggle-machine-window t]
	     ["Show/hide errors"       oz-toggle-errors t]
             ["Show Documentation ..." oz-doc t]
	    "-----"
	     ["Start Oz" run-oz t]
	     ["Halt Oz"  halt-oz t]
	     )
	    ("Font"
	     ["Small"      oz-small-font      t]
	     ["Default"    oz-default-font     t]
	     ["Large"      oz-large-font      t]
	     ["Very Large" oz-very-large-font t]
	     )
	     )))

  ;; else FSF Emacs 19
  (oz-make-menu 'Oz
	     '( (Feed\ buffer         . oz-feed-buffer)
		(Feed\ region         . oz-feed-region)
		(Feed\ line           . oz-feed-line)
		(Next\ Oz\ buffer     . oz-next-buffer)
		(Previous\ Oz\ buffer . oz-previous-buffer)
		(New\ Oz\ buffer      . oz-new-buffer)
		(Refresh\ buffer      . oz-prettyprint)
		(Region\ to\ printer  . oz-print-region)
		(Buffer\ to\ printer  . oz-print-buffer)
		(Include\ file        . oz-feed-file)
		(Compile\ file        . oz-precompile-file)
		(Show/hide\ compiler  . oz-toggle-compiler-window)
		(Show/hide\ machine   . oz-toggle-machine-window)
		(Show/hide\ errors    . oz-toggle-errors)
		(Show\ Documentation  . oz-doc)
		(Start\ Oz            . run-oz)
		(Halt\ Oz             . halt-oz)
		)))
  
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
  (oz-default-font)
  (if lucid-emacs
      (set-menubar oz-menubar))
;      (set-buffer-menubar oz-menubar))
  (run-hooks 'oz-mode-hook))

(defun run-oz ()
  "Run an inferior Oz process, input and output via buffer *Oz Compiler*."
  (interactive)
  (if (get-process "Oz Compiler")
      (error "Oz already running")
    (start-oz-process)
    (oz-new-buffer)))


(defun ensure-oz-process ()
  (start-oz-process))


(defun start-oz-process()
  (or (get-process "Oz Compiler")
      (let ((file (oz-make-temp-name "/tmp/ozsock")))
	(oz-set-state 'oz-compiler-state "booting...")
        (make-comint "Oz Compiler" oz-compiler nil "-S" file)

	(if (get-buffer "*Oz Machine*")
	    (kill-buffer "*Oz Machine*"))
	(oz-set-state 'oz-machine-state "booting...")
	(make-comint "Oz Machine" oz-machine nil "-S" file)

        (save-excursion
          (switch-to-buffer "*Oz Compiler*")
          (oz-input-mode))
        (set-process-filter  (get-process "Oz Compiler") 'oz-compiler-filter)
        (set-process-filter  (get-process "Oz Machine") 'oz-machine-filter)

	;; make sure buffer exists
	(get-buffer-create "*Oz Compiler*")
	
	(set-buffer (process-buffer (start-oz-process)))
	(oz-input-mode)
	(set-process-filter (get-process "Oz Compiler") 'oz-compiler-filter)
	(set-process-filter (get-process "Oz Machine")  'oz-machine-filter)

	(bury-buffer "*Oz Machine*")
	(bury-buffer "*Oz Compiler*")
	(oz-hide-errors)
	(get-process "Oz Compiler"))))


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
      (oz-feed-file file))
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
     (let (beg end)
       (beginning-of-line)
       (setq beg (point))
       (end-of-line)
       (oz-feed-region beg (point)))))


(defun oz-feed-file(file)
  (interactive "FInclude file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!include '" file "'\n"))) 

(defun oz-load-file(file)
  (interactive "FLoad file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!load '" file "'\n"))) 

(defun oz-precompile-file(file)
  (interactive "FPrecompile file: ")
  (oz-hide-errors)
  (oz-send-string (concat "!precompile '" file "'\n"))) 

(defun oz-hide-errors()
  (interactive)
  (if (get-buffer "*Oz Errors*")
      (delete-windows-on "*Oz Errors*")))



(defun oz-toggle-compiler-window()
  (interactive)
  (oz-toggle-window "*Oz Compiler*"))


(defun oz-toggle-errors()
  (interactive)
  (oz-toggle-window "*Oz Errors*"))


(defun oz-toggle-machine-window()
  (interactive)
  (oz-toggle-window "*Oz Machine*"))


(defun oz-toggle-window(buffername)
  (if (get-buffer buffername)
      (if (get-buffer-window buffername t)
	  (delete-windows-on buffername)
	(oz-show-buffer (get-buffer buffername)))))



(defun halt-oz()
  (interactive)
  (oz-send-string "!halt \n"))




(defun oz-send-string(string)
  (ensure-oz-process)
  (process-send-string "Oz Compiler" string)
  (process-send-eof "Oz Compiler"))


(defun oz-show-buffer (buffer)
  (let* ((win-to-split (or (get-buffer-window "*Oz Machine*")
			   (get-buffer-window "*Oz Compiler*")
			   (get-buffer-window "*Oz Errors*")
			   (selected-window)))
	 (old-win (selected-window))
	 (win (or (get-buffer-window buffer)
		  (split-window win-to-split))))
    (select-window win)
    (set-window-buffer win buffer)
    (goto-char (point-max))
    (select-window old-win))
  (bury-buffer buffer))

(defun oz-scroll-to-end(buf)
  (let ((win (get-buffer-window buf t)))
    (if win
	(save-excursion
	  (select-window win)
	  (goto-char (point-max))))))


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
		   "handle" "exists" "case" "begin" "process" "not"
		   "seq")))

(defconst oz-end-pattern
      (oz-make-keywords-for-match '("end" "fi" "ro")))

(defconst oz-middle-pattern 
      (concat (oz-make-keywords-for-match
	       '("in" "then" "else" "elseif" "by" "of" "wait"))
	      "\\|" "\\[\\]"))

(defconst oz-key-pattern
      (concat oz-begin-pattern "\\|" oz-middle-pattern "\\|" oz-end-pattern))

(defconst oz-left-pattern "[[({]")
(defconst oz-right-pattern "[])}]")

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
	((looking-at "\\<trigger\\>")
	 (if (search-backward "^" 0 t)
	     (current-column)
	   (error "no matching ^ for trigger")))
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
;  (debug)
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
	    ((looking-at "\\<proc\\>\\|\\<pred\\>")
	     (beginning-of-line)
	     (skip-chars-forward " \t")
	     (+ (current-column) oz-indent-chars))
	    ((looking-at "\\<meth\\>\\|\\<class\\>\\|\\<create\\>")
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
		((and (looking-at "\\<proc\\>\\|\\<pred\\>") (= n 0))
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
  (map-extents '(lambda(ext unused)
		  (delete-extent ext))
	       nil from to))


(defun oz-change-match-face (face beg end)
  (if lucid-emacs
      (set-extent-face (make-extent beg end) face)
    (overlay-put (make-extent beg end) 'face face)))


(defconst ozKeywords
   (concat
    (oz-make-keywords-for-match
     '(
       "pred" "proc" "true" "false" "local" "begin" "end"
       "in" "not" "process" "det" "if" "then" "else" "elseif" 
       "fi" "or" "ro" "meth" "create" "class" "from" "with" 
       "handle" "by" "exists" "on" "case" "of"
       "seq" "wait" "as" "div" "mod" "self"
       ))
    "\\|\\.\\|\\[\\]\\|#\\|!\\|\\^\\|:\\|\\@"
    ))



(if (not lucid-emacs)
    (defalias 'map-extents 'map-overlays))


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
    (while (re-search-forward "%\\|/\\*" end t)
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
  (oz-fontify-line))


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

(defvar oz-errors-found nil)

(defun oz-filter (proc string state-string)
  (let ((newbuf (process-buffer proc))
	(old-win (selected-window))
	 help-string old-point
	 match-start match-end)
    (save-excursion
      (set-buffer newbuf)
      ;; Insert the text, moving the process-marker.
      (goto-char (point-max))
      (setq old-point (point))
      (insert string)

      (oz-scroll-to-end newbuf)

      ;; show status messages of compiler in mini buffer
      (setq help-string string)

      ;; only display the last status message
      (while (setq match-start (string-match oz-status-string help-string))
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
    
    (select-window old-win)

    ;; error output
    (if (or oz-errors-found (string-match oz-error-string string))   ; contains errors ?
	(progn
	  (setq oz-errors-found t)
	  (oz-show-error string)))

    ;; reset error output if we have another message prefix than error
    (if (and (not (string-match oz-error-string string))
	     (string-match oz-escape-chars string))
	(setq oz-errors-found nil))))


(defun oz-show-error(string)
  (let ((buf (get-buffer-create "*Oz Errors*"))
	old-point)
    (save-excursion
      (setq buf (get-buffer-create "*Oz Errors*"))
      (set-buffer buf)
      ; if buffer is not visible then clear it out
      (if (not (get-buffer-window buf))
	  (delete-region (point-min) (point-max)))
      (setq old-point (point-max))
      (goto-char old-point)
      (insert string)
    
      ;; remove other than error messages
      (goto-char old-point)
      (while (search-forward-regexp 
	      (concat oz-error-string "\\|" oz-status-string ".*\n") nil t)
	(replace-match "" nil t)))

    (oz-show-buffer buf)))


(add-hook 'find-file-hooks 'oz-fontify-buffer)
;(add-hook 'write-file-hooks 'oz-fontify-buffer)


(defun oz-input-mode ()
  "Major mode for interacting with an inferior Oz process.

The following commands are available:
\\{oz-input-mode-map}

Commands:

Meta-Return sends current input.
\\[oz-delchar-or-maybe-eof-or-dynamic-complete] behavs as in \"ile\": either send EOF, delete character or list completions.
\\[comint-kill-input] and \\[backward-kill-word] are kill commands, imitating normal Unix input editing.
\\[comint-interrupt-subjob] interrupts the shell or its current subjob if any.
\\[comint-stop-subjob] stops, likewise. \\[comint-quit-subjob] sends quit signal, likewise."  
  (interactive)
  (comint-mode)
  (setq comint-prompt-regexp "^>+ *")
  (setq major-mode 'oz-input-mode)
  (setq mode-name "Oz Input")
  (cond (t;;(not oz-input-mode-map)
         (setq oz-input-mode-map (copy-keymap comint-mode-map))
         (define-key oz-input-mode-map "\C-m" 'newline)
         (define-key oz-input-mode-map "\C-d" 
	   'oz-delchar-or-maybe-eof-or-dynamic-complete)
         (define-key oz-input-mode-map "\M-\C-m" 'oz-send-input)
         (define-key oz-input-mode-map "\t" 'comint-dynamic-complete)
         (define-key oz-input-mode-map "\M-?"
                     'comint-dynamic-list-completions)))
  (use-local-map oz-input-mode-map)
  (make-local-variable 'scroll-step)
  (setq scroll-step 1)
  (run-hooks 'oz-input-mode-hook))

(defun oz-send-input()
  (interactive)
  (comint-send-input)
  (process-send-eof))

(defun oz-delchar-or-maybe-eof-or-dynamic-complete ()
  (interactive)
  (if (eobp)
      (if (bolp)
	  (oz-send-input)
	  (comint-dynamic-list-completions))
      (delete-char 1)))


  
(defun oz-print-buffer()
  "Print buffer."
  (interactive)
  (oz-print-region (point-min) (point-max)))

(defun oz-print-region(start end)
  "Print region."
  (interactive "r")
  (shell-command-on-region start end "oz2lpr -"))
    



(defvar oz-temp-counter 0)

(defun oz-make-temp-name(name)
  (setq oz-temp-counter (+ 1 oz-temp-counter))
  (format "%s%d" (make-temp-name name) oz-temp-counter))
