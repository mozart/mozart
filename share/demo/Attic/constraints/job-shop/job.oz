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
   fun {GetTaskName J T}
      {String.toAtom {VirtualString.toString j#J#t#T}}
   end

   fun {GetResourceName R}
      {String.toAtom {VirtualString.toString r#R}}
   end

   proc {TaskNameTo TN ?J ?T}
      S = {Atom.toString TN}.2
   in
      J = {String.toInt {List.takeWhile S Char.isDigit}}
      T = {String.toInt {List.dropWhile S Char.isDigit}.2}
   end
in
   class Job
      feat
         Number
         Parent
      attr
         Tasks:  nil
         NextX:  0

      meth init(parent:P number:N)
         self.Parent = P
         self.Number = N
         Tasks <- nil
         NextX <- 0
      end

      meth newTask(resource:R duration:D)
         Tasks <- {Append @Tasks
                   [{New Task
                     init(parent:   self.Parent
                          resource: R
                          duration: D
                          x:        @NextX
                          y:        (self.Number - 1) * JobDistance)}]}
         NextX <- @NextX + DurUnit * D
      end

      meth DelTask(Ts D $)
         case Ts of nil then nil
         [] T|Tr then
            case T==D then
               {ForAll Tr
                proc {$ T}
                   {T move(~{D getDuration($)} * DurUnit)}
                end} Tr
            else T|Job,DelTask(Tr D $)
            end
         end
      end

      meth deleteTask(D)
         {D tk(delete)}
         NextX <- @NextX - {D getDuration($)} * DurUnit
         Tasks <- Job,DelTask(@Tasks D $)
      end

      meth SetDur(Ts S D)
         case Ts of nil then skip
         [] T|Tr then
            case T==S then
               {ForAll Tr
                proc {$ T}
                   {T move((D-{S getDuration($)}) * DurUnit)}
                end}
            else Job,SetDur(Tr S D)
            end
         end
      end

      meth setDuration(T D)
         NextX <- @NextX + (D - {T getDuration($)}) * DurUnit
         Job,SetDur(@Tasks T D)
         {T setDuration(D)}
      end

      meth setSol(S)
         {Record.forAllInd S
          proc {$ A S}
             case A==pa orelse A==pe then skip
             else J T in
                {TaskNameTo A ?J ?T}
                case self.Number==J then {{Nth @Tasks T} setSol(S)}
                else skip
                end
             end
          end}
      end

      meth setEdit
         {ForAll @Tasks proc {$ T} {T setEdit} end}
      end

      meth getLastSpec($)
         case @Tasks of nil then nil else
            [{GetTaskName self.Number {Length @Tasks}}]
         end
      end

      meth getSpec($)
         {List.mapInd @Tasks
          fun {$ I T}
             {GetTaskName self.Number I} # {T getDuration($)} #
             case I==1 then [pa] else [{GetTaskName self.Number I-1}] end #
             {GetResourceName {T getResource($)}}
          end}
      end

   end

end
