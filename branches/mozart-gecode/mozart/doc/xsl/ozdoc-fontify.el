;;;
(standard-display-european 1)

(if (fboundp 'font-lock-add-keywords)
    (font-lock-add-keywords
     'emacs-lisp-mode
     '(("(\\(setq\\)[ \t]+\\([^ \t\n]+\\)\\>"
	(1 font-lock-keyword-face)
	(2 font-lock-variable-name-face)))))

(autoload 'oz-mode "oz" nil t)
(autoload 'oz-gump-mode "oz" nil t)

(setq font-lock-maximum-decoration t)

(defvar ozdoc-face-to-name-alist
  '((font-lock-comment-face       . COMMENT)
    (font-lock-keyword-face       . KEYWORD)
    (font-lock-string-face        . STRING)
    (font-lock-variable-name-face . VARIABLE)
    (font-lock-function-name-face . FUNCTION)
    (font-lock-builtin-face       . BUILTIN)
    (font-lock-reference-face     . REFERENCE)
    (font-lock-constant-face      . REFERENCE)
    (font-lock-type-face          . TYPE)
    (font-lock-warning-face       . WARNING)
    (nil                          . TEXT)))

(defun ozdoc-face-to-name (face)
  (if (consp face) (setq face (car face)))
  (let ((entry (assq face ozdoc-face-to-name-alist)))
    (if (null entry) (message (format "*** UNKNOWN FACE: %s" face)))
    (cdr entry)))

(defun ozdoc-output (data) (princ data t))

(defconst ozdoc-xml-special-chars
  "[<>&\000-\010\013-\037\177-\377]")

(defvar ozdoc-encode-buffer
  (get-buffer-create "* Mozart Encode Buffer *"))

(defun ozdoc-encode-data (data)
  ;; we need to encode strange characters in a way that
  ;; is acceptable to XML
  (save-excursion
    (set-buffer ozdoc-encode-buffer)
    (erase-buffer)
    (insert data)
    (goto-char (point-min))
    (while (search-forward-regexp ozdoc-xml-special-chars nil t)
      (replace-match
       (format "&#%d;" (char-after (match-beginning 0)))
       t t))
    (buffer-string)))

(defun ozdoc-output-data (data)
  (ozdoc-output (ozdoc-encode-data data)))

(defun ozdoc-fontify (id mode data)
  (let ((font-lock-verbose nil))
    (save-excursion
      (set-buffer (ozdoc-get-buffer mode))
      (erase-buffer)
      (insert data)
      (font-lock-fontify-buffer)
      (ozdoc-output "<HILITE ID=\"")
      (ozdoc-output id)
      (ozdoc-output "\">")
      (let (face b e)
	(goto-char (point-min))
	(while (< (point) (point-max))
	  (setq b (point)
		face (get-text-property (point) 'face)
		e (or (next-single-property-change (point) 'face)
		      (point-max)))
	  (ozdoc-output "<HILITE.FACE NAME=\"")
	  (ozdoc-output (ozdoc-face-to-name face))
	  (ozdoc-output "\">")
	  (ozdoc-output-data (buffer-substring-no-properties b e))
	  (ozdoc-output "</HILITE.FACE>")
	  (goto-char e)))
      (ozdoc-output "</HILITE>\n"))))

(defvar oz-mode-alist
  '((gump . oz-gump-mode)
    (cc   . c++-mode)
    (elisp . emacs-lisp-mode)
    (sh    . sh-mode)))

(defun ozdoc-declare-mode (name mode)
  (setq name (intern (downcase name)))
  (setq oz-mode-alist (cons (cons name mode) oz-mode-alist)))

(defun ozdoc-to-mode (mode)
  (setq mode (intern (downcase mode)))
  (cond ((cdr (assq mode oz-mode-alist)))
	((fboundp mode) mode)
	((let ((m (intern-soft
		   (format "%s-mode" mode))))
	   (and m (fboundp m) m)))
	(t 'fundamental-mode)))

(defun ozdoc-get-buffer (mode)
  (let* ((mode (ozdoc-to-mode mode))
	 (name (format "* Mozart Fontifier: %s *" mode))
	 (buf (get-buffer name)))
    (if buf t
      (setq buf (get-buffer-create name))
      (save-excursion
	(set-buffer buf)
	(funcall mode)))
    buf))

(defun ozdoc-fontify-alist (mode alist)
  (let ((font-lock-verbose nil)
	(spans nil) b e face span id min max)
    (save-excursion
      (set-buffer (ozdoc-get-buffer mode))
      (erase-buffer)
      (while alist
	(setq b (point-marker))
	(insert (cdr (car alist)))
	(setq e (point-marker))
	(setq spans (cons (list (car (car alist)) b e) spans)
	      alist (cdr alist)))
      (untabify (point-min) (point-max))
      (font-lock-fontify-buffer)
      (setq spans (reverse spans))
      (while spans
	(setq span (car spans) spans (cdr spans))
	(setq id (car span) min (car (cdr span)) max (car (cdr (cdr span))))
	(ozdoc-output "<HILITE ID=\"")
	(ozdoc-output id)
	(ozdoc-output "\">")
	(goto-char min)
	(while (< (point) max)
	  (setq b (point)
		face (get-text-property (point) 'face)
		e (or (next-single-property-change (point) 'face nil max)
		      (point-max)))
	  (ozdoc-output "<HILITE.FACE NAME=\"")
	  (ozdoc-output (ozdoc-face-to-name face))
	  (ozdoc-output "\">")
	  (ozdoc-output-data (buffer-substring-no-properties b e))
	  (ozdoc-output "</HILITE.FACE>")
	  (goto-char e))
	(ozdoc-output "</HILITE>\n")))))

(defun ozdoc-fontify-file (mode id file)
  (ozdoc-fontify-alist mode (list (cons id (ozdoc-find-file file)))))

(defvar ozdoc-file-path '("."))

(defun ozdoc-find-file (file)
  (let ((dirs ozdoc-file-path) (found file) path)
    (while dirs
      (setq path (expand-file-name file (car dirs)))
      (if(file-exists-p path)
	  (setq dirs nil found path)
	(setq dirs (cdr dirs))))
    (save-excursion
      (set-buffer ozdoc-encode-buffer)
      (erase-buffer)
      (insert-file-contents-literally found)
      (buffer-string))))
