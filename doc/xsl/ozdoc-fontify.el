;;;
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
  (cdr (assq face ozdoc-face-to-name-alist)))

(defun ozdoc-output (data) (princ data t))

(defconst ozdoc-xml-special-chars
  "[<>&\000-\010\013-\037\177-\376]")

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

(defun ozdoc-to-mode (mode)
  (setq mode (intern (downcase mode)))
  (cond ((fboundp mode) mode)
	((let ((m (intern-soft
		   (format "%s-mode" mode))))
	   (and m (fboundp m) m)))
	((eq mode 'gump ) 'oz-gump-mode)
	((eq mode 'cc   ) 'c++-mode)
	((eq mode 'elisp) 'emacs-lisp-mode)
	((eq mode 'sh   ) 'sh-mode)
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
	(setq b (point))
	(insert (cdr (car alist)))
	(setq e (point))
	(setq spans (cons (list (car (car alist)) b e) spans)
	      alist (cdr alist)))
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
