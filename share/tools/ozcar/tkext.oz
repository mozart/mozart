%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% some extensions to Tk widgets


/* a frame with title */

class TitleFrame from Tk.frame
   meth tkInit(title:Title<="" ...)=M
      Tk.frame,{Record.subtract M title}
      case Title == "" then
	 skip
      else
	 {Tk.send pack({New Tk.label tkInit(parent:self
					    bd:1 relief:raised
					    text:Title)} side:top fill:x)}
      end
   end
end
