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

class Task
   from Tk.canvasTag
   attr
      X0:0 Y0:0 X1:0
      Duration: 0
      Resource: unit
      EditMode: true

   meth init(parent:P resource:R duration:D x:X y:Y)
      Task, tkInit(parent:P)
      X0       <- X
      Y0       <- Y
      Duration <- D
      Resource <- R
      {P tk(create rectangle
            X                         Y - DurUnit div 2
            X + D*DurUnit - DurFrame  Y + DurUnit div 2
            fill:ResColors.R tags:self)}
      Task, tkBind(event:  '<1>'
                   args:   [int(y)]
                   action: P # action(self))
   end

   meth setDuration(D)
      Duration <- D
      Task, tk(coords
               @X0                      @Y0 - DurUnit div 2
               @X0 + D*DurUnit-DurFrame @Y0 + DurUnit div 2)
   end

   meth getDuration($)
      @Duration
   end

   meth setResource(R)
      Resource <- R
      Task, tk(itemconfigure fill:ResColors.R)
   end

   meth getResource($)
      @Resource
   end

   meth move(ByX)
      X0 <- @X0 + ByX
      Task,tk(move ByX 0)
   end

   meth setSol(S)
      X = S * DurUnit
   in
      case @EditMode then Task,tk(move X-@X0 0)
      else Task,tk(move X-@X1 0)
      end
      EditMode <- false
      X1       <- X
   end

   meth setEdit
      case @EditMode then skip else
         EditMode <- true
         Task,tk(move @X0-@X1 0)
      end
   end

end
