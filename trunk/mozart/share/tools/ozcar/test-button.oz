%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
T={New Tk.toplevel tkInit}
B={New Tk.button tkInit(parent:T action:proc {$}
					   {Debug.breakpoint}
					   {Show inAction}
					   thread
					      {Show a}
					   end
					   thread
					      {Show b}
					   end
					   {Show c}
					end)}
{Tk.send pack(B)}
