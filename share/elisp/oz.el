;; Major mode for editing Oz, and for running Oz under Emacs
;; Copyright (C) 1993 DFKI GmbH
;; Author: Ralf Scheidhauer and Michael Mehl ([scheidhr|mehl]@dfki.uni-sb.de)
;; $Id$

;; BUGS
;; - `/*' ... `*/' style comments are ignored for purposes of indentation.
;;   (Nesting and line breaks are problematic.)
;; - Line breaks inside strings, quotes or backquote variables
;;   are not allowed for auto-indent.
;; - 10thread is not recognized as a keyword as it should be.
;; - An ampersand as the last character in a string or before a
;;   backslash-escaped double quote in a string messes up fontification.
;; - The use of non-escaped double quotes in Oz-Gump regular expression
;;   tokens confuses fontification.
;; - Oz-Gump regular expressions are not ignored for indentation.
;; - Some indentation rules do not work correctly with infix operators, e.g.:
;;      feat
;;         f:
;;            5 +
;;         7
;;   The 7 should be underneath the 5.

(require 'comint)
(require 'compile)

;;------------------------------------------------------------
;; Global Effects
;;------------------------------------------------------------

(or (member ".ozo" completion-ignored-extensions)
    (setq completion-ignored-extensions
	  (append '(".ozo" ".ozs")
		  completion-ignored-extensions)))

;; automatically switch into Oz-Mode when loading
;; files ending in ".oz"
(or (assoc "\\.oz$" auto-mode-alist)
    (setq auto-mode-alist
	  (append '(("/\\.ozrc$" . oz-mode)
		    ("\\.oz$" . oz-mode)
		    ("\\.ozm$" . ozm-mode)
		    ("\\.ozg$" . oz-gump-mode))
		  auto-mode-alist)))


;;------------------------------------------------------------
;; GNU and Lucid Emacsen Support
;;------------------------------------------------------------

(defvar oz-gnu-emacs
  (string-match "\\`[0-9]+\\(\\.[0-9]+\\)*\\'" emacs-version))
(defvar oz-lucid-emacs
  (string-match "\\<XEmacs\\>\\|\\<Lucid\\>" emacs-version))


;;------------------------------------------------------------
;; win32 Support
;;------------------------------------------------------------
;; primary change: emulator is started by compiler,
;; so its output goes into the compiler buffer

(defvar oz-win32 nil)
(setq oz-win32 (eq system-type 'windows-nt))


;;------------------------------------------------------------
;; Variables/Initialization
;;------------------------------------------------------------

(defvar oz-use-new-compiler (getenv "OZUSENEWCOMPILER")
  "*If non-nil, use the new Oz Compiler.
This has the effect of not opening the *Oz Compiler* buffer and feeding
everything into the *Oz Emulator* buffer.")

(defvar oz-using-new-compiler nil
  "If non-nil, indicates that the new Oz Compiler is currently running.")

(defvar oz-other-map nil
  "If non-nil, choose alternate key bindings.
If nil, use default key bindings.")

(defvar oz-read-emulator-output nil
  "Non-nil iff output currently comes from the emulator.")

(defvar oz-gdb-autostart t
  "*If non-nil, start emulator immediately in gdb mode.
If nil, you have the possibility to first set breakpoints and only
run the emulator when you issue the command `run' to gdb.")

(defvar oz-auto-indent t
  "*If non-nil, automatically indent lines.")

(defvar oz-indent-chars 3
  "*Number of spaces Oz statements are indented wrt. containing block.")

(defvar oz-mode-syntax-table nil)
(defvar oz-mode-abbrev-table nil)

(defvar oz-mode-map (make-sparse-keymap)
  "Keymap used in oz-mode buffers.")
(defvar oz-compiler-buffer-map nil
  "Keymap used in *Oz Compiler* buffer.")
(defvar oz-emulator-buffer-map nil
  "Keymap used in *Oz Emulator* buffer.")

(defvar oz-emulator (concat (getenv "HOME") "/Oz/Emulator/oz.emulator.bin")
  "*Path to the Oz Emulator for gdb mode and for \\[oz-other].")

(defvar oz-new-compiler-url
  (concat "file:" (getenv "HOME") "/Oz/tools/compiler/Compiler.ozc")
  "*URL of the new Oz Compiler for gdb mode.")

(defvar oz-boot (concat (getenv "HOME") "/Oz/Compiler/backend/ozboot.ql")
  "*Path to the Oz Compiler boot file for \\[oz-other].")

(defvar oz-emulator-buffer "*Oz Emulator*"
  "Name of the Oz Emulator buffer.")

(defvar oz-compiler-buffer "*Oz Compiler*"
  "Name of the Oz Compiler buffer.")

(defvar oz-temp-buffer "*Oz Temp*"
  "Name of the Oz temporary buffer.")

(defvar oz-emulator-hook nil
  "If non-nil, hook used for starting the Oz Emulator.
This is set when gdb is active.")

(defvar OZ-HOME "/project/ps/soft/oz-devel/oz"
  "Directory where Oz is installed.
Only used as fallback if the environment variable OZHOME is not set.")

(defun oz-home ()
  "Return the path of the Oz installation directory."
  (let ((ret (getenv "OZHOME")))
    (if ret
	ret
      (message "OZHOME not set, using fallback: %s" OZ-HOME)
      (setenv "OZHOME" OZ-HOME)
      OZ-HOME)))

(defvar oz-doc-dir (concat (oz-home) "/doc/")
  "Directory containing the Oz documentation.")

(defvar oz-preview "xdvi"
  "*Viewer for Oz documentation files.")

(defvar oz-popup-on-error t
  "*If non-nil, pop up Compiler resp. Emulator buffer upon error.")

(defvar oz-error-string (format "%c" 17)
  "Regex to recognize error messages from Oz Compiler and Emulator.
Used for popping up the corresponding buffer.")

(defconst oz-remove-pattern
  (concat oz-error-string
	  "\\|" (char-to-string 18) "\\|" (char-to-string 19) "\\|
"
	  "\\|\\\\line.*% fromemacs\n")
  "Regex specifying what to remove from Compiler and Emulator output.
All strings matching this regular expression are removed.")

(defconst oz-bar-pattern
  "\'oz-bar \\([^ ]*\\) \\([^ ]*\\) \\([^ ]*\\)\'"
  "Regex for reading messages from the Oz debugger or profiler.")

(defconst oz-show-temp-pattern
  "\'oz-show-temp \\([^ ]*\\)\'"
  "Regex for reading messages from the Oz compiler.")

(defvar oz-want-font-lock t
  "*If non-nil, automatically enter font-lock-mode for oz-mode.")

(defvar oz-temp-counter 0
  "Internal counter for gensym.")

(defvar oz-gump-indentation nil
  "Non-nil iff Gump syntax is to be used for indentation.")


;;------------------------------------------------------------
;; Locating Errors (jd)
;;------------------------------------------------------------

(defvar oz-last-fed-buffer nil
  "Buffer from which the last region was fed; used to locate errors.")

(defvar oz-compiler-output-start nil
  "Where output of last compiler run began.")

(defvar oz-next-error-marker nil
  "Remembers place of last error message.")

(defvar oz-error-intro-pattern "\\(error\\|warning\\) \\*\\*\\*\\*\\*"
  "Regular expression for finding error messages.")

(defvar oz-error-pattern
  (concat "in "
	  "\\(file \"\\([^\"\n]+\\)\",? *\\)?"
	  "line \\([0-9]+\\)"
	  "\\(,? *column \\([0-9]+\\)\\)?")
  "Regular expression matching error coordinates.")


;;------------------------------------------------------------
;; Setting the Frame Title
;;------------------------------------------------------------
;; GNU Emacs supports frame-title as constant string
;; Lucid Emacs supports frame-title as format string (is better ...);
;;    see function mode-line-format

(defvar oz-change-title t
  "*If non-nil, change the title of the Emacs window while Oz is running.")

(defvar oz-old-frame-title
  (cond (oz-gnu-emacs
	 (cdr (assoc 'name (frame-parameters (car (visible-frame-list))))))
	(oz-lucid-emacs
	 frame-title-format))
  "Saved Emacs window title.")

(defvar oz-frame-title
  (concat "Oz Programming Interface (" oz-old-frame-title ")")
  "Format string for Emacs window title while Oz is running.")

(defun oz-set-title (frame-title)
  "Set the title of the Emacs window."
  (cond ((not oz-change-title) t)
	(oz-gnu-emacs
	 (mapcar '(lambda (scr)
		    (modify-frame-parameters
		     scr
		     (list (cons 'name frame-title))))
		 (visible-frame-list)))
	(oz-lucid-emacs
	 (setq frame-title-format frame-title))))


;;------------------------------------------------------------
;; Utilities
;;------------------------------------------------------------

(defun oz-make-temp-name (name)
  "gensym implementation."
  (setq oz-temp-counter (1+ oz-temp-counter))
  (format "%s%d" (make-temp-name name) oz-temp-counter))

(defun oz-line-region (arg)
  "Return starting and ending positions of ARG lines surrounding point.
Positions are returned as a pair ( START . END )."
  (save-excursion
    (let (start end)
      (cond ((> arg 0)
	     (beginning-of-line)
	     (setq start (point))
	     (forward-line (1- arg))
	     (end-of-line)
	     (setq end (point)))
	    ((= arg 0)
	     (setq start (point))
	     (setq end (point)))
	    ((< arg 0)
	     (end-of-line)
	     (setq end (point))
	     (forward-line arg)
	     (setq start (point))))
      (cons start end))))

(defun oz-paragraph-region (arg)
  "Return starting and ending positions of ARG paragraphs surrounding point.
Positions are returned as a pair ( START . END )."
  (save-excursion
    (let (start end)
      (cond ((> arg 0)
	     (backward-paragraph 1)
	     (setq start (point))
	     (forward-paragraph arg)
	     (setq end (point)))
	    ((= arg 0)
	     (setq start (point))
	     (setq end (point)))
	    ((< arg 0)
	     (forward-paragraph (1- arg))
	     (setq start (point))
	     (backward-paragraph (1- arg))
	     (setq end (point))))
      (cons start end))))

(defun oz-get-region (start end)
  (save-excursion
    (goto-char start)
    (skip-chars-forward " \t\n")
    (if (/= (count-lines start (point)) 0)
	(progn
	  (beginning-of-line)
	  (setq start (point))))
    (goto-char end)
    (skip-chars-backward " \t\n")
    (setq end (point)))
  (if (not (buffer-file-name))
      (setq oz-last-fed-buffer (current-buffer)))
  (concat "\\line " (1+ (count-lines 1 start))
	  " '" (or (buffer-file-name) "nofile") "' % fromemacs\n"
	  (buffer-substring start end)))


;;------------------------------------------------------------
;; Menus
;;------------------------------------------------------------
;; GNU Emacs: a menubar is a usual key sequence with prefix "menu-bar"
;; Lucid Emacs: a menubar is a new datastructure
;;    (see function set-buffer-menubar)

(defvar oz-menubar nil
  "Oz Menubar for Lucid Emacs.")

(defun oz-make-menu (list)
  (cond (oz-gnu-emacs
	 (oz-make-menu-gnu oz-mode-map
			   (list (cons "menu-bar" (cons nil list)))))
	(oz-lucid-emacs
	 (setq oz-menubar (oz-make-menu-lucid list)))))

(defun oz-make-menu-gnu (map list)
  (if list
      (progn
	(let* ((entry (car list))
	       (name (car entry))
	       (aname (intern name))
	       (command (car (cdr entry)))
	       (rest (cdr (cdr entry))))
	  (cond ((null rest)
		 (define-key map (vector (intern (oz-make-temp-name name)))
		   (cons name nil)))
		((null command)
		 (let ((newmap (make-sparse-keymap name)))
		   (define-key map (vector aname)
		     (cons name newmap))
		   (oz-make-menu-gnu newmap (reverse rest))))
		(t
		 (define-key map (vector aname) (cons name command))
		 (put command 'menu-enable (car rest)))))
	(oz-make-menu-gnu map (cdr list)))))

(defun oz-make-menu-lucid (list)
  (if (null list)
      nil
    (cons
     (let* ((entry (car list))
	    (name (car entry))
	    (command (car (cdr entry)))
	    (rest (cdr (cdr entry))))
       (cond ((null rest)
	      (vector name nil nil))
	     ((null command)
	      (cons name (oz-make-menu-lucid rest)))
	     (t
	      (vector name command (car rest)))))
     (oz-make-menu-lucid (cdr list)))))

(defvar oz-menu
 '(("Oz" nil
    ("Feed Buffer"    oz-feed-buffer t)
    ("Feed Region"    oz-feed-region (mark t))
    ("Feed Line"      oz-feed-line t)
    ("Feed Paragraph" oz-feed-paragraph t)
    ("Feed File"      oz-feed-file t)
    ("Compile File"   oz-precompile-file (not oz-using-new-compiler))
    ("-----")
    ("Find" nil
     ("Documentation Demo" oz-find-docdemo-file t)
     ("Other Demo"         oz-find-demo-file t)
     ("Modules File"       oz-find-modules-file t)
     )
    ("Print" nil
     ("Region" ps-print-region-with-faces (mark t))
     ("Buffer" ps-print-buffer-with-faces t)
     )
    ("Core Syntax" nil
     ("Buffer"    oz-to-coresyntax-buffer t)
     ("Region"    oz-to-coresyntax-region (mark t))
     ("Line"      oz-to-coresyntax-line t)
     ("Paragraph" oz-to-coresyntax-paragraph t)
     )
    ("Emulator Code" nil
     ("Buffer"    oz-to-emulatorcode-buffer t)
     ("Region"    oz-to-emulatorcode-region (mark t))
     ("Line"      oz-to-emulatorcode-line t)
     ("Paragraph" oz-to-emulatorcode-paragraph t)
     )
    ("Indent" nil
     ("Line"      oz-indent-line t)
     ("Region"    oz-indent-region (mark t))
     ("Buffer"    oz-indent-buffer t)
     )
    ("Comment" nil
     ("Comment Region"   oz-comment-region (mark t))
     ("Uncomment Region" oz-uncomment-region (mark t))
     )
    ("Browse" nil
     ("Region" oz-feed-region-browse (mark t))
     ("Line"   oz-feed-line-browse t)
     )
    ("Panel"          oz-view-panel t)
    ("Debugger"       oz-debug-start t)
    ("Profiler"       oz-profiler-start t)
    ("Compiler Panel" oz-compiler-panel oz-using-new-compiler)
    ("-----")
    ("Next Oz Buffer"     oz-next-buffer t)
    ("Previous Oz Buffer" oz-previous-buffer t)
    ("New Oz Buffer"      oz-new-buffer t)
    ("Fontify Buffer"     oz-fontify t)
    ("Show/Hide" nil
     ("Compiler" oz-toggle-compiler t)
     ("Emulator" oz-toggle-emulator t)
     )
    ("-----")
    ("Start Oz" run-oz t)
    ("Halt Oz"  oz-halt t)
    ))
  "Contents of the Oz menu.")

(oz-make-menu oz-menu)


;;------------------------------------------------------------
;; Debugger Stuff
;;------------------------------------------------------------

;;------------------------------------------------------------
;; Starting and stopping the debugger

(defun oz-debug-start ()
  "Start the Oz debugger."
  (interactive)
  (oz-send-string "{Ozcar on}"))

(defun oz-debug-stop ()
  "Stop the Oz debugger."
  (interactive)
  (oz-send-string "{Ozcar off}"))

;;------------------------------------------------------------
;; Setting and deleting breakpoints

(defvar oz-breakpoint-error
  "Cannot set breakpoints in unsaved buffers")

(defun oz-breakpoint-key-set ()
  "Set breakpoint at current line."
  (interactive)
  (oz-breakpoint-key " true"))

(defun oz-breakpoint-key-delete ()
  "Delete breakpoint at current line."
  (interactive)
  (oz-breakpoint-key " false"))

(defun oz-breakpoint-key (flag)
  (if (buffer-file-name)
      (oz-send-string (concat "{Ozcar bpAt('" (buffer-file-name) "' "
			      (count-lines 1 (1+ (point)))
			      flag ")}"))
    (error oz-breakpoint-error)))

(defun oz-breakpoint-mouse-set (event)
  "Set breakpoint at line where mouse points to."
  (interactive "e")
  (oz-breakpoint-mouse event " true"))

(defun oz-breakpoint-mouse-delete (event)
  "Delete breakpoint at line where mouse points to."
  (interactive "e")
  (oz-breakpoint-mouse event " false"))

(defun oz-breakpoint-mouse (event flag)
  (mouse-minibuffer-check event)
  (save-window-excursion
    (select-window (posn-window (event-start event)))
    (if (buffer-file-name)
	(oz-send-string (concat "{Ozcar bpAt('" (buffer-file-name) "' "
				(count-lines 1
					     (posn-point (event-start event)))
				flag ")}"))
      (error oz-breakpoint-error))))

;;------------------------------------------------------------
;; Displaying the bar

(make-face 'bar-running)
(make-face 'bar-runnable)
(make-face 'bar-blocked)

(defun oz-set-face (face foreground background)
  (cond (oz-gnu-emacs
	 (modify-face face foreground background nil nil nil nil))
	(oz-lucid-emacs
	 (set-face-foreground face foreground)
	 (set-face-background face background))))

(let ((planes (if (eq window-system 'x) (x-display-planes) 1)))
  (oz-set-face 'bar-running      "white"
	       (if (eq planes 1) "black" "#b0b0b0"))
  (oz-set-face 'bar-runnable     "white"
	       (if (eq planes 1) "black" "#7070c0"))
  (oz-set-face 'bar-blocked      "white"
	       (if (eq planes 1) "black" "#d05050")))

(defvar oz-bar-overlay nil)

(defun oz-bar (file line state)
  "Display bar at given line, load file if necessary."
  (if (string-equal file "unchanged")
      (oz-bar-configure state)
    (let* ((last-nonmenu-event t)
	   (buffer (find-file-noselect file))
	   (window (display-buffer buffer))
	   start end oldpos)
      (save-excursion
	(set-buffer buffer)
	(save-restriction
	  (widen)
	  (setq oldpos (point))
	  (goto-line line) (setq start (point))
	  (forward-line 1) (setq end (point))

	  (or oz-bar-overlay
	      (setq oz-bar-overlay
		    (cond (oz-gnu-emacs
			   (make-overlay start end))
			  (oz-lucid-emacs
			   (make-extent start end)))))
	  (cond (oz-gnu-emacs
		 (move-overlay
		  oz-bar-overlay start end (current-buffer)))
		(oz-lucid-emacs
		 (set-extent-endpoints
		  oz-bar-overlay start end (current-buffer))))

	  (if (string-equal state "unchanged")
	      ()
	    (oz-bar-configure state)))

	(if (or (< start (window-start)) (> start (window-end)))
	    (progn
	      (widen)
	      (goto-char start))
	  (goto-char oldpos)))
      (set-window-point window start))))

(defun oz-bar-configure (state)
  "Change color of bar while not moving it."
  (let ((face (cond ((string-equal state "running")
		     'bar-running)
		    ((string-equal state "runnable")
		     'bar-runnable)
		    ((string-equal state "blocked")
		     'bar-blocked))))
    (cond (oz-gnu-emacs
	   (overlay-put oz-bar-overlay 'face face))
	  (oz-lucid-emacs
	   (set-extent-face oz-bar-overlay face)))))


;;------------------------------------------------------------
;; Profiler Stuff
;;------------------------------------------------------------

(defun oz-profiler-start ()
  "Start the profiler."
  (interactive)
  (oz-send-string "{Profiler on}"))

(defun oz-profiler-stop ()
  "Stop the profiler."
  (interactive)
  (oz-send-string "{Profiler off}"))


;;------------------------------------------------------------
;; Start/Stop Oz
;;------------------------------------------------------------

(defun run-oz (&optional toggle-new-compiler)
  "Run the Oz Compiler and Oz Emulator.
Handle input and output via buffers whose names are found in
variables oz-compiler-buffer and oz-emulator-buffer."
  (interactive "P")
  (oz-check-running
   t (if toggle-new-compiler (not oz-use-new-compiler) oz-use-new-compiler))
  (if (not (or (equal mode-name "Oz")
	       (equal mode-name "Oz-Gump")
	       (equal mode-name "Oz-Machine")))
      (oz-new-buffer))
  (oz-show-buffer (get-buffer oz-compiler-buffer)))

(defvar oz-halt-timeout 15
  "How long to wait in oz-halt after sending the directive `\\halt'.")

(defun oz-halt (force)
  "Halt Oz Compiler and Emulator.
If FORCE is nil, send the `\\halt' directive and wait for the processes
to terminate.  Waiting time is limited by variable `oz-halt-timeout';
after this delay, the processes are simply killed if still living.
If FORCE is non-nil, kill the processes immediately."
  (interactive "P")
  (message "Halting Oz ...")
  (cond ((get-buffer oz-temp-buffer)
	 (delete-windows-on oz-temp-buffer)
	 (kill-buffer oz-temp-buffer)))
  (cond (oz-bar-overlay
	 (cond (oz-gnu-emacs
		(delete-overlay oz-bar-overlay))
	       (oz-lucid-emacs
		(delete-extent oz-bar-overlay)))
	 (setq oz-bar-overlay nil)))
  (if (and (not force)
	   (get-buffer-process oz-compiler-buffer)
	   (or oz-win32 (get-buffer-process oz-emulator-buffer)))
      (let* ((i (* 2 oz-halt-timeout))
	     (cproc (get-buffer-process oz-compiler-buffer))
	     (eproc (if oz-win32
			cproc
		      (get-buffer-process oz-emulator-buffer))))
	(oz-send-string "\\halt ")
	(while (and (or (eq (process-status eproc) 'run)
			(eq (process-status cproc) 'run))
		    (> i 0))
	  (message "Halting Oz ... %s" i)
	  (sleep-for 1)
	  (setq i (1- i)))))
  (if (get-buffer-process oz-compiler-buffer)
      (delete-process oz-compiler-buffer))
  (if (get-buffer-process oz-emulator-buffer)
      (delete-process oz-emulator-buffer))
  (message "Oz halted.")
  (oz-set-title oz-old-frame-title))

(defun oz-check-running (start-flag use-new-compiler)
  (let ((running t))
    (cond (oz-using-new-compiler
	   (setq running (get-buffer-process oz-emulator-buffer)))
	  (oz-win32
	   (setq running (get-buffer-process oz-compiler-buffer)))
	  ((and (get-buffer-process oz-compiler-buffer)
		(not (get-buffer-process oz-emulator-buffer)))
	   (setq running nil)
	   (message "Emulator died.")
	   (delete-process oz-compiler-buffer))
	  ((and (not (get-buffer-process oz-compiler-buffer))
		(get-buffer-process oz-emulator-buffer))
	   (setq running nil)
	   (message "Compiler died.")
	   (delete-process oz-emulator-buffer))
	  ((or (not (get-buffer-process oz-compiler-buffer))
	       (not (get-buffer-process oz-emulator-buffer)))
	   (setq running nil)))
    (if (not running)
	(let ((file (concat (oz-make-temp-name "/tmp/ozpipeout") ":"
			    (oz-make-temp-name "/tmp/ozpipein"))))
	  (if (not start-flag) (message "Oz died.  Restarting ..."))
	  (setq oz-using-new-compiler use-new-compiler)
	  (define-key oz-mode-map "\C-cc"
	    (if oz-using-new-compiler 'oz-compiler-panel 'oz-precompile-file))
	  (cond (oz-using-new-compiler
		 t)
		(oz-win32
		 (setq oz-read-emulator-output nil)
		 (make-comint "Oz Compiler"
			      "ozcompiler" nil "+E"))
		(t
		 (make-comint "Oz Compiler"
			      "oz.compiler" nil "-emacs" "-S" file)))
	  (oz-create-buffer oz-compiler-buffer 'compiler)
	  (save-excursion
	    (set-buffer oz-compiler-buffer)
	    (set (make-local-variable 'compilation-error-regexp-alist)
		 '((oz-error-pattern 2 3 5)))
	    (set (make-local-variable 'compilation-parsing-end)
		 (point))
	    (set (make-local-variable 'compilation-error-list)
		 nil)
	    (set (make-local-variable 'compilation-last-buffer)
		 (current-buffer)))
	  (if (not oz-using-new-compiler)
	      (set-process-filter (get-buffer-process oz-compiler-buffer)
				  'oz-compiler-filter))
	  (bury-buffer oz-compiler-buffer)

	  (if oz-emulator-hook
	      (funcall oz-emulator-hook file)
	    (setq oz-emulator-buffer "*Oz Emulator*")
	    (cond ((and oz-using-new-compiler oz-win32)
		   (setq oz-read-emulator-output t)
		   (let ((compilercomp
			  (or (getenv "OZCOMPILERCOMP")
			      (concat (oz-home) "/lib/Compiler.ozc"))))
		     (make-comint "Oz Emulator" "ozemulator" nil "-E"
				  "-u" compilercomp)))
		  (oz-using-new-compiler
		   (setq oz-read-emulator-output t)
		   (make-comint "Oz Emulator" "oznc" nil "-E"))
		  (oz-win32 t)
		  (t
		   (make-comint "Oz Emulator"
				"oz.emulator" nil "-emacs" "-S" file)))
	    (oz-create-buffer oz-emulator-buffer 'emulator)
	    (if (not oz-win32)
		(set-process-filter (get-buffer-process oz-emulator-buffer)
				    'oz-emulator-filter)))

	  (bury-buffer oz-emulator-buffer)

	  (oz-set-title oz-frame-title)
	  (message "Oz started.")))))


;;------------------------------------------------------------
;; GDB Support
;;------------------------------------------------------------

(defun oz-set-other (set-compiler)
  "Set the value of environment variables OZEMULATOR or OZBOOT.
If SET-COMPILER is non-nil, set the compiler boot file (can also
be done via \\[oz-set-compiler]); if it is nil, set the emulator
binary (can also be done via \\[oz-set-emulator]."
  (interactive "P")
  (if set-compiler
      (oz-set-compiler)
    (oz-set-emulator)))

(defun oz-set-emulator ()
  "Set the value of environment variable OZEMULATOR.
This is the emulator used for debugging with gdb.
Can be selected by \\[oz-other-emulator]."
  (interactive)
  (setq oz-emulator
	(expand-file-name
	 (read-file-name "Choose Emulator: " nil nil t nil)))
  (if (getenv "OZEMULATOR")
      (setenv "OZEMULATOR" oz-emulator)))

(defun oz-set-compiler ()
  "Set the value of environment variable OZBOOT.
This is to specify an alternative Oz Compiler boot file.
Can be selected by \\[oz-other-compiler]."
  (interactive)
  (setq oz-boot
	(expand-file-name
	 (read-file-name "Choose Compiler boot file: " nil nil t nil)))
  (if (getenv "OZBOOT")
      (setenv "OZBOOT" oz-boot)))

(defun oz-other (set-compiler)
  "Switch between global and local Oz Emulator or Oz Compiler boot file.
If SET-COMPILER is non-nil, switch the compiler boot file (via
\\[oz-other-emulator]); if it is nil, switch the emulator binary
\(via \\[oz-other-compiler])."
  (interactive "P")
  (if set-compiler
      (oz-other-compiler)
    (oz-other-emulator)))

(defun oz-other-emulator ()
  "Switch between global and local Oz Emulator.
The local emulator is given by the environment variable OZEMULATOR
or can be set by \\[oz-set-emulator]."
  (interactive)
  (cond ((getenv "OZEMULATOR")
	 (setenv "OZEMULATOR" nil)
	 (message "Oz Emulator: global"))
	(t
	 (setenv "OZEMULATOR" oz-emulator)
	 (message "Oz Emulator: %s" oz-emulator))))

(defun oz-other-compiler ()
  "Switch between global and local Oz Compiler boot file.
The local boot file is given by the environment variable OZBOOT
or can be set by \\[oz-set-compiler]."
  (interactive)
  (cond ((getenv "OZBOOT")
	 (setenv "OZBOOT" nil)
	 (message "Oz Compiler: global"))
	(t
	 (setenv "OZBOOT" oz-boot)
	 (message "Oz Compiler: %s" oz-boot))))

(defun oz-gdb ()
  "Toggle debugging of the Oz Emulator with gdb.
The emulator to use for debugging is set via \\[oz-set-emulator]."
  (interactive)
  (if (getenv "OZ_PI")
      t
    (setenv "OZ_PI" "1")
    (if (getenv "OZPLATFORM")
	t
      (message "OZPLATFORM not set, using fallback: solaris-sparc")
      (setenv "OZPLATFORM" "solaris-sparc"))
    (setenv "OZPATH"
	    (concat (or (getenv "OZPATH") ".") ":"
		    (oz-home) "/lib" ":"
		    (oz-home) "/platform/" (getenv "OZPLATFORM") ":"
		    (oz-home) "/demo"))
    (setenv "PATH"
	    (concat (getenv "PATH") ":" (oz-home) "/bin")))
  (cond (oz-emulator-hook
	 (setq oz-emulator-hook nil)
	 (message "gdb disabled"))
	(t
	 (setq oz-emulator-hook 'oz-start-gdb-emulator)
	 (message "gdb enabled: %s" oz-emulator))))

(defun oz-start-gdb-emulator (file)
  "Run the Oz Emulator under gdb.
This is hooked into the variable `oz-emulator-hook' via \\[oz-gdb].
The directory containing FILE becomes the initial working directory
and source-file directory for gdb.  If you wish to change this, use
the gdb commands `cd DIR' and `directory'."
  (let ((old-buffer (current-buffer))
	(init-str (if oz-using-new-compiler
		      (concat "set args -u " oz-new-compiler-url "\n")
		    (concat "set args -S " file "\n"))))
    (cond (oz-gnu-emacs (gdb (concat "gdb " oz-emulator)))
	  (oz-lucid-emacs (gdb oz-emulator)))
    (setq oz-emulator-buffer (buffer-name (current-buffer)))
    (comint-send-string
     (get-buffer-process oz-emulator-buffer)
     init-str)
    (if oz-gdb-autostart
	(comint-send-string
	 (get-buffer-process oz-emulator-buffer)
	 "run\n"))
    (switch-to-buffer old-buffer)))

(defun oz-continue ()
  "Resume execution of the Oz Emulator under gdb after an error."
  (interactive)
  (comint-send-string (get-buffer-process oz-emulator-buffer) "c\n"))


;;------------------------------------------------------------
;; Feeding to the Compiler
;;------------------------------------------------------------

(defun oz-zmacs-stuff ()
  (if (boundp 'zmacs-region-stays) (setq zmacs-region-stays t)))

(defun oz-feed-buffer ()
  "Feed the current buffer to the Oz Compiler."
  (interactive)
  (let ((file (buffer-file-name))
	(cur (current-buffer)))
    (if (and file (buffer-modified-p)
	     (y-or-n-p (format "Save buffer %s first? " (buffer-name))))
	(save-buffer))
    (if (and file (not (buffer-modified-p)))
	(oz-feed-file file)
      (oz-feed-region (point-min) (point-max)))
    (switch-to-buffer cur))
  (oz-zmacs-stuff))

(defun oz-feed-region (start end)
  "Feed the current region to the Oz Compiler."
  (interactive "r")
  (oz-send-string (oz-get-region start end))
  (oz-zmacs-stuff))

(defun oz-feed-line (arg)
  "Feed the current line to the Oz Compiler.
With ARG, feed that many lines.  If ARG is negative, feed that many
preceding lines as well as the current line."
  (interactive "p")
  (let ((region (oz-line-region arg)))
    (oz-feed-region (car region) (cdr region))))

(defun oz-feed-paragraph (arg)
  "Feed the current paragraph to the Oz Compiler.
If the point is exactly between two paragraphs, feed the preceding
paragraph.  With ARG, feed that many paragraphs.  If ARG is negative,
feed that many preceding paragraphs as well as the current paragraph."
  (interactive "p")
  (let ((region (oz-paragraph-region arg)))
    (oz-feed-region (car region) (cdr region))))

(defun oz-send-string (string)
  "Feed STRING to the Oz Compiler, restarting it if it died."
  (oz-check-running nil oz-use-new-compiler)
  (let ((proc (get-buffer-process (if oz-using-new-compiler
				      oz-emulator-buffer
				    oz-compiler-buffer))))
    (comint-send-string proc string)
    (comint-send-string proc "\n")
    (save-excursion
      (set-buffer oz-compiler-buffer)
      (setq oz-compiler-output-start (point-max))
      ;; (comint-send-eof) does not work under win32, but this does:
      (comint-send-string proc "")
      (setq oz-next-error-marker nil))))


;;------------------------------------------------------------
;; Patterns for Indentation and Expression Hopping
;;------------------------------------------------------------

(defun oz-make-keywords-for-match (args)
  (concat "\\<\\("
	  (mapconcat 'identity args "\\|")
	  "\\)\\>"))

(defconst oz-declare-pattern
  (oz-make-keywords-for-match '("declare")))

(defconst oz-class-begin-pattern
  (oz-make-keywords-for-match
   '("class")))
(defconst oz-gump-class-begin-pattern
  (oz-make-keywords-for-match
   '("scanner" "parser")))

(defconst oz-class-member-pattern
  (oz-make-keywords-for-match
   '("meth")))
(defconst oz-gump-class-member-pattern
  (oz-make-keywords-for-match
   '("lex" "mode" "prod" "syn")))

(defconst oz-class-between-pattern
  (oz-make-keywords-for-match
   '("from" "prop" "attr" "feat")))
(defconst oz-gump-class-between-pattern
  (oz-make-keywords-for-match
   '("token")))

(defconst oz-begin-pattern
  (oz-make-keywords-for-match
   '("local" "proc" "fun" "case" "if" "or" "dis" "choice" "condis" "not"
     "thread" "try" "raise" "lock")))

(defconst oz-gump-between-pattern
  "=>")

(defconst oz-middle-pattern
  (concat (oz-make-keywords-for-match
	   '("in" "then" "else" "of" "elseof" "elsecase" "elseif"
	     "catch" "finally" "with"))
	  "\\|" "\\[\\]"))
(defconst oz-gump-middle-pattern
  "//")

(defconst oz-end-pattern
  (oz-make-keywords-for-match '("end")))

(defconst oz-left-pattern
  "\\[\\($\\|[^]]\\)\\|[({]")
(defconst oz-right-pattern
  "[])}]")
(defconst oz-left-or-right-pattern
  "[][(){}]")

(defconst oz-any-pattern
  (concat "\\<\\(attr\\|c\\(ase\\|atch\\|lass\\|hoice\\|ondis\\)\\|"
	  "declare\\|dis\\|e\\(lse\\(case\\|if\\|of\\)?\\|nd\\)\\|"
	  "f\\(eat\\|inally\\|rom\\|un\\)\\|if\\|in\\|"
	  "local\\|lock\\|meth\\|not\\|of\\|or\\|proc\\|prop\\|raise\\|"
	  "t\\(hen\\|hread\\|ry\\)\\|with\\)\\>\\|\\[\\]\\|"
	  oz-left-or-right-pattern))
(defconst oz-gump-any-pattern
  (concat "\\<\\(attr\\|c\\(ase\\|atch\\|lass\\|hoice\\|ondis\\)\\|"
	  "declare\\|dis\\|e\\(lse\\(case\\|if\\|of\\)?\\|nd\\)\\|"
	  "f\\(eat\\|inally\\|rom\\|un\\)\\|if\\|in\\|"
	  "l\\(ex\\|ocal\\|ock\\)\\|meth\\|mode\\|not\\|of\\|or\\|"
	  "p\\(arser\\|roc\\|rod\\|rop\\)\\|raise\\|scanner\\|syn\\|"
	  "t\\(hen\\|hread\\|oken\\|ry\\)\\|with\\)\\>\\|=>\\|\\[\\]\\|"
	  "//\\|" oz-left-or-right-pattern))

;;------------------------------------------------------------
;; Moving Among Oz Expressions
;;------------------------------------------------------------

(defun oz-forward-keyword ()
  "Search forward for the next keyword or parenthesis following point.
Return non-nil iff such a keyword was found.  Ignore quoted keywords.
Point is left at the first character of the keyword."
  (let ((pattern (if oz-gump-indentation oz-gump-any-pattern oz-any-pattern))
	(continue t)
	(ret nil))
    (while continue
      (if (re-search-forward pattern nil t)
	  (save-match-data
	    (goto-char (match-beginning 0))
	    (cond ((oz-is-quoted)
		   (goto-char (match-end 0)))
		  ((oz-is-directive)
		   (forward-line))
		  (t
		   (setq ret t continue nil))))
	(setq continue nil)))
    ret))

(defun oz-backward-keyword ()
  "Search backward for the last keyword or parenthesis preceding point.
Return non-nil iff such a keyword was found.  Ignore quoted keywords.
Point is left at the first character of the keyword."
  (let ((pattern (if oz-gump-indentation oz-gump-any-pattern oz-any-pattern))
	(continue t)
	(ret nil))
    (while continue
      (if (re-search-backward pattern nil t)
	  (cond ((oz-is-quoted) t)
		((oz-is-directive) t)
		((oz-is-box)
		 (setq ret t continue nil))
		(t
		 (setq ret t continue nil)))
	(setq continue nil)))
    ret))

;; Note: The following do not allow for newlines inside quoted tokens
;; to make matching easier ...
;; Furthermore, `/*' ... `*/' style comments are not included here
;; because of the problems of nesting and line breaks.
(defconst oz-string-pattern
  "\"\\([^\"\C-@\\\n]\\|\\\\.\\)*\"")
(defconst oz-atom-pattern
  "'\\([^'\C-@\\\n]\\|\\\\.\\)*'")
(defconst oz-variable-pattern
  "`\\([^`\C-@\\\n]\\|\\\\.\\)*`")
(defconst oz-char-pattern
  "&\\([^\C-@\\\n]\\|\\\\.\\)")
(defconst oz-comment-pattern
  "%.*")
(defconst oz-quoted-pattern
  (concat oz-string-pattern "\\|" oz-atom-pattern "\\|"
	  oz-variable-pattern "\\|" oz-char-pattern "\\|"
	  oz-comment-pattern))

(defconst oz-directive-pattern
  "\\\\[a-zA-Z]+\\>")

(defun oz-is-quoted ()
  "Return non-nil iff the position of the point is quoted.
Return non-nil iff the point is inside a string, quoted atom, backquote
variable, ampersand-denoted character or one-line comment.  In this case,
move the point to the beginning of the corresponding token.  Else the
point is not moved."
  (let ((ret nil)
	(p (point))
	cont-point)
    (beginning-of-line)
    (while (and (not ret)
		(prog1
		    (re-search-forward "[\"'`&%]\\|$" nil t)
		  (setq cont-point (match-end 0))
		  (goto-char (match-beginning 0)))
		(< (point) p))
      (cond ((looking-at oz-quoted-pattern)
	     (let ((quote-end (match-end 0)))
	       (if (< p quote-end)
		   (setq ret t)
		 (goto-char quote-end))))
	    ((looking-at "\"")
	     (error
	      "Illegal string syntax or unterminated string"))
	    ((looking-at "'")
	     (error
	      "Illegal atom syntax or unterminated quoted atom"))
	    ((looking-at "`")
	     (error
	      "Illegal variable syntax or unterminated backquote variable"))
	    (t (goto-char cont-point))))
    (if (not ret) (goto-char p))
    ret))

(defun oz-is-directive ()
  "Return non-nil iff the point is one position after the start of a directive.
That means, if the point is at a keyword-lookalike and preceded by a
backslash.  If yes, the point is moved to the backslash."
  (let ((p (point)))
    (if (= p (point-min))
	t
      (backward-char))
    (if (looking-at oz-directive-pattern)
	t
      (goto-char p)
      nil)))

(defun oz-is-box ()
  "Return non-nil if point is at the second character of a `[]' token.
In this case, move point to the first character of this token."
  (let ((p (point)))
    (if (= p (point-min))
	t
      (backward-char))
    (if (and (looking-at "\\[\\]")
	     (not (oz-is-quoted)))   ; consider the list '[&[]'!
	t
      (goto-char p)
      nil)))

;;------------------------------------------------------------
;; Moving to Expression Boundaries

(defun oz-backward-begin (&optional is-field-value)
  "Move to the last unmatched begin and return column of point.
If IS-FIELD-VALUE is non-nil, a between-pattern of the same nesting
level is also considered a begin-pattern.  This is used by indentation
to handle lines like 'attr a:'."
  (let ((ret nil)
	(nesting 0))
    (while (not ret)
      (if (oz-backward-keyword)
	  (cond ((looking-at oz-declare-pattern)
		 (setq ret (current-column)))
		((or (looking-at oz-class-begin-pattern)
		     (looking-at oz-class-member-pattern)
		     (looking-at oz-begin-pattern)
		     (looking-at oz-left-pattern)
		     (and is-field-value
			  (or (looking-at oz-class-between-pattern)
			      (and oz-gump-indentation
				   (looking-at
				    oz-gump-class-between-pattern))))
		     (and oz-gump-indentation
			  (or (looking-at oz-gump-class-begin-pattern)
			      (looking-at oz-gump-class-member-pattern))))
		 (if (= nesting 0)
		     (setq ret (current-column))
		   (setq nesting (1- nesting))))
		((looking-at oz-end-pattern)
		 (setq nesting (1+ nesting)))
		((looking-at oz-right-pattern)
		 (oz-backward-paren)))
	(goto-char (point-min))
	(if (= nesting 0)
	    (setq ret 0)
	  (error "No matching begin token"))))
    ret))

(defun oz-backward-paren ()
  "Move to the last unmatched opening parenthesis and return column of point."
  (let ((continue t)
	(nesting 0))
    (while continue
      (if (re-search-backward oz-left-or-right-pattern nil t)
	  (cond ((oz-is-quoted) t)
		((looking-at oz-left-pattern)
		 (if (= nesting 0)
		     (setq continue nil)
		   (setq nesting (1- nesting))))
		((oz-is-box) t)
		(t
		 (setq nesting (1+ nesting))))
	(error "No matching opening parenthesis"))))
  (current-column))

(defun oz-forward-end ()
  "Move point to next unmatched end."
  (let ((continue t)
	(nesting 0))
    (while continue
      (if (oz-forward-keyword)
	  (let ((cont-point (match-end 0)))
	    (cond ((or (looking-at oz-class-begin-pattern)
		       (looking-at oz-class-member-pattern)
		       (looking-at oz-begin-pattern)
		       (and oz-gump-indentation
			    (or (looking-at oz-gump-class-begin-pattern)
				(looking-at oz-gump-class-member-pattern))))
		   (setq nesting (1+ nesting))
		   (goto-char cont-point))
		  ((looking-at oz-end-pattern)
		   (cond ((= nesting 1)
			  (setq continue nil))
			 ((= nesting 0)
			  (error "Containing expression ends prematurely"))
			 (t
			  (setq nesting (1- nesting))
			  (goto-char cont-point))))
		  ((looking-at oz-left-pattern)
		   (forward-char)
		   (oz-forward-paren)
		   (forward-char))
		  ((looking-at oz-right-pattern)
		   (error "Containing expression ends prematurely"))
		  (t
		   (goto-char cont-point))))
	(setq continue nil)))))

(defun oz-forward-paren ()
  "Move to the next unmatched closing parenthesis."
  (let ((continue t)
	(nesting 0))
    (while continue
      (if (re-search-forward oz-left-or-right-pattern nil t)
	  (progn
	    (goto-char (match-beginning 0))
	    (cond ((oz-is-quoted)
		   (goto-char (match-end 0)))
		  ((looking-at oz-right-pattern)
		   (if (= nesting 0)
		       (setq continue nil)
		     (setq nesting (1- nesting))
		     (forward-char)))
		  ((looking-at "\\[\\]")
		   (goto-char (match-end 0)))
		  (t
		   (forward-char)
		   (setq nesting (1+ nesting)))))
	(error "No matching closing parenthesis")))))


;;------------------------------------------------------------
;; Indentation
;;------------------------------------------------------------

(defun oz-electric-terminate-line ()
  "Terminate current line.
If variable `oz-auto-indent' is non-nil, indent the terminated line
and the following line."
  (interactive)
  (open-line 1)
  (cond (oz-auto-indent (oz-indent-line)))
  (delete-horizontal-space) ; Removes trailing whitespace
  (forward-line 1)
  (cond (oz-auto-indent (oz-indent-line))))

(defun oz-indent-buffer ()
  "Indent every line in the current buffer."
  (interactive)
  (oz-indent-region (point-min) (point-max)))

(defun oz-indent-region (start end)
  "Indent every line in the current region."
  (interactive "r")
  (goto-char start)
  (let ((current-line (count-lines 1 start))
	(end-line (count-lines 1 end)))
    (while (< current-line end-line)
      (message "Indenting line %s ..." current-line)
      (oz-indent-line t)
      (setq current-line (1+ current-line))
      (forward-line 1)))
  (message nil))

(defun oz-indent-line (&optional dont-change-empty-lines)
  "Indent the current line.
If DONT-CHANGE-EMPTY-LINES is non-nil and the current line is empty
save for whitespace, then its indentation is not changed.  If the
point was inside the line's leading whitespace, then it is moved to
the end of this whitespace after indentation."
  (interactive)
  (let ((case-fold-search nil))   ; respect case
    (unwind-protect
	(save-excursion
	  (beginning-of-line)
	  (skip-chars-forward " \t")
	  (if (and dont-change-empty-lines (oz-is-empty)) t
	    (let ((col (save-excursion (oz-calc-indent))))
	      ;; a negative result means: do not change indentation
	      (cond ((>= col 0)
		     (delete-horizontal-space)
		     (indent-to col))))))
      (if (oz-is-left)
	  (skip-chars-forward " \t")))))

(defun oz-calc-indent ()
  "Calculate the required indentation for the current line.
The point must be at the beginning of the current line.
Return a negative value if the indentation is not to be changed,
else return the column up to where the line should be indented."
  (cond ((looking-at oz-declare-pattern)
	 0)
	((looking-at "\\\\")   ; directive
	 0)
	((looking-at "%%%")
	 0)
	((looking-at "%[^%]")
	 -1)
	((oz-is-field-value)
	 (oz-backward-begin t)
	 (cond ((looking-at oz-left-pattern)   ; e.g., 'f(x:'
		(forward-char)
		(+ (oz-get-column-of-next-nonwhite) oz-indent-chars))
	       (t   ; e.g., 'attr x:'
		(+ (current-column) (* oz-indent-chars 2)))))
	((or (looking-at oz-middle-pattern)
	     (looking-at oz-end-pattern)
	     (and oz-gump-indentation
		  (looking-at oz-gump-middle-pattern)))
	 (oz-backward-begin))
	((looking-at oz-right-pattern)
	 (oz-backward-paren))
	(t
	 (let ((ret nil)
	       (is-class-member
		(or (looking-at oz-class-member-pattern)
		    (looking-at oz-class-between-pattern)
		    (and oz-gump-indentation
			 (or (looking-at oz-gump-class-member-pattern)
			     (looking-at oz-gump-class-between-pattern))))))
	   (while (not ret)
	     (if (oz-backward-keyword)
		 (cond ((looking-at oz-declare-pattern)
			(setq ret (current-column)))
		       ((or (looking-at oz-class-begin-pattern)
			    (looking-at oz-class-member-pattern)
			    (looking-at oz-begin-pattern)
			    (and oz-gump-indentation
				 (or (looking-at oz-gump-class-begin-pattern)
				     (looking-at oz-gump-class-member-pattern)
				     (looking-at oz-gump-between-pattern))))
			(setq ret (+ (current-column) oz-indent-chars)))
		       ((or (looking-at oz-class-between-pattern)
			    (and oz-gump-indentation
				 (looking-at oz-gump-class-between-pattern)))
			(if is-class-member t
			  (setq ret (+ (current-column) oz-indent-chars))))
		       ((or (looking-at oz-middle-pattern)
			    (and oz-gump-indentation
				 (looking-at oz-gump-middle-pattern)))
			(oz-backward-begin)
			(if (looking-at oz-declare-pattern)
			    ;; do not indent after 'declare X in'
			    (setq ret (current-column))
			  (setq ret (+ (current-column) oz-indent-chars))))
		       ((looking-at oz-end-pattern)
			(oz-backward-begin)
			(if (oz-is-left)
			    ;; this is an approximation made for efficiency
			    (setq ret (current-column))))
		       ((looking-at oz-left-pattern)
			(forward-char)
			(setq ret (oz-get-column-of-next-nonwhite)))
		       ((looking-at oz-right-pattern)
			(oz-backward-paren)))
	       (setq ret 0)))
	   ret))))

(defun oz-is-field-value ()
  "Return non-nil iff the token preceding the point is a colon.
This is to realize the indentation rule that in records with a feature
on one line and the corresponding subtree expression on another, the
expression has to be indented relative to the feature.  If this is the
case, move the point to the colon character."
  (let ((old (point)))
    (skip-chars-backward "? \n\t\r\v\f")
    (if (= (point) (point-min))
	t
      (backward-char))
    (if (and (looking-at ":") (not (oz-is-quoted)))
	t
      (goto-char old)
      nil)))

(defun oz-get-column-of-next-nonwhite ()
  "Return the column number of the first non-white character to follow point.
If there is none until the end of line, return the column of point."
  (let ((col (current-column)))
    (if (oz-is-right)
	col
      (re-search-forward "[^ \t]" nil t)
      (1- (current-column)))))

(defun oz-is-left ()
  "Return non-nil iff the point is only preceded by whitespace in the line."
  (save-excursion
    (skip-chars-backward " \t")
    (= (current-column) 0)))

(defun oz-is-right ()
  "Return non-nil iff the point is only followed by whitespace in the line."
  (looking-at "[ \t]*$"))

(defun oz-is-empty ()
  "Return non-nil iff the current line is empty save for whitespace."
  (and (oz-is-left) (oz-is-right)))


;;------------------------------------------------------------
;; Oz Expression Hopping
;;------------------------------------------------------------

(defun forward-oz-expr (&optional arg)
  "Move forward one balanced Oz expression.
With argument, do it that many times.  Negative ARG means backwards."
  (interactive "p")
  (let ((case-fold-search nil) pos)
    (or arg (setq arg 1))
    (if (< arg 0)
	(backward-oz-expr (- arg))
      (while (> arg 0)
	(if (oz-is-quoted)
	    (goto-char (match-end 0))
	  (let ((pos (scan-sexps (point) 1)))
	    (if (not pos)
		(progn (end-of-buffer) (setq arg 0))
	      (goto-char pos)
	      (if (= (char-syntax (preceding-char)) ?w)
		  (progn
		    (forward-word -1)
		    (cond ((or (looking-at oz-class-begin-pattern)
			       (looking-at oz-class-member-pattern)
			       (looking-at oz-begin-pattern)
			       (and oz-gump-indentation
				    (or (looking-at
					 oz-gump-class-begin-pattern)
					(looking-at
					 oz-gump-class-member-pattern))))
			   (oz-forward-end)
			   (goto-char (match-end 0)))
			  ((or (looking-at oz-class-between-pattern)
			       (looking-at oz-middle-pattern)
			       (and oz-gump-indentation
				    (or (looking-at
					 oz-gump-class-between-pattern)
					(looking-at
					 oz-gump-between-pattern)
					(looking-at
					 oz-gump-middle-pattern))))
			   (goto-char (match-end 0))
			   (setq arg (1+ arg)))
			  ((looking-at oz-end-pattern)
			   (error "Containing expression ends prematurely"))
			  (t
			   (forward-word 1)))))
	      (setq arg (1- arg)))))))))

(defun backward-oz-expr (&optional arg)
  "Move backward one balanced Oz expression.
With argument, do it that many times.  Argument must be positive."
  (interactive "p")
  (let ((case-fold-search nil))
    (or arg (setq arg 1))
    (while (> arg 0)
      (let ((pos (scan-sexps (point) -1)))
	(if (equal pos nil)
	    (progn (beginning-of-buffer) (setq arg 0))
	  (goto-char pos)
	  (cond ((looking-at oz-end-pattern)
		 (oz-backward-begin))
		((or (looking-at oz-class-between-pattern)
		     (looking-at oz-middle-pattern)
		     (and oz-gump-indentation
			  (or (looking-at oz-gump-class-between-pattern)
			      (looking-at oz-gump-between-pattern)
			      (looking-at oz-gump-middle-pattern))))
		 (setq arg (1+ arg)))
		((looking-at oz-begin-pattern)
		 (error "Containing expression ends prematurely")))
	  (setq arg (1- arg)))))))

(defun mark-oz-expr (arg)
  "Set mark ARG balanced Oz expressions from point.
The place mark goes is the same place \\[forward-oz-expr] would
move to with the same argument."
  (interactive "p")
  (push-mark
    (save-excursion
      (forward-oz-expr arg)
      (point))
    nil t))

(defun transpose-oz-exprs (arg)
  "Like \\[transpose-words] but applies to balanced Oz expressions.
Does not work in all cases."
  (interactive "*p")
  (transpose-subr 'forward-oz-expr arg))

(defun kill-oz-expr (arg)
  "Kill the balanced Oz expression following the cursor.
With argument, kill that many Oz expressions after the cursor.
Negative arg -N means kill N Oz expressions before the cursor."
  (interactive "p")
  (let ((pos (point)))
    (forward-oz-expr arg)
    (kill-region pos (point))))

(defun backward-kill-oz-expr (arg)
  "Kill the balanced Oz expression preceding the cursor.
With argument, kill that many Oz expressions before the cursor.
Negative arg -N means kill N Oz expressions after the cursor."
  (interactive "p")
  (let ((pos (point)))
    (forward-oz-expr (- arg))
    (kill-region pos (point))))


;;------------------------------------------------------------
;; oz-mode
;;------------------------------------------------------------

;; Use this for testing modifications to the syntax table:
;;    (setq oz-mode-syntax-table nil)
(if oz-mode-syntax-table
    ()
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?_ "w" table)
    (modify-syntax-entry ?\\ "/" table)
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
    (setq oz-mode-syntax-table table)))

(define-abbrev-table 'oz-mode-abbrev-table ())

(defun oz-mode-variables ()
  (set-syntax-table oz-mode-syntax-table)
  (setq local-abbrev-table oz-mode-abbrev-table)
  (set (make-local-variable 'paragraph-start)
       (concat "^$\\|" page-delimiter))
  (set (make-local-variable 'paragraph-separate)
       paragraph-start)
  (set (make-local-variable 'paragraph-ignore-fill-prefix)
       t)
  (set (make-local-variable 'fill-paragraph-function)
       'oz-fill-paragraph)
  (set (make-local-variable 'indent-line-function)
       'oz-indent-line)
  (set (make-local-variable 'comment-start)
       "%")
  (set (make-local-variable 'comment-end)
       "")
  (set (make-local-variable 'comment-start-skip)
       "/\\*+ *\\|% *")
  (set (make-local-variable 'parse-sexp-ignore-comments)
       t)
  (set (make-local-variable 'words-include-escapes)
       t)
  (set (make-local-variable 'compilation-last-buffer)
       (get-buffer-create oz-compiler-buffer)))

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
	(define-key map "\C-c\C-b\C-r"  'oz-feed-region-browse))
    (define-key map "\C-c\C-f\C-f" 'oz-feed-file)
    (define-key map "\M-\C-m"      'oz-feed-buffer)
    (define-key map "\M-r"         'oz-feed-region)
    (define-key map "\M-l"         'oz-feed-line)
    (define-key map "\C-c\C-p"     'oz-feed-paragraph)
    (define-key map "\C-cb"        'oz-feed-line-browse)
    (define-key map "\C-c\C-b"     'oz-feed-region-browse)
    (define-key map "\M-n"         'oz-next-buffer)
    (define-key map "\M-p"         'oz-previous-buffer)

    (define-key map "\C-c\C-d\C-r" 'oz-debug-start)
    (define-key map "\C-c\C-d\C-h" 'oz-debug-stop)
    (define-key map "\C-c\C-d\C-b" 'oz-breakpoint-key-set)
    (define-key map "\C-c\C-d\C-d" 'oz-breakpoint-key-delete)
    (cond (oz-gnu-emacs
	   (define-key map [(meta shift mouse-1)] 'oz-breakpoint-mouse-set)
	   (define-key map [(meta shift mouse-3)] 'oz-breakpoint-mouse-delete))
	  (oz-lucid-emacs
	   (define-key map [(meta shift button1)] 'oz-breakpoint-mouse-set)
	   (define-key map [(meta shift button3)] 'oz-breakpoint-mouse-delete))
	  )
    (define-key map "\C-c\C-f\C-r" 'oz-profiler-start)
    (define-key map "\C-c\C-f\C-h" 'oz-profiler-stop))

  (define-key map "\M-\C-x"	'oz-feed-paragraph)

  (define-key map [(control c) (control h)] 'oz-halt)
  (define-key map "\C-c\C-h"    'oz-halt)

  (define-key map "\C-c\C-c"    'oz-toggle-compiler)
  (define-key map "\C-c\C-e"    'oz-toggle-emulator)
  (define-key map "\C-c\C-t"    'oz-toggle-temp)
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
  (define-key map "\C-x`" 'oz-goto-next-error))

(oz-mode-commands oz-mode-map)

(defun oz-mode ()
  "Major mode for editing Oz code.

Commands:
\\{oz-mode-map}
Entry to this mode calls the value of `oz-mode-hook'
if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map oz-mode-map)
  (setq major-mode 'oz-mode)
  (setq mode-name "Oz")
  (oz-mode-variables)
  (if (and oz-lucid-emacs (not (assoc "Oz" current-menubar)))
      (set-buffer-menubar (oz-insert-menu oz-menubar current-menubar)))

  (set (make-local-variable 'oz-gump-indentation) nil)

  ;; font lock stuff
  (oz-set-font-lock-defaults)
  (if (and oz-want-font-lock window-system)
      (font-lock-mode 1))
  (run-hooks 'oz-mode-hook))

(defun ozm-mode ()
  "Major mode for displaying Oz machine code.

Commands:
\\{oz-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (use-local-map oz-mode-map)
  (setq major-mode 'ozm-mode)
  (setq mode-name "Oz-Machine")
  (oz-mode-variables)

  ;; font lock stuff
  (ozm-set-font-lock-defaults)
  (if (and oz-want-font-lock window-system)
      (font-lock-mode 1)))

(defun oz-gump-mode ()
  "Major mode for editing Oz code with embedded Gump specifications.

Commands:
\\{oz-mode-map}
Entry to this mode calls the value of `oz-mode-hook'
if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map oz-mode-map)
  (setq major-mode 'oz-gump-mode)
  (setq mode-name "Oz-Gump")
  (oz-mode-variables)
  (if (and oz-lucid-emacs (not (assoc "Oz" current-menubar)))
      (set-buffer-menubar (oz-insert-menu oz-menubar current-menubar)))

  (set (make-local-variable 'oz-gump-indentation) t)

  ;; font lock stuff
  (oz-gump-set-font-lock-defaults)
  (if (and oz-want-font-lock window-system)
      (font-lock-mode 1))
  (run-hooks 'oz-mode-hook))

(defun oz-insert-menu (menu list)
  "Add the Oz menu to the menu bar.
Take care not to move it too much to the right."
  (cond ((null list)
	 menu)
	((null (car list))
	 (cons (car menu) list))
	(t (cons (car list) (oz-insert-menu menu (cdr list))))))


;;------------------------------------------------------------
;; Lisp Paragraph Filling Commands
;;------------------------------------------------------------

(defun oz-fill-paragraph (&optional justify)
  "Like \\[fill-paragraph], but handle Oz comments.
If any of the current line is a comment, fill the comment or the
paragraph of it that point is in, preserving the comment's indentation
and initial percent signs."
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
       ;; percent sign which starts the comment shouldn't be part of a string
       ;; or character.
       ((progn
	  (while (not (looking-at "%\\|$"))
	    (skip-chars-forward "^%\n\\\\")
	    (cond
	     ((eq (char-after (point)) ?\\) (forward-char 2))))
	  (looking-at "%+[ \t]*"))
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

	;; Lines with only percent signs on them can be paragraph boundaries.
	(let ((paragraph-start (concat paragraph-start "\\|^[ \t%]*$"))
	      (paragraph-separate (concat paragraph-start "\\|^[ \t%]*$"))
	      (fill-prefix comment-fill-prefix))
	  (fill-paragraph justify))))
    t))


;;------------------------------------------------------------
;; Fontification
;;------------------------------------------------------------

(if window-system
    (require 'font-lock))

(defconst oz-keywords
  '("declare" "local" "in" "end"
    "proc" "fun"
    "case" "then" "else" "of" "elseof" "elsecase"
    "class" "from" "prop" "attr" "feat" "meth" "self"
    "true" "false" "unit"
    "div" "mod" "andthen" "orelse"
    "if" "elseif" "or" "dis" "choice" "condis" "not"
    "thread" "try" "catch" "finally" "raise" "with" "lock"
    "skip" "fail")
  "List of all Oz keywords with identifier syntax.")

(defconst oz-char-matcher
  (concat "&\\(" "[^\C-@\\\n]" "\\|" "\\\\" "\\("
	  "[0-7][0-7][0-7]\\|x[0-9A-Fa-f][0-9A-Fa-f]\\|[abfnrtc\\'\"`]"
	  "\\)" "\\)")
  "Regular expression matching an ampersand character constant.
Used only for fontification.")

(defconst oz-directive-matcher
  "\\(^\\|[^&]\\)\\(\\\\[a-z]\\([^\'%\n]\\|'[\"-~]'\\)*\\)"
  "Regular expression matching a compiler or macro directive.
Used only for fontification.")

(defconst oz-keywords-matcher-1
  (concat "^\\(" (mapconcat 'identity oz-keywords "\\|") "\\)\\>")
  "Regular expression matching any keyword at the beginning of a line.")

(defconst oz-keywords-matcher-2
  (concat "[^\\A-Za-z0-9_]\\("
	  (mapconcat 'identity oz-keywords "\\|") "\\)\\>")
  "Regular expression matching any keyword not preceded by a backslash.
This serves to distinguish between the directive `\\else' and the keyword
`else'.  Keywords at the beginning of a line are not matched.
The first subexpression matches the keyword proper (for fontification).")

(defconst oz-keywords-matcher-3
  "[.#!:@,]\\|\\[\\]"
  "Regular expression matching non-identifier keywords.")

(defconst oz-proc-fun-matcher
  (concat "\\<\\(proc\\|fun\\)\\>\\([^{\n]*\\){!?"
	  "\\([A-Z][A-Za-z0-9_]*\\|`[^`\n]*`\\)")
  "Regular expression matching proc or fun definitions.
The second subexpression matches the definition's identifier
\(if it is a variable) and is used for fontification.")

(defconst oz-class-matcher
  (concat "\\<class\\([ \t]+\\|[ \t]*!\\)"
	  "\\([A-Z][A-Za-z0-9_]*\\|`[^`\n]*`\\)")
  "Regular expression matching class definitions.
The second subexpression matches the definition's identifier
\(if it is a variable) and is used for fontification.")

(defconst oz-meth-matcher
  (concat "\\<meth\\([ \t]+\\|[ \t]*!\\)"
	  "\\([A-Za-z][A-Za-z0-9_]*\\|`[^`\n]*`\\|'[^'\n]*'\\)")
  "Regular expression matching method definitions.
The second subexpression matches the definition's identifier
and is used for fontification.")

(defconst oz-font-lock-keywords-1
  (list (cons oz-char-matcher 'font-lock-string-face)
	oz-keywords-matcher-1
	(cons oz-keywords-matcher-2 1)
	oz-keywords-matcher-3)
  "Subdued level highlighting for Oz mode.")

(defconst oz-font-lock-keywords oz-font-lock-keywords-1
  "Default expressions to highlight in Oz mode.")

(defconst oz-font-lock-keywords-2
  (cons (list oz-directive-matcher
	      '(2 font-lock-reference-face))
	oz-font-lock-keywords-1)
  "Medium level highlighting for Oz mode.")

(defconst oz-font-lock-keywords-3
  (append (list (list oz-proc-fun-matcher
		      '(2 font-lock-variable-name-face)
		      '(3 font-lock-function-name-face))
		(list oz-class-matcher
		      '(2 font-lock-type-face))
		(list oz-meth-matcher
		      '(2 font-lock-function-name-face)))
	  oz-font-lock-keywords-2)
  "Gaudy level highlighting for Oz mode.")

(defun oz-set-font-lock-defaults ()
  (set (make-local-variable 'font-lock-defaults)
       '((oz-font-lock-keywords oz-font-lock-keywords-1
	  oz-font-lock-keywords-2 oz-font-lock-keywords-3)
	 nil nil ((?& . "/")) beginning-of-line)))

;;------------------------------------------------------------
;; Fontification for Oz-Machine Mode

(defconst ozm-keywords-matcher
  "\\<\\(true\\|false\\|unit\\)\\>")

(defconst ozm-instr-matcher-1
  (concat
   "\t\\("
   (mapconcat
    'identity
    '("move" "moveMoveXYXY" "moveMoveYXYX" "allocateL"
      "moveMoveXYYX" "moveMoveYXXY" "createNamedVariable" "createVariable"
      "createVariableMove" "putInt" "putConstant" "putList" "putRecord"
      "setInt" "setConstant" "setValue" "setVariable" "setVoid" "getInt"
      "getConstant" "getList" "getListValVar" "getRecord" "unifyInt"
      "unifyConstant" "unifyValue" "unifyVariable" "unifyValVar" "unifyVoid"
      "unify" "branch" "callBuiltin" "inlineFun[1-3]" "inlineRel[1-3]"
      "inlineEqEq" "inlineDot" "inlineUparrow" "inlineAt" "inlineAssign"
      "genCall" "call" "tailCall" "fastCall" "fastTailCall" "genFastCall"
      "marshalledFastCall" "sendMsg" "tailSendMsg" "applMeth" "tailApplMeth"
      "thread" "threadX" "exHandler" "createCond" "nextClause" "shallowGuard"
      "shallowTest[12]" "testConst" "testNumber" "testBool" "switchOnTerm"
      "getVariable" "getVarVar" "getVoid" "lockThread" "getSelf" "det"
      "weakDet" "debugInfo" "globalVarname" "localVarname" "clearY") "\\|")
   "\\)("))

(defconst ozm-instr-matcher-2
  (concat
   "\t\\("
   (mapconcat
    'identity
    '("allocateL[1-9]" "allocateL10" "deAllocateL"
      "deAllocateL[1-9]" "deAllocateL10" "return" "popEx" "createOr"
      "createEnumOr" "createChoice" "clause" "emptyClause" "lastClause"
      "shallowThen" "failure" "succeed" "wait" "waitTop" "ask" "profileProc")
    "\\|")
   "\\)$"))

(defconst ozm-definition-matcher
  "\t\\(definition\\|endDefinition\\)(")

(defconst ozm-register-matcher
  "\\<\\(x\\|y\\|g\\)([0-9]+)")

(defconst ozm-label-matcher
  "^lbl([0-9]+)")

(defconst ozm-name-matcher
  "<N: [^>]+>")

(defconst ozm-builtin-name-matcher
  (concat "\t\\(callBuiltin\\|inlineRel[1-3]\\|inlineFun[1-3]\\|inlineEqEq\\|"
	  "shallowTest[12]\\)(\\([A-Za-z0-9_]+\\|'[^'\n]'\\)"))

(defconst ozm-font-lock-keywords-1
  (list (cons ozm-keywords-matcher 1)
	(list ozm-instr-matcher-1
	      '(1 font-lock-keyword-face))
	(list ozm-instr-matcher-2
	      '(1 font-lock-keyword-face))
	(list ozm-definition-matcher
	      '(1 font-lock-function-name-face))
	(cons ozm-name-matcher 'font-lock-string-face)))

(defconst ozm-font-lock-keywords ozm-font-lock-keywords-1)

(defconst ozm-font-lock-keywords-2
  (append (list (list ozm-register-matcher
		      '(1 font-lock-type-face))
		(cons ozm-label-matcher 'font-lock-reference-face)
		(list ozm-builtin-name-matcher
		      '(2 font-lock-variable-name-face)))
	  ozm-font-lock-keywords-1))

(defun ozm-set-font-lock-defaults ()
  (set (make-local-variable 'font-lock-defaults)
       '((ozm-font-lock-keywords ozm-font-lock-keywords-1
	  ozm-font-lock-keywords-2)
	 nil nil nil beginning-of-line)))

;;------------------------------------------------------------
;; Fontification for Oz-Gump Mode

(defconst oz-gump-keywords
  '("lex" "mode" "parser" "prod" "scanner" "syn" "token"))

(defconst oz-gump-regex-matcher
  (concat
   "\\<lex[^A-Za-z0-9_<\n][^<\n]*\\(<" "\\("
   "\\[\\([^]\\]\\|\\\\.\\)+\\]" "\\|"
   "\"[^\"\n]+\"" "\\|"
   "\\\\." "\\|"
   "[^]<>\"[\\\n]" "\\)+"
   ">\\|<<EOF>>\\)"))

(defconst oz-gump-keywords-matcher-1
  (concat "^\\(" (mapconcat 'identity oz-gump-keywords "\\|") "\\)\\>")
  "Regular expression matching any keyword at the beginning of a line.")

(defconst oz-gump-keywords-matcher-2
  (concat "[^\\A-Za-z0-9_]\\("
	  (mapconcat 'identity oz-gump-keywords "\\|") "\\)\\>")
  "Regular expression matching any keyword not preceded by a backslash.
This serves to distinguish between the directive `\\else' and the keyword
`else'.  Keywords at the beginning of a line are not matched.
The first subexpression matches the keyword proper (for fontification).")

(defconst oz-gump-keywords-matcher-3
  "=>\\|//"
  "Regular expression matching non-identifier keywords.")

(defconst oz-gump-scanner-parser-matcher
  (concat "\\<\\(parser\\|scanner\\)[ \t]+"
	  "\\([A-Z][A-Za-z0-9_]*\\|`[^`\n]*`\\)")
  "Regular expression matching parser or scanner definitions.
The second subexpression matches the definition's identifier
\(if it is a variable) and is used for fontification.")

(defconst oz-gump-lex-matcher
  (concat "\\<lex[ \t]+"
	  "\\([a-z][A-Za-z0-9_]*\\|'[^'\n]*'\\)[ \t]*=")
  "Regular expression matching lexical abbreviation definitions.
The first subexpression matches the definition's identifier
\(if it is an atom) and is used for fontification.")

(defconst oz-gump-syn-matcher
  (concat "\\<syn[ \t]+"
	  "\\([A-Za-z][A-Za-z0-9_]*\\|`[^`\n]*`\\|'[^'\n]*'\\)")
  "Regular expression matching syntax rule definitions.
The first subexpression matches the definition's identifier
and is used for fontification.")

(defconst oz-gump-font-lock-keywords-1
  (append (list (list oz-gump-regex-matcher
		      '(1 font-lock-string-face))
		oz-gump-keywords-matcher-1
		(cons oz-gump-keywords-matcher-2 1)
		oz-gump-keywords-matcher-3)
	  oz-font-lock-keywords-1)
  "Subdued level highlighting for Oz-Gump mode.")

(defconst oz-gump-font-lock-keywords oz-gump-font-lock-keywords-1
  "Default expressions to highlight in Oz-Gump mode.")

(defconst oz-gump-font-lock-keywords-2
  (append (list (list oz-gump-regex-matcher
		      '(1 font-lock-string-face))
		oz-gump-keywords-matcher-1
		(cons oz-gump-keywords-matcher-2 1)
		oz-gump-keywords-matcher-3)
	  oz-font-lock-keywords-2)
  "Medium level highlighting for Oz-Gump mode.")

(defconst oz-gump-font-lock-keywords-3
  (append (list (list oz-gump-regex-matcher
		      '(1 font-lock-string-face))
		oz-gump-keywords-matcher-1
		(cons oz-gump-keywords-matcher-2 1)
		oz-gump-keywords-matcher-3
		(list oz-gump-scanner-parser-matcher
		      '(2 font-lock-type-face))
		(list oz-gump-lex-matcher
		      '(1 font-lock-type-face))
		(list oz-gump-syn-matcher
		      '(1 font-lock-function-name-face)))
	  oz-font-lock-keywords-3)
  "Gaudy level highlighting for Oz-Gump mode.")

(defun oz-gump-set-font-lock-defaults ()
  (set (make-local-variable 'font-lock-defaults)
       '((oz-gump-font-lock-keywords oz-gump-font-lock-keywords-1
	  oz-gump-font-lock-keywords-2 oz-gump-font-lock-keywords-3)
	 nil nil ((?& . "/")) beginning-of-line)))

;;------------------------------------------------------------

(defun oz-fontify-buffer ()
  (interactive)
  (if window-system (font-lock-fontify-buffer)))

(defun oz-fontify (&optional arg)
  (interactive "P")
  (recenter arg)
  (oz-fontify-buffer))


;;------------------------------------------------------------
;; Filtering Process Output
;;------------------------------------------------------------
;; Under Win32 the emulator is started by the compiler, so its output
;; goes into the compiler buffer.  The compiler makes sure that the
;; compiler and emulator outputs are always separated by special
;; characters so that the emulator output can be redirected into
;; its own buffer.
;;
;; If you ever change the corresponding constants, also adapt
;; Compiler/frontend/cFunctions.cc accordingly.

(defvar oz-emulator-output-start (char-to-string 5)
  "Regex that matches when emulator output begins.")
(defvar oz-emulator-output-end   (char-to-string 6)
  "Regex that matches when compiler output begins.")

(defun oz-have-to-switch-outbuffer (string)
  "Return the column from which output has to go into the other buffer.
Return nil if the whole STRING goes into the current buffer."
  (let ((regex (if oz-read-emulator-output
		   oz-emulator-output-end
		 oz-emulator-output-start)))
    (string-match regex string)))

(defun oz-current-outbuffer ()
  "Return the buffer into which the current output has to be redirected."
  (if oz-read-emulator-output
      oz-emulator-buffer
    oz-compiler-buffer))

(defun oz-compiler-filter (proc string)
  "Filter for Oz Compiler output.
Since under Win32 the output from both Emulator and Compiler comes from
the same process, this filter separates the output into two buffers.
The rest of the output is then passed through the oz-filter."
  (if (not oz-win32)
      (oz-filter proc string (process-buffer proc))
    (let ((switch-col (oz-have-to-switch-outbuffer string)))
      (if (null switch-col)
	  (oz-filter proc string (oz-current-outbuffer))
	(oz-filter proc (substring string 0 switch-col) (oz-current-outbuffer))
	(setq oz-read-emulator-output (not oz-read-emulator-output))
	(oz-compiler-filter proc (substring string (1+ switch-col)))))))

(defun oz-emulator-filter (proc string)
  (if (not oz-using-new-compiler)
      (oz-filter proc string (process-buffer proc))
    (let ((switch-col (oz-have-to-switch-outbuffer string)))
      (if (null switch-col)
	  (oz-filter proc string (oz-current-outbuffer))
	(oz-filter proc (substring string 0 switch-col) (oz-current-outbuffer))
	(setq oz-read-emulator-output (not oz-read-emulator-output))
	(oz-emulator-filter proc (substring string (1+ switch-col)))))))

;; See elisp manual: "Filter Functions"
(defun oz-filter (proc string newbuf)
  (let ((old-buffer (current-buffer))
	(errs-found (and oz-popup-on-error
			 (string-match oz-error-string string))))
    (unwind-protect
	(let (moving old-point index)
	  (set-buffer newbuf)
	  (setq moving (= (point) (process-mark proc)))
	  (save-excursion

	    ;; Insert the text, moving the process-marker.
	    (goto-char (process-mark proc))

	    (setq old-point (point))
	    (goto-char (point-max))

	    ;; Irix outputs garbage when sending EOF
	    (setq index (string-match "\\^D" string))
	    (if index
		(setq string (concat (substring string 0 index)
				     (substring string (+ 4 index)))))

	    (insert-before-markers string)
	    (set-marker (process-mark proc) (point))

	    (goto-char old-point)

	    ;; remove escape characters
	    (while (search-forward-regexp oz-remove-pattern nil t)
	      (replace-match "" nil t))

	    ;; oz-show-temp?
	    (while (search-forward-regexp oz-show-temp-pattern nil t)
	      (let ((file (match-string 1)) buf)
		(replace-match "" nil t)
		(setq buf (get-buffer oz-temp-buffer))
		(if (null buf)
		    (setq buf (generate-new-buffer oz-temp-buffer)))
		(oz-show-buffer buf)
		(save-excursion
		  (set-buffer buf)
		  (erase-buffer)
		  (insert-file-contents file)
		  (delete-file file)
		  (cond ((string-match "\\.ozc$" file)
			 (oz-mode))
			((string-match "\\.ozm$" file)
			 (ozm-mode))))))

	    ;; oz-bar information?
	    (while (search-forward-regexp oz-bar-pattern nil t)
	      (let ((file  (match-string 1))
		    (line  (string-to-number (match-string 2)))
		    (state (match-string 3)))
		(replace-match "" nil t)
		(if (string-equal file "undef")
		    (cond (oz-bar-overlay
			   (cond (oz-gnu-emacs
				  (delete-overlay oz-bar-overlay))
				 (oz-lucid-emacs
				  (delete-extent oz-bar-overlay)))
			   (setq oz-bar-overlay nil)))
		  (oz-bar file line state)))))

	  (if (or moving errs-found) (goto-char (process-mark proc))))
      (set-buffer old-buffer))

    (if errs-found (oz-show-buffer newbuf))))


;;------------------------------------------------------------
;; Buffers
;;------------------------------------------------------------

(defvar oz-other-buffer-percent 35
  "*Percentage of screen to use for Oz Compiler and Emulator windows.")

(defun oz-show-buffer (buffer)
  (if (get-buffer-window buffer)
      t
    (save-excursion
      (let ((win (or (get-buffer-window oz-emulator-buffer)
		     (get-buffer-window oz-compiler-buffer)
		     (get-buffer-window oz-temp-buffer)
		     (split-window (get-largest-window)
				   (/ (* (window-height (get-largest-window))
					 (- 100 oz-other-buffer-percent))
				      100)))))
	(set-window-buffer win buffer)
	(set-buffer buffer)
	(set-window-point win (point-max))
	(bury-buffer buffer)))))

(defun oz-create-buffer (buffer which)
  (save-excursion
    (set-buffer (get-buffer-create buffer))
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
    (oz-set-mouse-error-key)
    (if oz-lucid-emacs
     (set-buffer-menubar (append current-menubar oz-menubar)))
    (delete-region (point-min) (point-max))))


(defun oz-toggle-compiler ()
  "Toggle Oz Compiler window.
If the compiler window is not visible, then show it.
If it is, then remove it."
  (interactive)
  (oz-toggle-window oz-compiler-buffer))

(defun oz-toggle-emulator ()
  "Toggle Oz Emulator window.
If the emulator window is not visible, then show it.
If it is, then remove it."
  (interactive)
  (oz-toggle-window oz-emulator-buffer))

(defun oz-toggle-temp ()
  "Toggle Oz Temp window.
If the temp window is not visible, then show it.
If it is, then remove it."
  (interactive)
  (oz-toggle-window oz-temp-buffer))

(defun oz-toggle-window (buffername)
  (let ((buffer (get-buffer buffername)))
    (if buffer
	(let ((win (get-buffer-window buffername nil)))
	  (if win
	      (save-excursion
		(set-buffer buffer)
		(if (= (window-point win) (point-max))
		  (if oz-gnu-emacs
		      (delete-windows-on buffername t)
		    (delete-windows-on buffername))
		  (set-window-point win (point-max))))
	    (oz-show-buffer (get-buffer buffername)))))))


(defun oz-new-buffer ()
  "Create a new buffer and edit it in oz-mode."
  (interactive)
  (switch-to-buffer (generate-new-buffer "Oz"))
  (oz-mode))

(defun oz-previous-buffer ()
  "Switch to the next buffer in the buffer list which runs in oz-mode."
  (interactive)
  (bury-buffer)
  (oz-walk-through-buffers (buffer-list)))

(defun oz-next-buffer ()
  "Switch to the last buffer in the buffer list which runs in oz-mode."
  (interactive)
  (oz-walk-through-buffers (reverse (buffer-list))))

(defun oz-walk-through-buffers (buffers)
  (let ((none-found t) (cur (current-buffer)))
    (while (and buffers none-found)
      (set-buffer (car buffers))
      (if (or (equal mode-name "Oz")
	      (equal mode-name "Oz-Gump")
	      (equal mode-name "Oz-Machine"))
	  (progn
	    (switch-to-buffer (car buffers))
	    (setq none-found nil))
	(setq buffers (cdr buffers))))
    (if none-found
	(progn
	  (set-buffer cur)
	  (error "This is the only Oz buffer")))))


;;------------------------------------------------------------
;; Misc Goodies
;;------------------------------------------------------------

(defun oz-comment-region (start end arg)
  (interactive "r\np")
  (comment-region start end arg))

(defun oz-uncomment-region (start end arg)
  (interactive "r\np")
  (comment-region start end (if (= arg 0) -1 (- arg))))

(defvar oz-temp-file (oz-make-temp-name "/tmp/ozemacs"))

(defun oz-to-coresyntax-buffer ()
  (interactive)
  (oz-to-coresyntax-region (point-min) (point-max)))

(defun oz-to-coresyntax-line (arg)
  (interactive "p")
  (let ((region (oz-line-region arg)))
    (oz-to-coresyntax-region (car region) (cdr region))))

(defun oz-to-coresyntax-paragraph (arg)
  (interactive "p")
  (let ((region (oz-paragraph-region arg)))
    (oz-to-coresyntax-region (car region) (cdr region))))

(defun oz-to-coresyntax-region (start end)
  (interactive "r")
  (oz-directive-on-region start end "\\core" ".ozc"))

(defun oz-to-emulatorcode-buffer ()
  (interactive)
  (oz-to-emulatorcode-region (point-min) (point-max)))

(defun oz-to-emulatorcode-line (arg)
  (interactive "p")
  (let ((region (oz-line-region arg)))
    (oz-to-emulatorcode-region (car region) (cdr region))))

(defun oz-to-emulatorcode-paragraph (arg)
  (interactive "p")
  (let ((region (oz-paragraph-region arg)))
    (oz-to-emulatorcode-region (car region) (cdr region))))

(defun oz-to-emulatorcode-region (start end)
  (interactive "r")
  (oz-directive-on-region start end "\\machine" ".ozm"))

(defun oz-directive-on-region (start end directive suffix)
  "Applies a directive to the region."
  (let ((file-1 (concat oz-temp-file ".oz"))
	(file-2 (concat oz-temp-file suffix)))
    (if (file-exists-p file-2)
	(delete-file file-2))
    (save-excursion
      (let ((string (oz-get-region start end)))
	(set-buffer (generate-new-buffer oz-temp-buffer))
	(insert string)
	(write-region (point-min) (point-max) file-1 nil 'quiet)
	(kill-buffer (current-buffer))))
    (if oz-using-new-compiler
	(oz-send-string (concat directive " '" file-1 "'"))
      (let ((buf (get-buffer oz-temp-buffer)))
	(if buf
	    (save-excursion
	      (set-buffer buf)
	      (erase-buffer))))
      (shell-command (concat "touch " file-2))
      (start-process "Oz Temp" oz-temp-buffer "tail" "+1f" file-2)
      (message nil)
      (oz-send-string (concat directive " '" file-1 "'"))
      (let ((buf (get-buffer oz-temp-buffer)))
	(oz-show-buffer buf)
	(cond ((string-match "\\.ozc$" file-2)
	       (save-excursion
		 (set-buffer buf)
		 (oz-mode)))
	      ((string-match "\\.ozm$" file-2)
	       (save-excursion
		 (set-buffer buf)
		 (ozm-mode)))))))
  (oz-zmacs-stuff))

(defun oz-feed-region-browse (start end)
  "Feed the current region to the Oz Compiler.
Assuming it to contain an expression, it is enclosed by an application
of the procedure Browse."
  (interactive "r")
  (let ((contents (oz-get-region start end)))
    (oz-send-string (concat "{Browse\n" contents "}")))
  (oz-zmacs-stuff))

(defun oz-feed-line-browse (arg)
  "Feed the current line to the Oz Compiler.
Assuming it to contain an expression, it is enclosed by an application
of the procedure Browse."
  (interactive "p")
  (let ((region (oz-line-region arg)))
    (oz-feed-region-browse (car region) (cdr region))))

(defun oz-view-panel ()
  "Feed `{Panel open}' to the Oz Compiler."
  (interactive)
  (oz-send-string "{Panel open}"))

(defun oz-compiler-panel ()
  "Feed `{`Compiler` openPanel(Tk TkTools Open Browse)}' to the Oz Compiler."
  (interactive)
  (if oz-using-new-compiler
      (oz-send-string "{`Compiler` openPanel(Tk TkTools Open Browse)}")
    (error "Compiler panel not supported with the old compiler")))

(defun oz-feed-file (file)
  "Feed a file to the Oz Compiler."
  (interactive "FFeed file: ")
  (oz-send-string (concat "\\threadedfeed '" file "'")))

(defun oz-precompile-file (file)
  "Precompile an Oz file."
  (interactive "FPrecompile file: ")
  (if oz-using-new-compiler
      (error "Precompiling not supported with the new compiler")
    (oz-send-string (concat "\\precompile '" file "'"))))

(defun oz-find-dvi-file ()
  "View a file from the Oz documentation directory."
  (interactive)
  (let ((name (read-file-name "View documentation file: " oz-doc-dir nil t)))
    (if (file-exists-p name)
	(start-process "Oz Doc" "*Preview*" oz-preview name)
      (error "File %s does not exist" name))))

(defun oz-find-docu-file ()
  "Find a text in the Oz documentation directory."
  (interactive)
  (oz-find-file "Find doc file: " "doc/"))

(defun oz-find-demo-file ()
  "Find an Oz file in the demo directory."
  (interactive)
  (oz-find-file "Find demo file: " "demo/"))

(defun oz-find-docdemo-file ()
  "Find an Oz file in the demo/documentation directory."
  (interactive)
  (oz-find-file "Find demo file: " "demo/documentation/"))

(defun oz-find-modules-file ()
  "Find an Oz file in the lib directory."
  (interactive)
  (oz-find-file "Find modules file: " "lib/"))

(defun oz-find-file (prompt file)
  (find-file (read-file-name prompt (concat (oz-home) "/" file) nil t nil)))


;;------------------------------------------------------------
;; Locating Errors
;; Author: Jochen Doerre
;;------------------------------------------------------------
;; The compile.el stuff is used only a little bit; it cannot be made
;; working, if an error message does not contain file information,
;; as is the case with oz-feed-region.
;;
;; Functionality:
;; oz-goto-next-error (C-x ` in compiler, emulator and *.oz buffers)
;; Visit next compilation error message and corresponding source code.
;; Applies to most recent compilation, started with one of the feed
;; commands.  However, if called in compiler or emulator buffer, it
;; visits the next error message following point (no matter whether
;; that came from the latest compilation or not).

;; In the compiler and emulator buffers, button mouse-2 invokes
;; oz-mouse-goto-error as well:
(defun oz-set-mouse-error-key ()
  (let ((map (current-local-map)))
    (if map
	(if oz-lucid-emacs
	    (define-key map [(shift button2)] 'oz-mouse-goto-error)
	  (define-key map [(shift mouse-2)] 'oz-mouse-goto-error)))))

(defun oz-mouse-goto-error (event)
  (interactive "e")
  (set-buffer (if oz-lucid-emacs
		  (event-buffer event)
		(window-buffer (posn-window (event-end event)))))
  (goto-char (if oz-lucid-emacs
		 (event-closest-point event)
	       (posn-point (event-end event))))
  (or (eq (current-buffer) (get-buffer oz-compiler-buffer))
      (eq (current-buffer) (get-buffer oz-emulator-buffer))
      (error "Neither in compiler buffer nor in emulator buffer"))
  (oz-goto-next-error))


(defun oz-goto-next-error ()
  "Visit next compilation error message and corresponding source code.
Applies to most recent compilation, started with one of the feed
commands.

When in compiler buffer, visit error message surrounding point.
When in emulator buffer, visit place indicated in next callstack line."
  (interactive)
  (let ((old-buffer (current-buffer))
	(comp-buffer (get-buffer oz-compiler-buffer))
	(emu-buffer (get-buffer oz-emulator-buffer))
	error-data)
    (save-excursion
      (cond ((eq old-buffer comp-buffer)
	     (oz-goto-error-start)
	     (setq error-data (oz-fetch-next-error-data)))
	    ((eq old-buffer emu-buffer)
	     (setq error-data (oz-fetch-next-callstack-data)))
	    ((bufferp comp-buffer)
	     (set-buffer comp-buffer)
	     (cond ((and oz-next-error-marker
			 (eq (marker-buffer oz-next-error-marker) comp-buffer))
		    (goto-char oz-next-error-marker))
		   ;; else new compilation
		   ((and (<= (point-min) oz-compiler-output-start)
			 (<= oz-compiler-output-start (point-max)))
		    (goto-char oz-compiler-output-start)
		    (setq oz-next-error-marker (point-marker)))
		   (t (error "No compilation found")))
	     (setq error-data (oz-fetch-next-error-data)))))
    (if error-data
	(let* ((error-marker (car error-data))
	       (file (nth 1 error-data))
	       (lineno-string (nth 2 error-data))
	       (lineno (car (read-from-string lineno-string)))
	       (column-string (nth 3 error-data))
	       (column (and column-string
			    (car (read-from-string column-string))))
	       (buf (if (string-equal file "nofile")
			oz-last-fed-buffer
		      (compilation-find-file error-marker file nil)))
	       source-marker)
	  (if (not buf)
	      (error "No source buffer found"))
	  (set-buffer buf)
	  (save-excursion
	    (save-restriction
	      (widen)
	      (goto-line lineno)
	      (if (and column (> column 0)) (forward-char column))
	      (setq source-marker (point-marker))))
	  (compilation-goto-locus (cons error-marker source-marker)))
      (message "No next error")
      (sit-for 1))))

(defun oz-fetch-next-error-data ()
  (cond ((re-search-forward oz-error-intro-pattern nil t)
	 (beginning-of-line)
	 (let ((error-marker (point-marker)))
	   (if (re-search-forward oz-error-pattern nil t)
	       (let ((file (or (match-string 2) "nofile"))
		     (lineno-string (match-string 3))
		     (column-string (match-string 5)))
		 (setq oz-next-error-marker (point-marker))
		 (list error-marker file lineno-string column-string))
	     (let ((win (get-buffer-window oz-compiler-buffer nil)))
	       (if win
		   (set-window-point win (point-max))))
	     nil)))
	(t
	 (let ((win (get-buffer-window oz-compiler-buffer nil)))
	   (if win
	       (set-window-point win (point-max))))
	 nil)))

(defun oz-fetch-next-callstack-data ()
  (beginning-of-line)
  (if (re-search-forward oz-error-pattern nil t)
      (let ((file (or (match-string 2) "nofile"))
	    (lineno-string (match-string 3))
	    (column-string (match-string 5)))
	(setq oz-next-error-marker (point-marker))
	(beginning-of-line)
	(list (point-marker) file lineno-string column-string))))

;; if point is in the middle of an error message (in the compiler buffer),
;; then it is moved to the start of the message.
(defun oz-goto-error-start ()
  (let ((errstart
	 (save-excursion
	   (beginning-of-line)
	   (if (looking-at "%\\*\\*")
	       (re-search-backward oz-error-intro-pattern nil t)))))
    (if errstart (goto-char errstart))))


(provide 'oz)
