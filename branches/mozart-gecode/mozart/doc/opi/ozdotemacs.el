(or (getenv "OZHOME")
    (setenv "OZHOME"
	    "/usr/local/oz"))   ; or wherever Mozart is installed
(setenv "PATH" (concat (getenv "OZHOME") "/bin:" (getenv "PATH")))

(setq load-path (cons (concat (getenv "OZHOME") "/share/elisp")
		      load-path))

(setq auto-mode-alist
      (append '(("\\.oz\\'" . oz-mode)
		("\\.ozg\\'" . oz-gump-mode))
	      auto-mode-alist))

(autoload 'run-oz "oz" "" t)
(autoload 'oz-mode "oz" "" t)
(autoload 'oz-gump-mode "oz" "" t)
(autoload 'oz-new-buffer "oz" "" t)
