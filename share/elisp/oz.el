;; --------------------------------------------------------------------------
;; OZ-Mode
;; credits to mm and rs
;; --------------------------------------------------------------------------
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
;;  deleted some keywords for highlighting, fixed error
;; --------------------------------------------------------------------------


(require 'comint)

(defvar lucid-emacs
  (if (string= (substring emacs-version 0 2) "19")
      t
    nil
    )
  "Use Lucid-Emacs functions for highlighting for example")

(if lucid-emacs
    (load "blink-paren")
)

(random t)

(defvar oz-indent-chars 3
"*Indentation of Oz statements with respect to containing block.")

(defvar oz-input-to-oz nil 
"Defines wheter direkt input to oz is allowed")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)
(defvar oz-mode-map (make-sparse-keymap))
(defvar oz-input-mode-map nil)

(defvar oz-system "oz.compiler.csh"
  "Oz system used by run-oz")
(defvar oz-machine "oz.machine.csh"
  "Oz machine used by run-oz")

(defvar oz-doc-dir "/usr/share/gs/soft/oz/doc/"
  "The default doc directory")

(defvar oz-doc-file "quick.dvi"
  "The default doc file")

(defvar oz-preview "xdvi"
  "The previewer for doc files")

(if (not lucid-emacs)
    (defun abs(x)
      (if (< x 0) (- 0 x) x)
      ))


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
  (define-key map "\M-u"    'oz-clear-buffer)
  (define-key map "\M-c"    'oz-clone-buffer)
  (define-key map "\M-e"    'oz-toggle-compiler-window)
  (define-key map "\M-z"    'oz-prettyprint))

(oz-mode-commands oz-mode-map)

;;; OZ MODE
(if lucid-emacs
(defvar oz-menubar 
  (append default-menubar
	  '(("Oz"     
	     ["Feed buffer"          oz-feed-buffer t]
	     ["Feed region"          oz-feed-region t]
	     ["Feed line"            oz-feed-line t]
              "-----"
	     ["Next Oz buffer"       oz-next-buffer t]
	     ["Previous Oz buffer"   oz-previous-buffer t]
	     ["New Oz buffer"        oz-new-buffer t]
	     ["Clone buffer"         oz-clone-buffer t]
	     ["Clear buffer"         oz-clear-buffer t]
	     ["Pretty print buffer"  oz-prettyprint t]
	     ["Include file into Oz" oz-feed-file t]
	     ["Show/Hide errors"     oz-toggle-compiler-window t]
             ["Show Documentation ..." oz-doc t]
              "-----"
	     ["Start Oz" run-oz t]
	     ["Halt Oz"  halt-oz t]
	     ))))
)

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
(if lucid-emacs
  (set-buffer-menubar oz-menubar)
)
  (run-hooks 'oz-mode-hook))

(defun run-oz ()
  "Run an inferior Oz process, input and output via buffer *Oz Compiler*."
  (interactive)
  (let ((cur (current-buffer))
	(proc-buff (get-buffer-process "*Oz Compiler*")))
    (if proc-buff
	(error "Oz already running")
      (setq proc-buff (process-buffer (start-oz-process)))
      (set-buffer proc-buff)
      (oz-clear-buffer)
      (oz-input-mode)
      (if (not oz-input-to-oz) 
	  (toggle-read-only 1))
      (set-process-filter (get-process "Oz Compiler") 'oz-filter)
;      (switch-to-buffer cur)
      (oz-new-buffer)
      (oz-hide-errors))))


(defun ensure-oz-process ()
  (start-oz-process))


(defun start-oz-process()
  (or (get-process "Oz Compiler")
      (let ((port (format "%d" (+ 9000 (random 1000)))))
	(message "Starting Oz")
	(start-oz-machine port)
	(message "Starting Compiler")
	(make-comint "Oz Compiler" oz-system nil port)
	(save-excursion
	  (switch-to-buffer "*Oz Compiler*")
	  (oz-input-mode))
	(set-process-filter  (get-process "Oz Compiler") 'oz-filter)
	(get-process "Oz Compiler"))))

(defun start-oz-machine(port)
  (message (format "Starting Machine (port = %s)" port))
  (start-process "Oz Machine" nil oz-machine port)
;  (make-comint "Oz Machine" oz-machine nil port)
;; wait a little bit until machine has bind it's socket
  (sleep-for 10)
)

(defun oz-doc ()
  (interactive)
  (let ((name (read-file-name
	       (format "Preview File [%s]: " oz-doc-file)
	       oz-doc-dir (concat oz-doc-dir oz-doc-file) t)))
    (if (file-exists-p name)
	(start-process "OZ Doc" "*Preview*" oz-preview name)
      (error "file %s doesn't exists" name)
      )
    )
  )


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
  "Consults the region"
   (interactive "r")
   (oz-hide-errors)
   (let ((contents (buffer-substring start end)))
    (oz-send-string (concat contents "\n"))))

(defun oz-feed-line ()
  "Consults the region"
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

(defun oz-hide-errors()
  (interactive)
  (if (get-buffer "*Oz Compiler*")
      (delete-windows-on "*Oz Compiler*")))


(defun oz-toggle-compiler-window()
  (interactive)
  (if (get-buffer "*Oz Compiler*")
      (if (get-buffer-window "*Oz Compiler*")
	  (oz-hide-errors)
	(oz-show-compiler (get-buffer "*Oz Compiler*")))
    (error "Oz not yet runnning")))


(defun halt-oz()
  (interactive)
  (oz-send-string "!halt \n"))

(defun oz-clear-buffer(&optional beg end)
  (interactive)
  (oz-hide-errors)
  (oz-clear-buffer0 beg end))

(defun oz-clear-buffer0(&optional beg end)
  (interactive)
  (if (buffer-file-name)
      (error "Buffer is associated with a file. Use kill-buffer.")
    (if (not oz-input-to-oz) 
	(let ((oldro  buffer-read-only))
	  (toggle-read-only 0)
	  (delete-region (or beg (point-min)) (or end (point-max)))
	  (if oldro (toggle-read-only oldro))))))




(defun oz-send-string(string)
  (ensure-oz-process)
  (save-excursion
    (switch-to-buffer (get-buffer "*Oz Compiler*"))
    (oz-clear-buffer0 (point-min) (point-max)))
  (process-send-string "Oz Compiler" string)
  (process-send-eof "Oz Compiler"))


(defun oz-show-compiler (buffer)
  (let ((cur (current-buffer)))
    (pop-to-buffer buffer)
    (goto-char (point-max))
    (pop-to-buffer cur)))



(defun find-buffer-other-window ()
  (interactive)
  (let ((cur (current-buffer)))
    (x-new-screen)
    (switch-to-buffer cur)))

(defun switch-to-buffer-new-screen (buffer)
  (interactive "BSwitch to buffer in other screen: ")
  (let ((win (get-buffer-window buffer t)))
    (if win
	(progn
	  ;; (select-window win)
	  (make-screen-visible (window-screen win))
	  (raise-screen  (window-screen win)))
      (x-new-screen)
      (switch-to-buffer buffer))))


(defun oz-new-buffer()
  (interactive)
  (oz-hide-errors)
  (let ((buf (generate-new-buffer "Oz")))
    (switch-to-buffer buf)
    (oz-mode)))


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
      (error "No other oz-buffer")))
)


(defun oz-clone-buffer()
  (interactive)
  (let ((cur (current-buffer)))
    (oz-new-buffer)
    (insert-buffer cur)))

  
(defun oz-make-keywords-for-match(args)
  (if (null args) 
      (error "oz-make-keywords-for-match: bad arglist")
    (concat "\\<" 
	    (car args) 
	    "\\>"
	    (if (null (cdr args))
		""
	      (concat "\\|" (oz-make-keywords-for-match (cdr args)))
	      )
	    )
    )
  )

(defconst oz-begin-pattern
      (oz-make-keywords-for-match 
	         '("local" "class" "meth" "create" "or" "if" "pred" "proc"
		   "global" "handle" "exists" "case" "begin" "process" "not"
		   "trigger")))

(defconst oz-end-pattern
      (oz-make-keywords-for-match '("end" "fi" "ro"))
      )

(defconst oz-middle-pattern 
      (concat (oz-make-keywords-for-match '("in" "then" "else" "by" "of"))
	      "\\|" "\\[\\]"))

(defconst oz-key-pattern
      (concat oz-begin-pattern "\\|" oz-middle-pattern "\\|" oz-end-pattern)
      )

(defconst oz-left-pattern "[[({]")
(defconst oz-right-pattern "[])}]")

(defun oz-indent-buffer()
  (interactive)
  (goto-char 0)
  (while (< (point) (point-max))
    (oz-indent-line t)
    (forward-line 1)
    )
  )

(defun oz-indent-region(b e)
  (interactive "r")
  (let ((end (copy-marker e)))
    (goto-char b)
    (while (< (point) end)
      (oz-indent-line t)
      (forward-line 1)
      )
    )
  )

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
		   (indent-to col)
		   )
		  )
	    )
	  )
      (if (is-first-in-row)
	  (skip-chars-forward " \t")
	)
      (setq case-fold-search search-mode)
      )
    )
  )

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
      nil)
    )
  )

(defun is-last-in-row()
  (if (looking-at "[ \t]*$")
      t
    nil)
  )

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
		(t (setq n (+ n 1)))
		)
	(error "no matching left paren")
	)
      )
    )
  (current-column)
  )

(defun oz-search-matching-begin(&optional search-exists)
  (let ((s t)
	(n 0)
	)
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
		   (setq n (- n 1))
		   )
		 )
		((and (looking-at "\\<else\\>\\|\\[\\]") (= n 0))
		 (setq s nil))
		((looking-at oz-middle-pattern)
		 ;; 'then' '[]'
		 t
		 )
		(t
		 ;; 'end'
		 (setq n (+ n 1)))
		)
	(error "no matching begin token")
	)
      (setq search-exists nil)
      )
    )
  (current-column)
  )

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

(defun delete-extents ()
  (map-extents '(lambda(ext unused)
		  (delete-extent ext))))

(defun change-match-face (face)
  (set-extent-face (make-extent (match-beginning 0) (match-end 0))
		   face))

(defconst ozKeywords
   (concat
    (oz-make-keywords-for-match
     '(
       "pred" "proc" "true" "false" "local" "begin" "end"
       "in" "not" "process" "det" "if" "then" "else" 
       "fi" "or" "ro" "meth" "create" "class" "from" "with" 
       "handle" "by" "exists" "on" "case" "of" "trigger"
       "seq" "wait"
       ))
    "\\|\\.\\|\\[\\]\\|#\\|!\\|\\^\\|:"
    ))


(defun oz-highlight-keywords()
  (let ((pmax (point-max)))
    (while (re-search-forward ozKeywords pmax t)
      (change-match-face 'bold))))

(defun oz-highlight-comments()
  (let ((pmax (point-max)))
    (while (re-search-forward "%.*$\\|/\\*[\001-\175\n]*\\*/" pmax t)
      ;; delete extents within comment
      (map-extents '(lambda(ext unused)
		      (delete-extent ext))
		   nil
		   (match-beginning 0)
		   (match-end 0))
      (change-match-face 'italic))))


(defun oz-highlight-buffer()
  (interactive)
  (if (and lucid-emacs (eq major-mode 'oz-mode))
      (let ((old-case case-fold-search))
	;; search case dependent:
	(setq case-fold-search nil)
	(unwind-protect
	    (save-excursion
	      (goto-char 0)
	      (delete-extents)
	      (oz-highlight-keywords)
	      (goto-char 0)
	      (oz-highlight-comments))
	  (setq case-fold-search old-case)))))


(defun oz-prettyprint(&optional arg)
  (interactive "P")
  (oz-hide-errors)
  (recenter arg)
  (oz-highlight-buffer))



(defun oz-filter (proc string)
  (let ((old-buffer (current-buffer))
	newbuf ppp
	match)
    (setq newbuf (process-buffer proc))
    (set-buffer newbuf)
    (if (null oz-input-to-oz) (toggle-read-only 0))
      ;; Insert the text, moving the process-marker.
    (goto-char (process-mark proc))
    (save-excursion
      (insert string)

      ;; show status messages of compiler in mini buffer
      (if (eq 0
	      (string-match "^ *----" string))
	  (message (substring string 4)))

      ;; 
      (if (null oz-input-to-oz)
	  (progn 
	    (toggle-read-only 0)

	    (goto-char (point-min))
	    (while (search-forward "OZ> " nil t)
	      (replace-match "" nil t))

	    (toggle-read-only 1)))

      (if (string-match "\\*\\*\\*" string)
	  (progn (oz-show-compiler
		  (process-buffer (get-process "Oz Compiler")))
		 (message ""))))

    (goto-char (point-max))
    (set-marker (process-mark proc) (point))
    (pop-to-buffer old-buffer)))


(if lucid-emacs
(add-hook 'find-file-hooks 'oz-highlight-buffer)
)

;(add-hook 'write-file-hooks 'oz-highlight-buffer)

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
         (setq oz-input-mode-map (full-copy-sparse-keymap comint-mode-map))
         (define-key oz-input-mode-map "\C-m" 'newline)
	 (define-key oz-input-mode-map "\M-e" 'oz-toggle-compiler-window)
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


(defun oz-convert-one-application()
  "Convert one next application after point of the form 
   \"Variable(...)\" into \"{Variable ...}\""
  (interactive)
  (let ((old case-fold-search))
    (setq case-fold-search nil) ;; search case sensitive
    (if (not (re-search-forward "\\<[A-Z_][a-zA-Z_0-9]*(" (point-max) t))
	(progn (setq case-fold-search old)
	       nil)
      (let ((start (match-beginning 0))
	    (end (1- (match-end 0))))
	;; start points to the start of the predicates name
	;; end points to "("
	(goto-char end)  ;; now we are at the "("
	(forward-sexp)   ;; search corresponding ")"
	(delete-char -1) ;; we delete the ")" (it is to the left of point)
	(insert-string "}")
	(goto-char end)  ;; back to the "("
	(delete-char 1)
	;; "Bla()" should go to "{Bla}" and not "{Bla }"
	(if (not (looking-at "}")) 
	    (insert-string " "))
	(goto-char start)
	(insert-string "{")
	(setq case-fold-search old)
	t))))

(defun oz-remove-comma()
  "replace the next comma by <blank>"
  (interactive)
  (if (not (re-search-forward "," (point-max) t))
      nil
      (progn (delete-char -1)
       (insert-string " ")
       t)))
  

(defun oz-replace-point()
  "replace the next comma by <blank>"
  (interactive)
  (if (not (re-search-forward "[.][^0-9]" (point-max) t))
      nil
    (progn (backward-char 1) 
	   (delete-char -1)
	   (insert-string "|")
	   t)))
  
(defun oz-replace-opensquare()
  "replace the next [ by .("
  (interactive)
  (if (not (re-search-forward "[[][^]]" (point-max) t))
      nil
    (progn (backward-char 1) (delete-char -1) (insert-string ".(") t)))

  
(defun oz-replace-closesquare()
  "replace the next comma by <blank>"
  (interactive)
  (if (not (re-search-forward "[^[][]]" (point-max) t))
      nil
      (progn (delete-char -1)
       (insert-string ")") t)))
  

(global-set-key [(f10)] 'oz-convert-one-application)

(defun oz-convert-buffer()
  (interactive)
  (save-excursion
    (goto-char (point-min))
    (while (oz-convert-one-application))
    (goto-char (point-min))
    (while (oz-remove-comma))
    (goto-char (point-min))
    (while (oz-replace-point))
    (goto-char (point-min))
    (while (oz-replace-opensquare))
    (goto-char (point-min))
    (while (oz-replace-closesquare))

))






    
;(debug-on-entry 'oz-convert)
