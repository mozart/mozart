%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   A = proc{$ X Y Z}
	  AA = proc{$}
		  %{Debug.breakpoint}
		  X = 42
		  {Show 'X'#X}
	       end
       in
	  thread
	     Z = X + Y
	     {Show 'Z'#Z}
	  end
	  {AA}
	  {B Z}
       end
   B = proc{$ X}
	  %{Debug.breakpoint}
	  Y = X + 1
       in
	  {Show 'Y'#Y}
       end
in
   {Browse {A _ 11 $}}
end
