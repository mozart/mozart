%%%
%%% Authors:
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Contributor:
%%%   Jörg Würtz
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

import
   FD Schedule

export
   compile: Compile

define

   local
      fun {Insert Ts N P}
         case Ts of nil then [N#P]
         [] T|Tr then
            if N<T.1 then N#P|Ts else T|{Insert Tr N P} end
         end
      end
   in
      class DistributionManager
         prop final
         attr
            Tasks: nil

         meth init
            thread DistributionManager,Drive end
         end

         meth add(N P)
            NewTs OldTs
         in
            OldTs = (Tasks <- NewTs)
            NewTs={Insert OldTs N P}
         end

         meth Drive
            choice skip end
            case @Tasks of nil then skip
            [] T|Tr then
               Tasks <- Tr
               {T.2}
               DistributionManager,Drive
            end
         end
      end
   end


   fun {Compile Spec}

      %% Specification is as follows:
      %%  Spec.x, Spec.y: size of the target plate
      %%  Spec.squares.D=N: N squares of size D

      %% Number of all squares
      N  = {Record.foldL Spec.squares Number.'+' 0}
      %% Dimension of X and Y
      DX = Spec.x
      DY = Spec.y

      %% Mapping: Sqs -> Size
      SqsSize = {MakeTuple '#' N}

      %% Mapping: Sqs -> Area
      SqsArea = {MakeTuple '#' N}

      %% Initialize area and size
      {Record.foldRInd Spec.squares
       fun {$ D M I}
          {For I I+M-1 1 proc {$ J}
                            SqsSize.J=D SqsArea.J=D*D
                         end}
          I+M
       end 1 _}

      %% Mapping: I -> Atom(Int), needed for scheduling propagators
      IntToAtom = {MakeTuple '#' N}

      {For 1 N 1 proc {$ I}
                    IntToAtom.I = {VirtualString.toAtom I}
                 end}

   in

      proc {$ Root}
         SqsX0 % left coordinates
         SqsX1 % right coordinates
         SqsY0 % upper coordinates
         SqsY1 % lower coordinates
         Cuts  % cut information

         DM = {New DistributionManager init}

         local
            fun {FindCut1 N [I] X0 X1 Y0 Y1}
               SqsX0.I=X0 SqsY0.I=Y0 X1>=:SqsX1.I Y1>=:SqsY1.I
               {DM add(N proc {$}
                            {FD.distribute splitMin X0#Y0}
                         end)}
               nil
            end

            fun {FindCut2 N [I1 I2] X0 X1 Y0 Y1}
               S1X0=SqsX0.I1 S1X1=SqsX1.I1 S1Y0=SqsY0.I1 S1Y1=SqsY1.I1
               S2X0=SqsX0.I2 S2X1=SqsX1.I2 S2Y0=SqsY0.I2 S2Y1=SqsY1.I2
               C={FD.bool}
            in
               S1X0=X0 S1Y0=Y0
               {DM add(N proc {$}
                            {FD.distribute splitMin C#S1X0#S1Y0}
                         end)}
               or S2X0=X0   S2Y0=S1Y1 X1>=:S1X1 Y1>=:S2Y1 C=0
               [] S2X0=S1X1 S2Y0=Y0   X1>=:S2X1 Y1>=:S1Y1 C=1
               end
               case C
               of 0 then info(cut:S1Y1 dir:x nil nil)
               [] 1 then info(cut:S1X1 dir:y nil nil)
               end
            end

            fun {FindCut3 N [I1 I2 I3] X0 X1 Y0 Y1}
               S1X0=SqsX0.I1 S1X1=SqsX1.I1 S1Y0=SqsY0.I1 S1Y1=SqsY1.I1
               S2X0=SqsX0.I2 S2X1=SqsX1.I2 S2Y0=SqsY0.I2 S2Y1=SqsY1.I2
               S3X0=SqsX0.I3 S3X1=SqsX1.I3 S3Y0=SqsY0.I3 S3Y1=SqsY1.I3
               C={FD.int 0#3}
            in
               S1X0=X0 S1Y0=Y0
               {DM add(N proc {$}
                            {FD.distribute splitMin C#S1X0#S1Y0}
                         end)}
               or S2X0=X0 S2Y0=S1Y1 C::0#1
                  or %% in y direction: 1,2,3
                     S3X0=X0   S3Y0=S2Y1 X1>=:S1X1 Y1>=:S3Y1
                     C=0
                  [] %% in y direction: 1,(2,3)
                     S3X0=S1X1 S3Y0=S1Y1 Y1>=:S2Y1 X1>=:{FD.max S1X1 S3X1}
                     C=1
                  end
               [] S2X0=S1X1 S2Y0=Y0 C::2#3
                  or %% in x direction: 1,2,3
                     S3X0=S2X1 S3Y0=Y0   X1>=:S3X1 Y1>=:S1Y1
                     C=2
                  [] %% in x direction: 1,(2,3)
                     S3X0=S2X0 S3Y0=S2Y1 X1>=:S2X1 Y1>=:{FD.max S1Y1 S3Y1}
                     C=3
                  end
               end
               case C
               of 0 then info(cut:S1Y1 dir:x nil info(cut:S2Y1 dir:x nil nil))
               [] 1 then info(cut:S1Y1 dir:x nil info(cut:S2X1 dir:y nil nil))
               [] 2 then info(cut:S1X1 dir:y nil info(cut:S2Y1 dir:x nil nil))
               [] 3 then info(cut:S1X1 dir:y nil info(cut:S2X1 dir:y nil nil))
               end
            end

            local
               fun {Group Is GIs GIr S}
                  case Is
                  of nil then
                     GIr=nil [GIs]
                  [] I|Ir then SI=SqsSize.I in
                     if S==SI then NGIr in
                        GIr=I|NGIr
                        {Group Ir GIs NGIr S}
                     else NGIr NGIs in
                        NGIs=I|NGIr GIr=nil
                        GIs|{Group Ir NGIs NGIr SI}
                     end
                  end
               end
            in
               fun {GroupBySize I|Ir}
                  GIs GIr in GIs=I|GIr {Group Ir GIs GIr SqsSize.I}
               end
            end


            proc {ArrangeOneSize Is SqsXY0 SqsXY1 Cut ?Ls Lr ?Rs Rr}
               case Is of nil then Ls=Lr Rs=Rr
               [] I|Ir then
                  dis
                     SqsXY1.I =<: Cut
                  then
                     Ls=I|{ArrangeOneSize Ir SqsXY0 SqsXY1 Cut $ Lr Rs Rr}
                  []
                     SqsXY0.I >=: Cut
                     {ForAll Ir proc {$ I}
                                   SqsXY0.I>=:Cut
                                end}
                  then
                     Ls=Lr Rs={Append Is Rr}
                  end
               end
            end

            proc {Arrange Gs SqsXY0 SqsXY1 Cut ?Ls ?Rs}
               case Gs
               of nil then
                  Ls=nil Rs=nil
               [] Is|Gr then Lr Rr in
                  {ArrangeOneSize Is SqsXY0 SqsXY1 Cut ?Ls Lr ?Rs Rr}
                  {Arrange Gr SqsXY0 SqsXY1 Cut ?Lr ?Rr}
               end
            end

            proc {FindCutM N Is X0 X1 Y0 Y1 ?Info}
               %% More than three squares
               Cut      = {FD.decl}
               I1|I1r = Is
               I2|I2r = I1r
               MinSize  = {FoldL I2r fun {$ M I}
                                        {Min SqsSize.I M}
                                     end SqsSize.I2}
               Gs       = {GroupBySize I1r}
            in
               %% Arrange first square
               SqsX0.I1 = X0
               SqsY0.I1 = Y0
               {DM add(N
                       proc {$}
                          choice
                             Ls Rs InfoL InfoR
                          in
                             %% Cut
                             Cut >=: X0+SqsSize.I1
                             Cut + MinSize =<: X1
                             Info = info(dir:y cut:Cut InfoL InfoR)
                             {FD.distribute mid [Cut]}
                             {Arrange Gs SqsX0 SqsX1 Cut ?Ls ?Rs}
                             InfoL={FindCut N+1 I1|Ls X0 Cut Y0 Y1}
                             InfoR={FindCut N+1 Rs    Cut X1 Y0 Y1}
                          []
                             Ls Rs InfoL InfoR
                          in
                             %% Cut
                             Cut >=: Y0+SqsSize.I1
                             Cut + MinSize =<: Y1
                             Info = info(dir:x cut:Cut InfoL InfoR)
                             {FD.distribute mid [Cut]}
                             {Arrange Gs SqsY0 SqsY1 Cut ?Ls ?Rs}
                             InfoL = {FindCut N+1 I1|Ls X0 X1 Y0 Cut}
                             InfoR = {FindCut N+1 Rs    X0 X1 Cut Y1}
                          end
                       end)}
            end

         in
            fun {FindCut N Is X0 X1 Y0 Y1}
               thread
                  case {Length Is}
                  of 0 then fail nil
                  [] 1 then {FindCut1 N Is X0 X1 Y0 Y1}
                  [] 2 then {FindCut2 N Is X0 X1 Y0 Y1}
                  [] 3 then {FindCut3 N Is X0 X1 Y0 Y1}
                  else      {FindCutM N Is X0 X1 Y0 Y1}
                  end
               end
            end
         end

         proc {Fit SqsXY Cap}
            Tasks = IntToAtom
            As    = {Record.foldR Tasks fun {$ A Ar} A|Ar end nil}
            Dur   = {Record.make dur   As}
            Start = {Record.make start As}
            Use   = Dur
         in
            {For 1 N 1
             proc {$ I}
                A = Tasks.I
             in
                Dur.A=SqsSize.I Start.A=SqsXY.I
             end}
            {Schedule.cumulative [Tasks] Start Dur Use [Cap]}
         end

      in

         SqsX0 = {MakeTuple '#' N}
         SqsX1 = {MakeTuple '#' N}
         SqsY0 = {MakeTuple '#' N}
         SqsY1 = {MakeTuple '#' N}

         Root = root(x:SqsX0 y:SqsY0 d:SqsSize dx:DX dy:DY cuts:Cuts)


         %% Set up problem variables
         {For 1 N 1 proc {$ I}
                       Size = SqsSize.I
                       X0   = {FD.int 0#{Max 0 DX-Size}}
                       Y0   = {FD.int 0#{Max 0 DY-Size}}
                       X1   = {FD.decl}
                       Y1   = {FD.decl}
                    in
                       X1 =: X0 + Size  Y1 =: Y0 + Size
                       SqsX0.I = X0  SqsY0.I = Y0
                       SqsX1.I = X1  SqsY1.I = Y1
                    end}


         %% The squares must fit the target
         {FD.sum SqsArea '=<:' DX*DY}


         %% Fix some freedom for first square (wolg)
         SqsX0.1=0 SqsY0.1=0


         %% Remove permutations of equally-sized squares by ordering them
         {For 1 N-1 1 proc {$ I}
                         if SqsSize.I==SqsSize.(I+1) then
                            %% This is respected by the no overlap
                            SqsX0.I * DY + SqsY1.I =<:
                            SqsX0.(I+1) * DY + SqsY1.(I+1)
                         end
                      end}

         %% No Overlaps allowed
         {FD.distinct2 SqsX0 SqsSize SqsY0 SqsSize}

         %% In any direction (be it x or y) the squares must
         %% fit into the height/width of the rectangle
         {Fit SqsX0 DY}
         {Fit SqsY0 DX}

         Cuts = {FindCut 1 {List.number 1 N 1} 0 DX 0 DY}

      end

   end

end


   /*

declare
Compile = {Module.apply [Script]}.1.compile


declare
%Spec = spec(x:6 y:5 squares:s(3:2 2:3 1:0))
%Spec = spec(x:7 y:9 squares:s(2:4 5:1 1:1 3:1))
%Spec = spec(x:8 y:8 squares:s(4:4 1:1))
%Spec = spec(x:7 y:7 squares:s(5:1 3:1 2:3))
%Spec = spec(x:14 y:14 squares:s(5:2 4:4 3:3 2:5 1:0))
Spec = spec(x:14 y:14 squares:s(5:2 4:4 3:3 2:10 1:15))
%Spec = spec(x:17 y:20 squares:s(5:2 4:3 3:3 2:5 1:5))
%Spec = spec(x:10 y:10 squares:s(1:1 2:4 3:2 4:0 5:2))
%Spec = spec(x:10 y:10 squares:s(1:2 2:2 3:2 4:2 5:1))
%Spec = spec(x:10 y:12 squares:d(4:2 3:2 2:7 1:2))
%Spec = spec(x:10 y:12 squares:d(4:6))
{ExploreOne {Compile Spec}}

declare R={{Compile Spec}}

declare
fun {DrawSquares N Sol}
   DX=20 DY=20
   HX=10 HY=10
   T = {New Tk.toplevel tkInit(title:'Node '#N)}
   C = {New Tk.canvas tkInit(parent: T
                             bg:     ivory
                             width:  Sol.x * DX
                             height: Sol.y * DY)}
in
   {For 1 {Width Sol.sqsx} 1
    proc {$ I}
       SD = Sol.sqsd.I
       SX = Sol.sqsx.I
       SY = Sol.sqsy.I
    in
       if {IsDet SX} andthen {IsDet SY} then
          X=SX*DX
          Y=SY*DY
       in
          {C tk(create rectangle X+2 Y+2 X+SD*DX-2 Y+SD*DY-2
                fill:    c(60 179 113)
                width:   1
                outline: black)}
          {C tk(create text X+SD*HX Y+SD*HY
                text:I)}
       end
    end}
   {Tk.send pack(C)}
   {Browse Sol}
   T#tkClose
end

{Explorer.object add(information fun {$ N Sol}
                                    {DrawSquares N Sol}
                                 end)}

*/
