%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Denys Duchier (Denys.Duchier@ps.uni-sb.de)
%%%
%%% Copyright:
%%%   Nils Franzén, 1999
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%declare AA=

functor
import
   Tk
export
   ScrollFrame
define
   class ScrollFrame from Tk.frame
      feat frame
      meth tkInit(width:W<=200 ...)=M
	 Canvas Vscroll
      in
	 Tk.frame, M

	 Canvas={New Tk.canvas tkInit(parent:self yscrollincrement:1 width:200)}
	 Vscroll={New Tk.scrollbar tkInit(parent:self orient:v width:9)}
	 {Tk.addYScrollbar Canvas Vscroll}
	 
	 self.frame = {New Tk.frame tkInit(parent:Canvas)}

	 {Canvas tk(create window 0 0 window:self.frame anchor:nw tags:foo)}
	 {self.frame tkBind(event:'<Configure>'
			    action:proc{$} [X Y W H]={Canvas tkReturnListInt(bbox all $)} in
				      {Canvas tk(configure scrollregion:q(X Y W H))}
				   end)}
	 {Canvas tkBind(event:'<Configure>'
			action:proc{$} W={Tk.returnInt winfo(width(Canvas))} in
				  {self.frame tk(conf width:W)}
				  {Canvas tk(itemconf foo width:W)}
			       end)}
	 {Tk.batch [grid(columnconfigure self 0 weight:1)
		    grid(rowconfigure self 0 weight:1)
		    grid(Canvas  row:0 column:0 sticky:nswe ipadx:1)
		    grid(Vscroll row:0 column:1 sticky:nsw)
		   ]}
      end
   end
end

/*

Put on top of page: declare AA=

declare
[A]={Module.apply [AA]}
T={New Tk.toplevel tkInit()}
F={New A.scrollFrame tkInit(parent:T bg:red)}
fun{NF} {New Tk.frame tkInit(parent:F.frame bd:1 relief:sunken height:2 width:10)} end
L={New Tk.message tkInit(parent:F.frame text:"Hello Sweet World!")}
{Tk.send pack(F {NF} L {NF} fill:x)}

*/

