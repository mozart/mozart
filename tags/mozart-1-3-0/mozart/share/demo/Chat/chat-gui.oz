%%%
%%% Authors:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Denys Duchier, 1998
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

functor
import
   Tk Application(exit:Exit)
export
   ChatWindow
define
   class ChatWindow from Tk.toplevel
      attr canvas y:0 vscroll hscroll tag:0 selfPort entry quit
      meth init(SelfPort)
         Tk.toplevel,tkInit
         selfPort <- SelfPort
         canvas <- {New Tk.canvas
                    tkInit(parent:self bg:ivory width:400 height:300)}
         vscroll <- {New Tk.scrollbar tkInit(parent:self orient:v)}
         hscroll <- {New Tk.scrollbar tkInit(parent:self orient:h)}
         entry   <- {New Tk.entry     tkInit(parent:self)}
         quit    <- {New Tk.button    tkInit(parent:self text:'Quit'
                                             action:proc{$} {Exit 0} end)}
         {Tk.addYScrollbar @canvas @vscroll}
         {Tk.addXScrollbar @canvas @hscroll}
         {@canvas tk(configure scrollregion:q(0 0 200 0))}
         {@entry tkBind(event:'<KeyPress-Return>'
                        action:proc {$} {self post} end)}
         {Tk.batch [grid(row:0 column:0 @canvas sticky:ns)
                    grid(row:1 column:0 @entry sticky:ew)
                    grid(row:0 column:1 @vscroll sticky:ns)
                    grid(row:2 column:0 @hscroll sticky:ew)
                    grid(row:3 column:0 @quit sticky:w)
                    grid(columnconfigure self 0 weight:1)
                    grid(rowconfigure self 0 weight:1)]}
      end
      meth show(TEXT)
         {@canvas tk(create text 0 @y text:TEXT anchor:nw tags:@tag)}
         local
            [X1 Y1 X2 Y2] = {@canvas tkReturnListInt(bbox all $)}
         in
            y<-Y2
            {@canvas tk(configure scrollregion:q(X1 Y1 X2 Y2))}
         end
      end
      meth post
         {Port.send @selfPort say({@entry tkReturn(get $)})}
         {@entry tk(delete 0 'end')}
      end
   end
end
