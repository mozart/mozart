%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Emacs from UrObject
   %% prints an arrow at line L in file F
   %% loads a file if necessary
   meth displayLine(file:F line:L)
      {ThisEmacs
       eval('
	    (let* ((last-nonmenu-event t)
		   (buffer (find-file-noselect "' # F # '"))
		   (window (display-buffer buffer))
		   (pos))
	     (save-excursion
	      (set-buffer buffer)
	      (save-restriction
	       (widen)
	       (goto-line ' # L # ')
	       (setq pos (point))
	       (setq overlay-arrow-string "=>")
	       (or overlay-arrow-position
		   (setq overlay-arrow-position (make-marker)))
	       (set-marker overlay-arrow-position (point) (current-buffer)))
	      (cond ((or (< pos (point-min)) (> pos (point-max)))
		     (widen)
		     (goto-char pos))))
	     (set-window-point window overlay-arrow-position)))
       ' _)}
   end
end
