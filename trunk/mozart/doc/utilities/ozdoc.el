;;; Author:
;;;   Leif Kornstaedt <kornstae@ps.uni-sb.de>
;;;
;;; Copyright:
;;;   Leif Kornstaedt, 1998
;;;
;;; Last change:
;;;   $Date$ by $Author$
;;;   $Revision$
;;;
;;; This file is part of Mozart, an implementation of Oz 3:
;;;   $MOZARTURL$
;;;
;;; See the file "LICENSE" or
;;;   $LICENSEURL$
;;; for information on usage and redistribution
;;; of this file, and for a DISCLAIMER OF ALL
;;; WARRANTIES.
;;;

(require 'cl)

(defvar ozdoc-fontified-buffer nil)
(defvar ozdoc-target-buffer nil)

(defvar ozdoc-line-number-rate nil
  "*Either an integer or nil to disable numbering.")

(defun ozdoc-usage (arg)
  (message "Usage: ozdoc-fontify <target> <input file> <output file>")
  (kill-emacs arg))

(defun ozdoc-target-insert-string (string)
  (save-excursion
    (set-buffer ozdoc-target-buffer)
    (insert string)))

(defun ozdoc-match-string (num &optional string)
  "Return string of text matched by last search.
NUM specifies which parenthesized expression in the last regexp.
 Value is nil if NUMth pair didn't match, or there were less than NUM pairs.
Zero means the entire text matched by the whole regexp or whole string.
STRING should be given if the last search was by `string-match' on STRING."
  (if (match-beginning num)
      (if string
	  (substring string (match-beginning num) (match-end num))
	(buffer-substring (match-beginning num) (match-end num)))))

;;; Converting to HTML
;;;--** currently ignores ozdoc-line-number-rate

(defvar ozdoc-htmlize-character-table
  (let ((table (make-vector 256 ?\0)))
    (dotimes (i 256)
      (setf (aref table i) (char-to-string i)))
    (setf (aref table ?&) "&amp;"
	  (aref table ?<) "&lt;"
	  (aref table ?>) "&gt;"
	  (aref table ? ) "&nbsp;")
    table))

(defun ozdoc-htmlize-protect (string)
  (mapconcat (lambda (char)
	       (aref ozdoc-htmlize-character-table char))
	     string ""))

(defun ozdoc-htmlize-face-color-string (face)
  (if (eq face 'font-lock-comment-face) "#B22222"
    (if (eq face 'font-lock-keyword-face) "#A020F0"
      (if (eq face 'font-lock-string-face) "#BC8F8F"
	(if (eq face 'font-lock-function-name-face) "#0000FF"
	  (if (eq face 'font-lock-type-face) "#228B22"
	    (if (eq face 'font-lock-variable-name-face) "#B8860B"
	      (if (eq face 'font-lock-reference-face) "#5F9EA0"
		"#000000"))))))))   ; or nil to not select a color

(defun ozdoc-htmlize-face-mono-string (face)
  (if (eq face 'font-lock-comment-face) "I"
    (if (eq face 'font-lock-keyword-face) "B"
      (if (eq face 'font-lock-function-name-face) "U"
	(if (eq face 'font-lock-type-face) "U"
	  nil)))))

(defun ozdoc-htmlize-generate (iscolor start end face)
  (let ((color-or-tag-name
	 (if iscolor
	     (ozdoc-htmlize-face-color-string face)
	   (ozdoc-htmlize-face-mono-string face))))
    (if color-or-tag-name
	(if iscolor
	    (ozdoc-target-insert-string
	     (concat "<FONT color=\"" color-or-tag-name "\">"))
	  (ozdoc-target-insert-string
	   (concat "<" color-or-tag-name ">"))))
    (ozdoc-target-insert-string
     (ozdoc-htmlize-protect (buffer-substring start end)))
    (if color-or-tag-name
	(if iscolor
	    (ozdoc-target-insert-string "</FONT>")
	  (ozdoc-target-insert-string
	   (concat "</" color-or-tag-name ">"))))))

(defun ozdoc-htmlize-buffer (iscolor)
  (while (not (eobp))
    (let* ((plist (text-properties-at (point)))
	   (next-change (or (next-property-change (point) (current-buffer))
			    (point-max)))
	   (face-list (plist-get plist 'face))
	   (face (cond ((consp face-list) (car face-list))
		       ((null face-list) 'default)
		       (t face-list))))
      (ozdoc-htmlize-generate iscolor (point) next-change face)
      (goto-char next-change))))

;;; Converting to LaTeX

(defvar ozdoc-latexize-functions
  (list (cons 'quote
	      (function (lambda (string)
			  (ozdoc-latexize-quote string nil))))
	(cons 'string-quote
	      (function (lambda (string)
			  (ozdoc-latexize-quote string t))))
	(cons 'typewriter
	      (function (lambda (string)
			  (concat "{\\ttfamily " string "}"))))
	(cons 'bold
	      (function (lambda (string)
			  (concat "{\\ttbf " string "}"))))
	(cons 'italic
	      (function (lambda (string)
			  (concat "{\\itshape " string "\\/}"))))
	(cons 'underline
	      (function (lambda (string)
			  (concat "\\underline{" string "}"))))))

(defvar ozdoc-latexize-alist
  '((default                      quote        typewriter)
    (font-lock-keyword-face       quote        bold)
    (font-lock-comment-face       quote        typewriter italic)
    (font-lock-string-face        string-quote typewriter)
    (font-lock-function-name-face quote        typewriter underline)
    (font-lock-type-face          quote        typewriter)
    (font-lock-variable-name-face quote        typewriter)
    (font-lock-reference-face     quote        typewriter bold italic)))

(defun ozdoc-latexize-newline (point)
  (if ozdoc-line-number-rate
      (let ((lineno (count-lines (point-min) point)))
	(if (or (= (count-lines (point-min) point) 1)
		(zerop (mod lineno ozdoc-line-number-rate)))
	    (format "\\hfill\\ {\\lineno%s}\n\n" lineno)
	  "\n\n"))
    "\n\n"))

(defun ozdoc-latexize-quote (string quote-space)
  ;; This should always be the first function in the property list,
  ;; since it depends on the correspondence between string indices
  ;; and buffer positions.
  (let ((length (length string))
	(new-string "")
	(start 0)
	(index 0))
    (while (< index length)
      (setq start index)
      (while (and (< index length)
		  (string-match "\\`[A-Za-z0-9]" (substring string index)))
	(setq index (1+ index)))
      (setq new-string
	    (concat new-string
		    (if (= index length)
			(substring string start length)
		      (let* ((old-index index)
			     (substr (substring string start old-index))
			     (char (aref string index)))
			(setq index (1+ index))
			(cond ((= char ?\n)
			       (concat substr (ozdoc-latexize-newline
					       (+ old-index (point)))))
			      ((= char ?\t)
			       (concat substr "\\ \\ \\ \\ \\ \\ \\ \\ "))
			      ((= char ?\f)
			       (concat substr "\\pagebreak{}"))
			      ((and (= char ? ) (not quote-space))
			       (concat substr "\\ "))
			      (t
			       (concat substr "\\char`\\"
				       (char-to-string char)))))))))
    new-string))

(defun ozdoc-latexize-transform-string (string properties)
  (if (null properties)
      string
    (let ((function (cdr (assoc (car properties) ozdoc-latexize-functions))))
      (ozdoc-latexize-transform-string (funcall function string)
				       (cdr properties)))))

(defun ozdoc-latexize-generate (start end face)
  (let (new-string)
    (save-excursion
      (goto-char start)   ; some functions in ozdoc-latexize-functions
			  ; depend on this
      (let ((string (buffer-substring start end))
	    (properties (cdr (assoc face ozdoc-latexize-alist))))
	(setq new-string
	      (ozdoc-latexize-transform-string string properties))))
    (ozdoc-target-insert-string new-string)))

(defun ozdoc-latexize-buffer ()
  (while (not (eobp))
    (let* ((plist (text-properties-at (point)))
	   (next-change (or (next-property-change (point) (current-buffer))
			    (point-max)))
	   (face-list (plist-get plist 'face))
	   (face (cond ((consp face-list) (car face-list))
		       ((null face-list) 'default)
		       (t face-list))))
      (ozdoc-latexize-generate (point) next-change face)
      (goto-char next-change))))

;;; Main Function

(defun ozdoc-fontify-command (target)
  (interactive "sTarget type: ")
  (goto-char (point-min))
  (setq ozdoc-fontified-buffer (get-buffer-create "*ozdoc-fontified*"))
  (setq ozdoc-target-buffer (get-buffer-create "*ozdoc-target*"))
  (while (looking-at "\\([^\004]+\\)\004")
    (let ((major-mode-to-set (intern (ozdoc-match-string 1))))
      (goto-char (match-end 0))
      (looking-at "\\([^\004]*\\)\004")
      (let ((string (ozdoc-match-string 1)))
	(goto-char (match-end 0))
	(save-excursion
	  (set-buffer ozdoc-fontified-buffer)
	  (erase-buffer)
	  (insert string)
	  (goto-char (point-min))
	  (if (fboundp major-mode-to-set)
	      (funcall major-mode-to-set))
	  (font-lock-fontify-buffer)
	  (cond ((string= target "html-color")
		 (ozdoc-htmlize-buffer t))
		((string= target "html-mono")
		 (ozdoc-htmlize-buffer nil))
		((string= target "latex")
		 (ozdoc-latexize-buffer))
		(t
		 (ozdoc-usage 2)))
	  (ozdoc-target-insert-string (char-to-string 4))))))
  (set-buffer ozdoc-target-buffer))

(defun ozdoc-fontify ()
  (if (not noninteractive)
      (error "Function ozdoc-fontify called from interactive mode"))
  (if (/= (length command-line-args-left) 3)
      (ozdoc-usage 2))
  (let ((target (car command-line-args-left))
	(infile (car (cdr command-line-args-left)))
	(outfile (car (cdr (cdr command-line-args-left))))
	(ozdoc-want-font-lock nil)
	(font-lock-verbose nil))
    (set-buffer (get-buffer-create "*ozdoc-source*"))
    (insert-file-contents infile)
    (ozdoc-fontify-command target)
    (write-file outfile)
    (kill-emacs 0)))
