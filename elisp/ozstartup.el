;(setq debug-on-error t)

(defvar
  lucid-emacs 
  (string-match "Lucid" emacs-version)
  "Use Lucid-Emacs functions for highlighting for example")


(defun ozhome()
"Return directory where Oz was installed as a string"
  (or (getenv "OZHOME") ""))

;;; where lemacs searches for elisp files:
(setq load-path (append (list "." 
			      (concat (ozhome) "/elisp"))
		      load-path))

;;; where lemacs searches for online documentation
(setq Info-directory-list (list (concat (ozhome) "/lemacs/info")))

;;; where lemacs searches for online documentation too
(setq exec-directory (concat (ozhome) "/lemacs/etc/"))


;; 
;;  Oz Mode
;;
;(defvar oz-input-to-oz t)

(autoload 'oz-mode        "oz" "Mode for Oz" t)
(autoload 'run-oz         "oz" "Mode for Oz" t)
(autoload 'oz-previous-buffer "oz" "Mode for Oz" t)
(autoload 'oz-next-buffer "oz" "Mode for Oz" t)
(autoload 'oz-new-buffer  "oz" "Mode for Oz" t)

(global-set-key "\M-n"   'oz-next-buffer)
(global-set-key "\M-p"   'oz-previous-buffer)
(global-set-key "\M-o"   'oz-new-buffer)


;; automatically switch into Oz-Mode when loading
;; files ending in ".oz"
(setq auto-mode-alist (cons '("\.oz$" . oz-mode)
			    auto-mode-alist))


(put 'eval-expression 'disabled nil)


(defun button3-popup()
  (interactive)
  (popup-menu button3-menubar))

(defun buffer-menu-new-window()
  (interactive)
  (mybuffer-menu 'switch-to-buffer))

(defun buffer-menu-new-screen()
  (interactive)
  (mybuffer-menu 'switch-to-buffer-new-screen))

(defun mybuffer-menu (switcher)
  (let (name
	buffers)
    ;; sort the buffers alphabetically
    (setq buffers (sort (buffer-list)
			(function (lambda (x y) (string< (buffer-name x) (buffer-name y))))))
    (setq buffers
	  (mapcar (function
		   (lambda (buffer)
		     (if (setq name (format-buffers-menu-line buffer))
			 (vector name
				 (list switcher (buffer-name buffer))
				 t))))
		  buffers))
    (popup-menu  (cons "" (delq nil buffers)))))


(defun line-to-top()
  (interactive)
  (recenter 0))

(if lucid-emacs
    (progn 
      (global-set-key [(button3)]               'button3-popup)
      (global-set-key [(control button3)]       'buffer-menu-new-window)
      (global-set-key [(control shift button3)]       'buffer-menu-new-screen)
      (global-set-key [(button1)]               'mouse-track)
      (global-set-key [(f2)]                    'line-to-top)
      (global-set-key [(f3)]                    'goto-line-selected)
      (global-set-key [(control c) (g)]         'goto-line)))

(defun goto-line-selected()
"Goto line selected via X11, or prompt for line-number"
  (interactive)
  (let ((line (string-to-int 
	       (or (condition-case () (x-get-selection) (error ()))
		   "0"))))
    (if (= line 0)
        (call-interactively 'goto-line)
	(goto-line line))))

(setq server-visit-hook 'find-buffer-other-window)

(defun find-buffer-other-window ()
  (interactive)
  (let ((cur (current-buffer)))
    (x-new-screen)
    (switch-to-buffer cur)))


(defun switch-to-buffer-new-screen (buffer)
  (interactive "BSwitch to buffer in other screen: ")
  (let ((screens (screen-list))
	(screen)
	(win))
    (setq win (get-buffer-window buffer t) )
    (if win
	(progn
	  (select-window win)
	  (make-screen-visible (window-screen win))
	  (raise-screen  (window-screen win)))
	(progn
	  (x-new-screen)
	  (switch-to-buffer buffer)))))


(run-oz)
