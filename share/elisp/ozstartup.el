;; ozstartup.el
;; ---------------------------------------------------------
;; a minimal ozstartup file:
;; setting oz-home, load-path and auto-mode-alist
;; starting oz

(defvar oz-home (or (getenv "OZHOME") ".")
  "directory where Oz was installed"
  )

;;; where lemacs searches for elisp files:
(setq load-path (append (list "." 
			      (concat oz-home "/lib/elisp"))
		      load-path))


;; if we distribute LEMACS with Oz, you need to add the following
;; where lemacs searches for online documentation
;(setq Info-directory-list (list (concat (ozhome) "/lemacs/info")))

;; where lemacs searches for online documentation too
;(setq exec-directory (concat (ozhome) "/lemacs/etc/"))


;; automatically switch into Oz-Mode when loading
;; files ending in ".oz"
(setq auto-mode-alist (cons '("\.oz$" . oz-mode)
			    auto-mode-alist))


(load "oz")
(run-oz)
