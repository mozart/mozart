(load "gdb")

(defun set-machine()
  (interactive)
  (setenv "OZMACHINE" 
	  (expand-file-name 
	   (read-file-name "Choose Machine: "
			   nil
			   nil
			   t
			   nil))))

(defvar slow-oz-machine "oz.machine.bin")
(defvar fast-oz-machine "oz.machine.bin")

(defun set-gdb-machine ()
  (interactive)
  (if (getenv "OZ_PI")
      t
    (message "setting OZPATH/PATH") (sleep-for 1)
    (setenv "OZ_PI" "1")
    (setenv "OZHOME" (or (getenv "OZHOME") "/usr/share/gs/soft/oz"))
    (setenv "OZPATH" 
	    (concat (or (getenv "OZPATH") ".") ":"
		    (getenv "OZHOME") "/lib:"
		    (getenv "OZHOME") "/lib/sun4:"
		    (getenv "OZHOME") "/demo"))
    (setenv "PATH"
	    (concat (getenv "PATH") ":" (getenv "OZHOME") "/bin")))


  (if oz-machine-hook
      (if (string= gdb-oz-machine fast-oz-machine)
	  (setq oz-machine-hook nil)
	(setq gdb-oz-machine fast-oz-machine)
	)
    (setq oz-machine-hook 'gdb-machine)
    (setq gdb-oz-machine slow-oz-machine))

  (if oz-machine-hook
      (message "set gdb machine: %s"  gdb-oz-machine)
    (message "set global machine")))


(global-set-key "\C-cm" 'set-machine)
(global-set-key "\C-cd" 'set-gdb-machine)
