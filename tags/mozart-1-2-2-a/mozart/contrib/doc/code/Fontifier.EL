;;;
;;; Author:
;;;   Denys Duchier <duchier@ps.uni-sb.de>
;;;   Leif Kornstaedt <kornstae@ps.uni-sb.de>
;;;
;;; Copyright:
;;;   Denys Duchier, 1998
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

;(require 'font-lock)
(if (fboundp 'font-lock-add-keywords)
    (font-lock-add-keywords
     'emacs-lisp-mode
     '(("(\\(setq\\)[ \t]+\\([^ \t\n]+\\)\\>"
	(1 font-lock-keyword-face)
	(2 font-lock-variable-name-face)))))

(setq font-lock-maximum-decoration t)

(defvar ozdoc-face-to-tag-alist
  '((font-lock-comment-face       . comment)
    (font-lock-keyword-face       . keyword)
    (font-lock-string-face        . string)
    (font-lock-variable-name-face . variable)
    (font-lock-function-name-face . function)
    (font-lock-builtin-face       . builtin)
    (font-lock-reference-face     . reference)
    (font-lock-constant-face      . reference)
    (font-lock-type-face          . type)
    (font-lock-warning-face       . warning)
    (nil                          . text)))

(defun ozdoc-face-to-tag (face)
  (cdr (assq face ozdoc-face-to-tag-alist)))

(defun ozdoc-fontify ()
  (let ((in  (nth 0 command-line-args-left))
	(out (nth 1 command-line-args-left))
	(oz-want-font-lock nil)
	(font-lock-verbose nil))
    (ozdoc-fontify-internal in out)))

;;; input file has the following structure:
;;;
;;; <FILE>	::= <REQUEST>*
;;; <REQUEST>	::= <MARK BEGIN> <MODE> <DESC> <MARK END>
;;; <MODE>	::= <TEXT>
;;; <DESC>	::= <MARK TEXT> <TEXT>
;;;		|   <MARK OPEN> <DESC>* <MARK CLOSE>
;;; <MARK BEGIN>::= ^D B
;;; <MARK END>	::= ^D E
;;; <MARK TEXT>	::= ^D T
;;; <MARK OPEN>	::= ^D O
;;; <MARK CLOSE>::= ^D C

;;; here we abstract the details of the <MARK ...> scheme
;;; so that it may easily be changed.

(defconst ozdoc-mark-begin "\004B")
(defconst ozdoc-mark-end   "\004E")
(defconst ozdoc-mark-text  "\004T")
(defconst ozdoc-mark-open  "\004O")
(defconst ozdoc-mark-close "\004C")

(defun ozdoc-next-mark ()
  "leaves point in front of next <MARK ...>"
  (skip-chars-forward "^\004"))

(defun ozdoc-after-mark ()
  "the point is moved over the <MARK ...>"
  (forward-char 2))

;;; parse next request in the input buffer

(defun ozdoc-parse-request ()
  "should be invoked in the buffer contain the sequence of
requests. the point should be positioned at the beginning of
a request. the request is parsed and a description of it is
returned as a tree and the point is left at the beginning of
the next entry.  The parsed representation of a request has
the form:

	(MODE SPEC)

where MODE is a symbol, and SPEC has the form:

	(simple P1 P2)
or	(complex SPEC1 ... SPECn)

where P1 and P2 are positions in the request buffer for a
span of textual data."
  (let (start mode spec)
    (if (not (looking-at ozdoc-mark-begin))
	(error "expected begin mark"))
    (ozdoc-after-mark)
    (setq start (point))
    (ozdoc-next-mark)
    (setq mode (intern (buffer-substring-no-properties
			start (point))))
    (setq spec (ozdoc-parse-spec))
    (if (not (looking-at ozdoc-mark-end))
	(error "expected end mark"))
    (ozdoc-after-mark)
    (list mode spec)))

(defun ozdoc-parse-spec ()
  (cond ((looking-at ozdoc-mark-text)
	 (ozdoc-parse-simple))
	((looking-at ozdoc-mark-open)
	 (ozdoc-parse-complex))
	(t (error "expected text or open mark"))))

(defun ozdoc-parse-simple ()
  (ozdoc-after-mark)
  (let ((start (point)))
    (ozdoc-next-mark)
    `(simple ,start ,(point))))

(defun ozdoc-parse-complex ()
  (ozdoc-after-mark)
  (let ((specs nil))
    (while (not (looking-at ozdoc-mark-close))
      (setq specs (cons (ozdoc-parse-spec) specs)))
    (ozdoc-after-mark)
    `(complex ,@(reverse specs))))

;;; process a spec, inserting corresponding text into the
;;; current buffer (the tmp-buffer). return a new spec where
;;; positions are into the current buffer.

(defun ozdoc-install-spec (spec)
  "takes a parsed SPEC as single arg (see ozdoc-parse-request).
The appropriate text is inserted into the current buffer with
text properties to encode the structure.  The data is taken from
src-buffer."
  (cond ((eq 'simple (car spec))
	 (ozdoc-install-simple spec))
	((eq 'complex (car spec))
	 (ozdoc-install-complex spec))
	(t (error "bad spec: %S" spec))))

(defun ozdoc-install-simple (request)
  (let ((p1 (nth 1 request))
	(p2 (nth 2 request))
	(p  (point)))
    (insert-buffer-substring src-buffer p1 p2)
    `(simple ,p ,(point))))

(defun ozdoc-install-complex (request)
  (let ((old-specs (cdr request))
	(new-specs nil))
    (while old-specs
      (setq new-specs
	    (cons (ozdoc-install-spec (car old-specs))
		  new-specs))
      (setq old-specs (cdr old-specs)))
    `(complex ,@(reverse new-specs))))

(defun ozdoc-examine-spec (spec)
  "takes a SPEC as single arg (see ozdoc-parse-request) and
examines the current buffer for face changes in the span
corresponding to SPEC. For a simple spec (simple P1 P2),
these face changes are returned as an alist:
	(simple (TAG . STRING) (TAG . STRING) ...)
where TAG is a symbol corresponding to the face and STRING
is the text that was thus highlighted.  For a complex spec,
we return (complex SPEC1 ... SPECn) where the result SPECs
are obtain by processing the corresponding SPECs in the
argument."
  (cond ((eq (car spec) 'simple)
	 (ozdoc-examine-simple spec))
	((eq (car spec) 'complex)
	 (ozdoc-examine-complex spec))
	(t (error "bad spec: %S" spec))))

(defun ozdoc-examine-simple (spec)
  (let ((p1 (nth 1 spec))
	(p2 (nth 2 spec))
	(alist nil) face next)
    (goto-char p1)
    (while (< (point) p2)
      (setq face (get-text-property (point) 'face))
      (setq next
	    (or (next-single-property-change (point) 'face nil p2)
		p2))
      (setq alist (cons (cons (ozdoc-face-to-tag face)
			      (buffer-substring-no-properties
			       (point) next))
			alist))
      (goto-char next))
    `(simple ,@(reverse alist))))

(defun ozdoc-examine-complex (spec)
  (let ((specs (cdr spec))
	(result nil))
    (while specs
      (setq result (cons (ozdoc-examine-spec (car specs)) result))
      (setq specs (cdr specs)))
    `(complex ,@(reverse result))))

;;; the output file has the following structure:
;;;
;;; <FILE>	::= <ANSWER>*
;;; <ANSWER>	::= <MARK BEGIN> <SPEC> <MARK END>
;;; <SPEC>	::= <SIMPLE>
;;;		|   <COMPLEX>
;;; <SIMPLE>	::= <MARK OPEN> S <ENTRY>* <MARK CLOSE>
;;; <COMPLEX>	::= <MARK OPEN> C <SPEC>*  <MARK CLOSE>
;;; <ENTRY>	::= <MARK TEXT> <TEXT> <MARK TEXT> <TEXT>
;;;
;;; in an <ENTRY> the first <TEXT> is the face and the 2nd
;;; one the data.

(defun ozdoc-output-answer (spec)
  "inserts into the current buffer a representation of
the result spec."
  (insert ozdoc-mark-begin)
  (ozdoc-output-spec spec)
  (insert ozdoc-mark-end))

(defun ozdoc-output-simple (spec)
  (let ((alist (cdr spec)))
    (insert ozdoc-mark-open ?S)
    (while alist
      (insert ozdoc-mark-text (symbol-name (car (car alist)))
	      ozdoc-mark-text (cdr (car alist)))
      (setq alist (cdr alist)))
    (insert ozdoc-mark-close)))

(defun ozdoc-output-complex (spec)
  (let ((specs (cdr spec)))
    (insert ozdoc-mark-open ?C)
    (while specs
      (ozdoc-output-spec (car specs))
      (setq specs (cdr specs)))
    (insert ozdoc-mark-close)))

(defun ozdoc-output-spec (spec)
  (cond ((eq (car spec) 'simple)
	 (ozdoc-output-simple spec))
	((eq (car spec) 'complex)
	 (ozdoc-output-complex spec))
	(t (error "bad spec: %S" spec))))

;;; process one request

(defun ozdoc-process-request ()
  "processes one request. must be invoked in src-buffer."
  (let (request mode spec)
    (setq request (ozdoc-parse-request))
    (save-excursion
     (set-buffer tmp-buffer)
     (kill-all-local-variables)
     (widen)
     (erase-buffer)
     (setq mode (car request))
     (setq mode (or (and (fboundp mode) mode)
		    (let ((m (intern-soft
			      (format "%s-mode" mode))))
		      (and m (fboundp m) m))
		    'fundamental-mode))
     (setq spec (car (cdr request)))
     (setq spec (ozdoc-install-spec spec))
     (funcall mode)
     (font-lock-fontify-buffer)
     (setq spec (ozdoc-examine-spec spec))
     (set-buffer out-buffer)
     (goto-char (point-max))
     (ozdoc-output-answer spec))))

(defun ozdoc-fontify-internal (in out)
  (let ((tmp-buffer (get-buffer-create "*ozdoc-tmp*"))
	(src-buffer (find-file-noselect in t t))
	(out-buffer (get-buffer-create "*ozdoc-out*"))
 	;(coding-system-for-write nil)
 	;(buffer-file-coding-system nil)
 	;(file-coding-system-alist nil)
 	(enable-local-variables nil)
	)
    (set-buffer src-buffer)
    (goto-char (point-min))
    (while (not (eobp))
      (ozdoc-process-request))
    (set-buffer out-buffer)
    (write-region (point-min) (point-max) out)))

;;; Local Variables: ***
;;; mode: emacs-lisp ***
;;; byte-compile-dynamic-docstrings: nil ***
;;; byte-compile-compatibility: t ***
;;; End: ***
