declare


proc {BinaryPartition ?S1 ?S2 S}
   %% Sets S1 and S2 are binary partition of S
   C1 C2
in
   S1={FS.var.decl} S2={FS.var.decl}
   {FS.disjoint S1 S2} {FS.union S1 S2 S}
   C1={FS.card S1} C2={FS.card S2}
   C1>:0 C2>:0 C1+C2=:{FS.card S}
end

local
   fun {Run I1 I2 Is}
      I1|if I1==I2 then Is else {Run I1+1 I2 Is} end
   end
in
   fun {UnfoldSetSpec Ss}
      case Ss of nil then nil
      [] S|Sr then
         case S of I1#I2 then {Run I1 I2 {UnfoldSetSpec Sr}}
         else S|{UnfoldSetSpec Sr}
         end
      end
   end
end

fun {Compile Spec PlaceAll}

   %% Specification is as follows:
   %%  Spec.x, Spec.y: size of the target plate
   %%  Spec.squares.D=N: N squares of size D

   N  = {Record.foldL Spec.squares Number.'+' 0}
   DX = Spec.x
   DY = Spec.y


   %% Area covered by squares.
   proc {Covered Sqs}
      {FD.sum {Record.map Sqs proc {$ Sq ?Res}
                                 Area = Sq.d * Sq.d
                              in
                                 Res :: [0 Area]
                                 Sq.placed =: (Area =: Res)
                              end} '=<:' DX*DY}
   end

   proc {NoOverlap Sqs}
      %% No rectangles must overlap, return variables for arrangement
      thread
         {For 1 N 1
          proc {$ I}
             Sq1 = Sqs.I
          in
             if Sq1.placed==1 then
                X1=Sq1.x Y1=Sq1.y D1=Sq1.d
             in
                {For 1 I-1 1
                 proc {$ J}
                    Sq2=Sqs.J
                 in
                    if Sq2.placed==1 then
                       X2=Sq2.x Y2=Sq2.y D2=Sq2.d
                    in
                       (X1 + D1 =<: X2) +  (X2 + D2 =<: X1) +
                       (Y1 + D1 =<: Y2) +  (Y2 + D2 =<: Y1) >: 0
                    end
                 end}
             end
          end}
      end
   end

   fun {Merge Xs Ys}
      case Xs of nil then Ys
      [] X|Xr then X|{Mix Ys Xr}
      end
   end
   fun {Mix Xs Ys}
      {WaitOr Xs Ys}
      if {IsDet Xs} then {Merge Xs Ys} else {Merge Ys Xs} end
   end

   proc {Distribute Sqs Cuts}
      %% Place all rectangles
      if PlaceAll then
         {Record.forAll Sqs proc {$ Sq}
                               Sq.placed = 1
                            end}
      else
         {FD.distribute generic(order:naive value:max)
          {Record.map Sqs fun {$ Sq} Sq.placed end}}
      end
      %% Size down the unplaced sqaures
      {Record.forAll Sqs proc {$ Sq}
                            if Sq.placed==0 then
                               Sq.x=0 Sq.y=0
                            end
                         end}
      %% Find the position
      {ForAll
       {Mix thread {Map Cuts fun {$ C} C.cut end} end
        {Record.foldR Sqs fun {$ Sq XYs}
                             if Sq.placed==1 then
                                Sq.x|Sq.y|XYs
                             else
                                XYs
                             end
                          end nil}}
       proc {$ X}
          {FD.distribute splitMin [X]}
       end}
   end

   local
      fun {Swap XY}
         case XY of x then y [] y then x end
      end
   in
      fun {MakeCuts XY SQS Sqs}
         thread
            cond {FS.card SQS}<:4 then
               %% For this, a cut always exists and is trivial to find
               nil
            else
               SQS1 SQS2 Cut
            in
               {BinaryPartition SQS1 SQS2 SQS}
               Cut :: 1#N-1
               {ForAll {UnfoldSetSpec {FS.reflect.upperBound SQS}}
                proc {$ I}
                   SqsI=Sqs.I SqsIXY=SqsI.XY
                   IsLeftOfCut  = Cut >=: SqsIXY + SqsI.d
                   IsRightOfCut = Cut =<: SqsIXY
                in
                   {FD.nega IsLeftOfCut IsRightOfCut}
                   IsLeftOfCut  = {FS.reified.isIn I SQS1}
                   IsRightOfCut = {FS.reified.isIn I SQS2}
                end}
               cut(xy:XY cut:Cut SQS1 SQS2)|
               {Mix
                {MakeCuts {Swap XY} SQS1 Sqs}
                {MakeCuts {Swap XY} SQS2 Sqs}}
            end
         end
      end
   end

in

   proc {$ Root}
      root(squares:Sqs cuts:Cuts) = Root
   in

      Sqs = {MakeTuple '#' N}

      %% Set up problem variables
      {Record.foldRInd Spec.squares
       fun {$ D M I}
          {For I I+M-1 1 proc {$ J}
                            Sqs.J=square(x:      {FD.int 0#{Max 0 DX-D}}
                                         y:      {FD.int 0#{Max 0 DY-D}}
                                         d:      D
                                         placed: {FD.bool})
                         end}
          I+M
       end 1 _}

      %% The placed squares must fit the target
      {Covered Sqs}

      %% Fix some freedom for first square (wolg)
      Sqs.1.x =<: Sqs.1.y

      %% Remove permutations of equally-sized squares by ordering them
      {For 1 N-1 1 proc {$ I}
                      Sq1=Sqs.I Sq2=Sqs.(I+1)
                   in
                      if Sq1.d==Sq2.d then
                         Sq1.x =<: Sq2.x
                         (Sq1.x \=: Sq2.x) + (Sq1.y <: Sq2.y) >=: 1
                      end
                   end}

      %% Squares must not overlap
      {NoOverlap Sqs}

      %% At any position (be it x or y) the placed squares must
      %% fit into the height/width of the rectangle
      {ForAll [x#DY y#DX]
       proc {$ XY#DXY}
          {For 0 DXY-1 1
           proc {$ I}
              {FD.sum {Record.map Sqs
                       proc {$ Sq ?D}
                          D :: [0 Sq.d]
                          {FD.conj Sq.placed
                           %% this the same as:
                           %%  {FD.conj I=<:Sq.XY Sq.XY+Sq.d>=:I}
                           (Sq.XY :: {Max 0 I-Sq.d+1}#I)
                           (D=:Sq.d)}
                       end} '=<:' DXY}
           end}
       end}


      Cuts = {MakeCuts x {FS.value.make [1#N]} Sqs}

      {Distribute Sqs Cuts}

   end

end



declare
%Spec = spec(x:5 y:5 squares:s(3:1 2:2))
%Spec = spec(x:7 y:8 squares:s(2:4 5:1 1:1 3:1))
Spec = spec(x:16 y:15 squares:s(5:2 4:3 3:3 2:5 1:4))
%Spec = spec(x:8 y:9 ds:[4 4 3 3 2 2 2 2 2 1 1 1])
{ExploreOne {Compile
             Spec
             true}}




declare
fun {DrawSquares Spec Sol}
   DX=20 DY=20
   HX=10 HY=10
   T = {New Tk.toplevel tkInit}
   C = {New Tk.canvas tkInit(parent: T
                             bg:     ivory
                             width:  Spec.x * DX
                             height: Spec.y * DY)}
in
   {Browse Sol}
   {Tk.send pack(C)}
   {For 1 {Width Sol.squares} 1
    proc {$ I}
       Sq = Sol.squares.I
       SX = Sq.x
       SY = Sq.y
    in
       if {IsDet Sq.placed} andthen Sq.placed==1 andthen
          {IsDet SX} andthen {IsDet SY}
       then
          X=SX*DX
          Y=SY*DY
       in
          {C tk(create rectangle X+2 Y+2 X+Sq.d*DX-2 Y+Sq.d*DY-2
                fill:    blue
                width:   1
                outline: black)}
          {C tk(create text X+Sq.d*HX Y+Sq.d*HY
                text:I)}
       end
    end}
   T#tkClose
end

{Explorer.object add(information fun {$ N Sol}
                                    {DrawSquares Spec Sol}
                                 end)}
