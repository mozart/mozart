(provide 'oz-server)

(defconst oz-server-name "*Oz Server*")
(defvar oz-server-process nil)

(defun oz-server-open (host port)
  (let ((process (open-network-stream
		  oz-server-name nil host port)))
    (setq oz-server-process process)
    (set-process-filter   process 'oz-server-filter)
    (set-process-sentinel process 'oz-server-sentinel)))

(defconst oz-server-eoa "\001")
(defconst oz-server-eom "\002")
(defconst oz-server-quo "\003")
(defconst oz-server-quo-skip (concat "^" oz-server-quo))
(defconst oz-server-all-skip (concat "^"
				     oz-server-eoa
				     oz-server-eom
				     oz-server-quo))

(defvar oz-server-buffered-input "")

(defun oz-server-filter (process input)
  (let ((i (string-match oz-server-eom input)))
    (if i
	(let ((prefix (substring input 0 i))
	      (suffix (substring input (1+ i)))
	      args)
	  (setq args (concat oz-server-buffered-input prefix))
	  (setq oz-server-buffered-input suffix)
	  (oz-server-process-message
	   (mapcar 'oz-server-unescape (split-string args oz-server-eoa))))
      (setq oz-server-buffered-input
	    (concat oz-server-buffered-input input)))))

(defun oz-server-unescape (string)
  (let ((b (get-buffer-create "*Oz Server Temp*")))
    (save-excursion
      (save-restriction
	(set-buffer b)
	(widen)
	(erase-buffer)
	(insert string)
	(goto-char (point-min))
	(let ((redo t))
	  (while redo
	    (skip-chars-forward oz-server-quo-skip)
	    (if (eobp)
		(setq redo nil)
	      (let ((c (- (char-after (1+ (point))) 100)))
		(delete-char 2)
		(insert c)))))
	(buffer-string)))))

(defun oz-server-escape (string)
  (let ((b (get-buffer-create "*Oz Server Temp*")))
    (save-excursion
      (save-restriction
	(set-buffer b)
	(widen)
	(erase-buffer)
	(insert string)
	(goto-char (point-min))
	(let ((redo t))
	  (while redo
	    (skip-chars-forward oz-server-all-skip)
	    (if (eobp)
		(setq redo nil)
	      (let ((c (+ (char-after (point)) 100)))
		(delete-char 1)
		(insert oz-server-quo c)))))
	(buffer-string)))))

(defun oz-server-sentinel (process status)
  (setq oz-server-process nil))

(defun oz-server-process-message (args)
  (let ((tag (intern (car args))))
    (cond ((eq tag 'reply)      (oz-server-process-reply (cdr args)))
	  ((eq tag 'replyError) (oz-server-process-replyError (cdr args)))
	  ((eq tag 'event)      (oz-server-process-event (cdr args)))
	  (t (error "unknown tag `%s'" tag)))))

(defun oz-server-alist-make ()
  (list 'ALIST))

(defun oz-server-alist-get (alist key)
  (cdr (assq key (cdr alist))))

(defun oz-server-alist-put (alist key val)
  (let ((e (assq key (cdr alist))))
    (if e (setcdr e val)
      (setcdr alist (cons (cons key val) (cdr alist))))))

(defun oz-server-alist-del (alist key)
  (let ((prev alist)
	(curr (cdr alist)))
    (while curr
      (if (eq (caar curr) key)
	  (progn
	    (setcdr prev (cdr curr))
	    (setq curr nil))
	(setq prev curr curr (cdr curr))))))

(defvar oz-server-callback-alist (oz-server-alist-make))
(defvar oz-server-handler-alist nil)

;;; a callback is of the form (FUNCTION . DATA)
;;; and is invoked as (FUNCTION ERRFLAG ARGS DATA)
;;; where ERRFLAG indicates if an error occurred
;;; and ARGS is a list of strings returned as a result

(defun oz-server-process-reply-generic (args errflag)
  (let* ((id (string-to-int (car args)))
	 (vals (cdr args))
	 (callback (oz-server-alist-get oz-server-callback-alist id)))
    (if callback
	(progn
	  (oz-server-alist-del oz-server-callback-alist id)
	  (condition-case nil
	      (funcall (car callback) errflag vals (cdr callback))
	    (error nil))))))

(defun oz-server-process-reply (args)
  (oz-server-process-reply-generic args nil))

(defun oz-server-process-replyError (args)
  (oz-server-process-reply-generic args t))

(defun oz-server-display-error (MSG)
  (let ((b (get-buffer-create "*Oz Server Errors*")))
    (save-excursion
      (save-restriction
	(set-buffer b)
	(widen)
	(let ((p (point-max)) w)
	  (goto-char p)
	  (insert MSG ?\n ?\n)
	  (setq w (display-buffer b))
	  (set-window-start w p))))))

(defun oz-server-process-event (args)
  (let* ((tag (intern (car args)))
	 (handler (cdr (assq tag oz-server-handler-alist))))
    (if handler
	(condition-case nil
	    (funcall handler tag (cdr args))
	  (error nil)))))

(defvar oz-server-query-count 0)

(defun oz-server-query (args callback data)
  (let* ((id oz-server-query-count)
	 (msg (mapcar 'oz-server-escape
		      (cons (int-to-string id) args))))
    (setq oz-server-query-count (1+ id))
    (oz-server-alist-put oz-server-callback-alist id
			 (cons callback data))
    (while msg
      (process-send-string
       oz-server-process (concat (car msg) oz-server-eoa))
      (setq msg (cdr msg)))
    (process-send-string
     oz-server-process oz-server-eom)))

(defvar oz-server-synchronous-polling nil)
(defvar oz-server-synchronous-reply   nil)

(defun oz-server-synchronous-callback (errflag vals data)
  (message "errflag %s" errflag)
  (setq oz-server-synchronous-polling nil)
  (setq oz-server-synchronous-reply (cons errflag vals)))

(defconst oz-server-polling-delay 0.05)

(defun oz-server-query-synchronously (args)
  (unwind-protect
      (progn
	(setq oz-server-synchronous-polling t)
	(oz-server-query args 'oz-server-synchronous-callback nil)
	(while oz-server-synchronous-polling
	  (sit-for oz-server-polling-delay))
	(let ((errflag (car oz-server-synchronous-reply))
	      (vals    (cdr oz-server-synchronous-reply)))
	  (if errflag
	      (signal 'oz-server-query vals)
	    vals)))
    (setq oz-server-synchronous-polling nil
	  oz-server-synchronous-reply   nil)))
