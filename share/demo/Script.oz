declare


proc {Map3 Xs F ?Y1s ?Y2s ?Y3s}
   case Xs of nil then Y1s=nil Y2s=nil Y3s=nil
   [] X|Xr then Y1|Y1r=Y1s Y2|Y2r=Y2s Y3|Y3r=Y3s in
      {F X ?Y1 ?Y2 ?Y3} {Map3 Xr F Y1r Y2r Y3r}
   end
end

fun {SwapDir Dir}
   case Dir of x then y [] y then x end
end

local
   proc {Do Bs N}
      case Bs of nil then skip
      [] B|Br then NN=1-N in
         choice skip end
         if {Not {IsDet B}} then
            choice B=N [] B=NN end
         end
         {Do Br NN}
      end
   end
in
   proc {Alternate Bs}
      {Do Bs 0}
   end
end

fun {Compile Spec}

   %% Specification is as follows:
   %%  Spec.x, Spec.y: size of the target plate
   %%  Spec.squares.D=N: N squares of size D

   %% Number of all squares
   N  = {Record.foldL Spec.squares Number.'+' 0}
   %% The number of all squares of size greater than 1
   M  = N - {CondSelect Spec.squares 1 0}
   %% Dimension of X and Y
   DX = Spec.x
   DY = Spec.y


   fun {MakeCuts Rect Sqs}
      Dir = Rect.dir % Direction of the cut (x or y)
      SQS = Rect.sqs % The set of squares to be cut
      Cut = Rect.cut % The coordinate of cut
   in
      if {FS.card SQS}<:4==1 then
         %% Just go an place the squares
         %% Placing one suffices:
         %%  1) it can always be placed (due to capacity)
         %%  2) the upper left edge is okay, since all solutions
         %%     are symmetric with respect to rotation and
         %%     translation
         Sq=Sqs.({FS.reflect.upperBoundList SQS}.1)
      in
%        choice skip end
%        Sq.x = {FD.reflect.min Sq.x}
%        choice skip end
%        Sq.y = {FD.reflect.min Sq.y}
%        {ForAll
%         proc {$ I}
%            Sq=Sqs.I
%         in
%            {FD.distribute splitMin Sq.x#Sq.y}
%         end}
         %% Nothing to cut
         %% For at most three squares there always exists a cut
         Cut = 0
         nil
      else
         %% Partition the squares into two disjoint sets
         SQS1    = {FS.var.decl}
         SQS2    = {FS.var.decl}
         %% The other direction (direction of the next cut)
         Rid     = {SwapDir Dir}
         DimXY = Rect.Dir
         DimYX = Rect.Rid
         IsLs IsRs DDs
         DimD  = {FD.decl}
         DimL  = {FD.decl}
         DimR  = {FD.decl}
      in
         DimD    =: DimYX.2 - DimYX.1
         %% SQS1 and SQS2 are are partition of the squares
         {FS.union SQS1 SQS2 SQS}
         %% The cut must be inside the current dimensions, and
         %% of course the squares we consider here have at least size 2!
         DimXY.1+2=<:Cut Cut+2=<:DimXY.2
         %% Find Left and Right, just cardinality matters
         {Map3 {FS.reflect.upperBoundList SQS}
          proc {$ I ?IsL ?IsR ?DD}
             Sq=Sqs.I SqXY=Sq.Dir SqD=Sq.d
          in
             IsL = (Cut >=: SqXY + SqD)
             IsR = (Cut =<: SqXY)
             {FD.nega IsL IsR}
             IsL = {FS.reified.isIn I SQS1}
             IsR = {FS.reified.isIn I SQS2}
             DD  = SqD*SqD
          end ?IsLs ?IsRs ?DDs}
         %% However, the capacities must fit (redundant)
         DimL =: DimD * Cut - DimD * DimXY.1
         {FD.sumC DDs IsLs '=<:' DimL}
         DimR =: DimD * DimXY.2 - DimD * Cut
         {FD.sumC DDs IsRs '=<:' DimR}
         %% Distribute squares to be either right or left
         {Alternate IsLs}
         %% These are the next rectangles to be cut
         rect(dir:Rid cut:{FD.decl} Dir:DimXY.1#Cut Rid:DimYX sqs:SQS1)#
         rect(dir:Rid cut:{FD.decl} Dir:Cut#DimXY.2 Rid:DimYX sqs:SQS2)
      end
   end

   fun {DriveCuts Infos Sqs}
      if Infos==nil then nil else
         {Map Infos fun {$ I} I.cut end}|{DriveCuts {FoldL Infos
                                                     fun {$ Is I}
                                                        NI={MakeCuts I Sqs}
                                                     in
                                                        case NI
                                                        of nil then Is
                                                        [] I1#I2 then I1|I2|Is
                                                        end
                                                     end nil} Sqs}
      end
   end

   proc {Distribute Sqs ?Cuts}
      %%  1. Find a position for the cut, starting from the middle
      %%  2. Distribute all squares either to the left or right side
      %%     of the cut
      %%  3. Only consider squares with size graeter than 1
      Cuts={DriveCuts
            [info(dir:x cut:{FD.decl} x:0#DX y:0#DY
                  sqs:{FS.value.make [1#M]})] Sqs}
      %% The rest is deterministic
      {For M+1 N 1 proc {$ I}
                      choice skip end
                      Sqs.I.x = {FD.reflect.min Sqs.I.x}
                      choice skip end
                      Sqs.I.y = {FD.reflect.min Sqs.I.y}
%                     {FD.distribute naive [Sqs.I.x Sqs.I.y]}
                   end}
      %% The rest is deterministic
      {ForAll {FoldL Cuts Append nil}
       proc {$ Cut}
          choice skip end
          Cut = {FD.reflect.min Cut}
       end}
%      {FD.distribute naive
%       {FoldL Cuts Append nil}}
   end


in

   proc {$ Root}
      Sqs
      Cuts
   in



      Root = root(squares:Sqs cuts:Cuts x:DX y:DY)

      Sqs  = {MakeTuple '#' N}


      %% Set up problem variables
      {Record.foldRInd Spec.squares
       fun {$ D M I}
          {For I I+M-1 1 proc {$ J}
                            Sqs.J=square(x: {FD.int 0#{Max 0 DX-D}}
                                         y: {FD.int 0#{Max 0 DY-D}}
                                         d: D)
                         end}
          I+M
       end 1 _}


      %% The squares must fit the target
      {Record.foldL Sqs fun {$ N Sq}
                           N+Sq.d*Sq.d
                        end 0} =<: DX*DY


      %% Fix some freedom for first square (wolg)
      Sqs.1.x=0 Sqs.1.y=0


      %% Remove permutations of equally-sized squares by ordering them
      {For 1 N-1 1 proc {$ I}
                      Sq1=Sqs.I Sq2=Sqs.(I+1)
                   in
                      if Sq1.d==Sq2.d then
                         %% This is respected by the no overlap
                         Sq1.x =<: Sq2.x
                      end
                   end}


      %% No rectangles must overlap, return variables for arrangement
      {For 1 N 1
       proc {$ I}
          Sq1=Sqs.I X1=Sq1.x Y1=Sq1.y D1=Sq1.d
       in
          {For 1 I-1 1
           proc {$ J}
              Sq2=Sqs.J X2=Sq2.x Y2=Sq2.y D2=Sq2.d
           in
              if D1==D2 then
                 %% Simplified due to symmetry relations:
                 %% Sq2 is either below or to the right of Sq1
                 (X2 + D2 =<: X1) + (Y2 + D2 =<: Y1) >: 0
              else
                 (X1 + D1 =<: X2) +  (X2 + D2 =<: X1) +
                 (Y1 + D1 =<: Y2) +  (Y2 + D2 =<: Y1) >: 0
              end
           end}
       end}

      %% In any direction (be it x or y) the squares must
      %% fit into the height/width of the rectangle
      {ForAll [x#DY y#DX]
       proc {$ Dir#DXY}
          {For 0 DXY-1 1
           proc {$ I}
              {FD.sum {Record.map Sqs
                       proc {$ Sq ?D}
                          D :: [0 Sq.d]
                          %% this the same as:
                          %%  {FD.conj I=<:Sq.XY Sq.XY+Sq.d>=:I}
                          (Sq.Dir :: {Max 0 I-Sq.d+1}#I)=(D=:Sq.d)
                       end} '=<:' DXY}
           end}
       end}

      Cuts={Distribute Sqs}

   end

end



declare
%Spec = spec(x:5 y:5 squares:s(3:1 2:2))
%Spec = spec(x:7 y:9 squares:s(2:4 5:1 1:1 3:1))
%Spec = spec(x:8 y:8 squares:s(4:4))
%Spec = spec(x:14 y:14 squares:s(5:2 4:4 3:3 2:5 1:35))
%Spec = spec(x:14 y:14 squares:s(5:2 4:4 3:3 2:10 1:15))
Spec = spec(x:9 y:9 squares:d(4:2 3:2 2:5 1:3))
%2*16+3*4+
{ExploreOne {Compile Spec}}

/*

declare
S={Compile
             Spec
   true}
declare R
{Browse R}

{S R}
*/



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
   {For 1 {Width Sol.squares} 1
    proc {$ I}
       Sq = Sol.squares.I
       SX = Sq.x
       SY = Sq.y
    in
       if {IsDet SX} andthen {IsDet SY} then
          X=SX*DX
          Y=SY*DY
       in
          {C tk(create rectangle X+2 Y+2 X+Sq.d*DX-2 Y+Sq.d*DY-2
                fill:    c(60 179 113)
                width:   1
                outline: black)}
          {C tk(create text X+Sq.d*HX Y+Sq.d*HY
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
