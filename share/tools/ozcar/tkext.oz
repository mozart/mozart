%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% some extensions to Tk widgets


/* a frame with a title */

class TitleFrame from Tk.frame
   feat Label
   meth tkInit(title:Title<="" ...)=M
      Tk.frame,{Record.subtract M title}
      case Title == "" then skip
      else
	 self.Label = {New Tk.label tkInit(parent:self
					   bd:1 relief:raised
					   text:Title)}
	 {Tk.send pack(self.Label side:top fill:x)}
      end
   end
   meth title(S)
      case {IsDet self.Label} then
	 {self.Label tk(conf text:S)}
      else skip end
   end
end
