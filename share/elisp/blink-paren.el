;; Blink the matching paren, just like Zmacs.  By devin@lucid.com.
;; Copyright (C) 1992 Free Software Foundation, Inc.

;; This file is part of GNU Emacs.

;; GNU Emacs is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

(defvar blink-paren-timeout 0.2
  "*If the cursor is on a parenthesis, the matching parenthesis will blink.
This variable controls how long each phase of the blink lasts in seconds.
This should be a fractional part of a second (a float.)")


;;; The blinking paren alternates between the faces blink-paren-on and
;;; blink-paren-off.  The default is for -on to look just like default
;;; text, and -off to be invisible.  You can change this so that, for
;;; example, the blinking paren fluctuates between bold and italic...

(or (find-face 'blink-paren-on) (make-face 'blink-paren-on))
(or (find-face 'blink-paren-off) (make-face 'blink-paren-off))

(or (face-differs-from-default-p 'blink-paren-off)
    (progn
      (set-face-background 'blink-paren-off (face-background 'default))
      (set-face-foreground 'blink-paren-off (face-background 'default))))


;; extent used to change the face of the matching paren
(defvar blink-paren-extent ())

;; timeout to blink the face
(defvar blink-paren-timeout-id ())

;; find if we should look foward or backward to find the matching paren
(defun blink-paren-sexp-dir ()
  (cond ((and (< (point) (point-max))
	      (eq (char-syntax (char-after (point))) ?\())
	 1)
	((and (> (point) (point-min))
	      (eq (char-syntax (char-after (- (point) 1))) ?\)))
	 -1)
	(t ())))

;; make an extent on the matching paren if any.  return it.
(defun blink-paren-make-extent ()
  (let ((dir (blink-paren-sexp-dir)))
    (and dir
	 (condition-case ()
	     (let* ((other-pos (save-excursion (forward-sexp dir) (point)))
		    (extent (if (= dir 1)
				(make-extent (- other-pos 1) other-pos)
			      (make-extent other-pos (+ other-pos 1)))))
	       (set-extent-face extent 'blink-paren-on)
	       extent)
	   (error nil)))))

;; callback for the timeout
;; swap the face of the extent on the matching paren
(defun blink-paren-timeout (arg)
  ;; The extent could have been deleted for some reason and not point to a
  ;; buffer anymore.  So catch any error to remove the timeout.
  (condition-case ()
      (set-extent-face blink-paren-extent 
		       (if (eq (extent-face blink-paren-extent)
			       'blink-paren-on)
			   'blink-paren-off
			 'blink-paren-on))
    (error (blink-paren-pre-command))))

;; called after each command is executed in the post-command-hook
;; add the extent and the time-out if we are on a paren.
(defun blink-paren-post-command ()
  (blink-paren-pre-command)
  (if (and (setq blink-paren-extent (blink-paren-make-extent))
	   (not (and (face-equal 'blink-paren-on 'blink-paren-off)
		     (progn
		       (set-extent-face blink-paren-extent 'blink-paren-on)
		       t)))
	   (or (floatp blink-paren-timeout)
	       (integerp blink-paren-timeout)))
      (setq blink-paren-timeout-id
	    (add-timeout blink-paren-timeout 'blink-paren-timeout ()
			 blink-paren-timeout))))

;; called before a new command is executed in the pre-command-hook
;; cleanup by removing the extent and the time-out
(defun blink-paren-pre-command ()
  (condition-case c  ; don't ever signal an error in pre-command-hook!
      (let ((inhibit-quit t))
	(if blink-paren-timeout-id
	    (disable-timeout (prog1 blink-paren-timeout-id
			       (setq blink-paren-timeout-id nil))))
	(if blink-paren-extent
	    (delete-extent (prog1 blink-paren-extent
			     (setq blink-paren-extent nil)))))
    (error
     (message "blink paren error! %s" c))))


(defun blink-paren-init ()
  (add-hook 'pre-command-hook 'blink-paren-pre-command)
  (add-hook 'post-command-hook 'blink-paren-post-command)
  (setq blink-matching-paren nil)  ; don't need this loser any more
  )

;; go go go johnny go
(blink-paren-init)
