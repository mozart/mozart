%%%
%%% Authors:
%%%   Martin Mueller (mmueller@ps.uni-sb.de)
%%%
%%% Contributors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Martin Mueller, 1998
%%%   Christian Schulte, 1998
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

   Width       = 600
   Height      = 165
   TruckHeight = 60
   Free        = 25

   functor MakeTruckRace prop once

   import
      Tk

      TkTools

      Applet

   body

      Url = 'http://www.ps.uni-sb.de/ozhome/demo/images/trucks/'

      Images = {TkTools.images [Url#'truck-right.gif'
                                Url#'truck-left.gif']}

      class Truck

         prop
            final

         from
            Time.repeat Tk.canvasTag

         attr
            state:off
            step:1

         meth init(c:Canvas x:X<=0 y:Y<=0)
            Truck, tkInit(parent:Canvas)

            {Canvas tk(crea image X Y image:Images.'truck-right' tag:self)}

            {self setRepAll(delay:  100
                            number: Width div @step
                            action: moveTruck
                            final:  switch)}

            {self tkBind(event:'<1>' action:self#toggle)}
            {self tkBind(event:'<2>' action:self#change(~1))}
            {self tkBind(event:'<3>' action:self#change(1))}
         end

         meth moveTruck
            Truck, tk(move @step 0)
         end

         meth switch
            step <- ~1 * @step
            case @step>0
            then {self tk(itemconf image:Images.'truck-right')}
            else {self tk(itemconf image:Images.'truck-left')}
            end
            {self go}
         end

         meth toggle
            try
               case @state
               of off then state<-on  {self go}
               [] on  then state<-off {self stop}
               end
            catch system(...) then skip
            end
         end

         meth change(S)
            N = {self getRep(delay:$)}
         in
            {self setRepDelay({Max 2 N+S*(N div 2)})}
         end

         meth close
            Time.repeat,  stop
            Tk.canvasTag, tkClose
         end
      end

      C = {New Tk.canvas tkInit(parent: Applet.toplevel
                                bg:     white
                                width:  Width
                                height: Height)}
      {Tk.send pack(C fill:both)}

      thread {New Truck init(c:C y:Free)  _} end
      thread {New Truck init(c:C y:Free+TruckHeight)  _} end
      thread {New Truck init(c:C y:Free+TruckHeight*2) _} end

   end

in

   {Application.applet
    'trucks.oza'

    MakeTruckRace

    single(title(type:string default:"Truck Race"))
   }

end
