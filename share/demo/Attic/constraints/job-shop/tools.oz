%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   \insert 'resource-tool.oz'

   \insert 'duration-tool.oz'

in

   class Tools
      from TkTools.note
      feat Board
      attr Current:create(MaxRes 1)
      meth tkInit(parent:P board:B)
         TkTools.note,tkInit(parent:P text:'Edit')

         Var  = {New Tk.variable tkInit(create)}

         proc {ResA}
            {DurT disable} {Var tkSet(none)}
            Current <- resource(fun {$} {ResT getRes($)} end)
         end
         ResT = {New ResourceTool tkInit(parent:self action:ResA)}

         proc {DurA}
            {ResT disable} {Var tkSet(none)}
            Current <- duration(fun {$} {DurT getDur($)} end)
         end
         DurT = {New DurationTool tkInit(parent:self action:DurA)}

         proc {CreA}
            {ResT disable} {DurT disable}
            Current <- create({ResT getRes($)} {DurT getDur($)})
         end
         CreT = {New Tk.radiobutton tkInit(parent: self
                                           var:    Var
                                           val:    create
                                           text:   'Create Task'
                                           action: CreA
                                           font:   Helv
                                           relief: ridge
                                           bd:     2
                                           anchor: w)}

         proc {DelA}
            {ResT disable} {DurT disable}
            Current <- delete
         end

         DelT = {New Tk.radiobutton tkInit(parent: self
                                           var:    Var
                                           val:    delete
                                           text:   'Delete Task'
                                           action: DelA
                                           font:   Helv
                                           relief: ridge
                                           bd:     2
                                           anchor: w)}
      in
         {Tk.batch [pack(ResT DurT padx:2 fill:x)
                    pack(CreT DelT pady:1 ipadx:2 fill:x)]}
         {DurT disable}
         {ResT disable}
         self.Board = B
      end

      meth getTool($)
         @Current
      end

      meth toTop
         {self.Board setEdit}
         {Explorer.object close}
      end

   end

end
