declare



fun {Compile Spec PlaceAll}

   %% Specification is as follows:
   %%  Spec.x, Spec.y: size of the target plate
   %%  Spec.squares.D=N: N squares of size D

   N     = {Record.foldL Spec.squares Number.'+' 0}
   Sizes = {Reverse {Arity Spec.squares}} % ordered in increasing size
   DX    = Spec.x
   DY    = Spec.y


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

   proc {Distribute Sqs}
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
      {FD.distribute splitMin
       {Record.foldR Sqs fun {$ Sq XYs}
                            if Sq.placed==1 then
                               Sq.x|Sq.y|XYs
                            else
                               XYs
                            end
                         end nil}}
   end

in

   proc {$ Sqs}
      Sqs = {MakeTuple '#' N}

      %% Set up problem variables
      {Record.foldRInd Spec.squares
       fun {$ D I M}
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

      if N>1 then
         %% Place the largest on at the lower left (wlog)
         Sqs.1.x=0  Sqs.1.y=0
         %% Also fix some freedom for second square
         Sqs.2.x =<: Sqs.2.y
      end

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

      %% The master rectangle must be splittable
%      {Splitting Squares}


      {Distribute Sqs}
   end

end



declare
Spec = spec(x:5 y:5 squares:s(3:1 2:2))
%Spec = spec(x:7 y:8 ds:[2 2 2 5 2 1 3])
%Spec = spec(x:12 y:13 ds:[5 5 4 4 4 3 3 3 2 2 2 2 2 1 1 1 1])
%Spec = spec(x:8 y:9 ds:[4 4 3 3 2 2 2 2 2 1 1 1])
{ExploreOne {Compile
             Spec
             true}}




/*

declare
fun {DrawSquares Spec Sol}
   DX=20 DY=20
   T = {New Tk.toplevel tkInit}
   C = {New Tk.canvas tkInit(parent: T
                             bg:     ivory
                             width:  Spec.x * DX
                             height: Spec.y * DY)}
in
   {Tk.send pack(C)}
   {For 1 {Length Spec.ds} 1
    proc {$ I}
       Sq = Sol.I
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
       end
    end}
   T#tkClose
end

{Explorer.object add(information fun {$ N Sol}
                                    {DrawSquares Spec Sol}
                                 end)}

*/

/*

proc {Splitting Sqs}
   thread
      if {FD.sum {Record.map Sqs fun {$ Sq} S.placed end} '>:' 1} then
         {Split Sqs Sqs Sqs}
      end
   end
end


   % The whole rectangle must be cuttable with straight cuts.
   % If a cut is possible (checked by CheckSplit), cut the rectangle
   % into two pieces and call Split recursively with both halves.
   % First it is tested whether cuttable by x-coordinate.
   % The variant with deep guards is slightly more efficient than a variant
   % with CheckSplit returning a value and flat guards.
   proc {Split XSqs YSqs Sqs}
      cond XSqs=[_] YSqs=[_] then skip else
         case XSqs
         of Square|SquareRest then
               if if Square.chosen=1
                     {CheckSplit Square.x AllSquares nil nil x}
                  then
                     LeftSquares RightSquares
                  in
                     {DoSplit Square.x AllSquares LeftSquares RightSquares x}
                     {Split LeftSquares LeftSquares LeftSquares}
                     {Split RightSquares RightSquares RightSquares}
                  else fail
                  end
               then skip
               else  {Split SquareRest YSquares AllSquares}
               end
         [] nil then
            case YSquares
            of Square|SquareRest
            then
               if if Square.chosen=1
                     {CheckSplit Square.y AllSquares nil nil y}
                  then
                     LeftSquares RightSquares
                  in
                     {DoSplit Square.y AllSquares LeftSquares RightSquares y}
                     {Split LeftSquares LeftSquares LeftSquares}
                     {Split RightSquares RightSquares RightSquares}
                  else fail
                  end
               then skip
               else  {Split XSquares SquareRest AllSquares}
               end
            end
         [] nil then fail
            end
         end
      end
   end

   % Check whether straight cut is possible at position Pos (lower
   % left corner of a square). To avoid nontermination,
   % at least one square must be left and right of the cut.
   proc {CheckSplit Pos Squares Left Right Axis}
      case Squares of nil then Left=left Right=right
      [] S|Sr
      then X=S.Axis C=S.chosen in
         thread
            if C=1 X>=:Pos
            then {CheckSplit Pos Sr Left right Axis}
            [] C=1 X+S.size=<:Pos
            then {CheckSplit Pos Sr left Right Axis}
            [] C=0 then {CheckSplit Pos Sr Left Right Axis}
            [] C=1 X+S.size>:Pos X<:Pos then fail
            else  fail
            end
         end
      end
   end

   % Split the rectangle into two: one left of the cut, one right of it.
   % The cut is at position Cut.
   proc {DoSplit Cut Squares LeftSquares RightSquares Axis}
      case Squares
      of nil
      then LeftSquares=RightSquares=nil
      [] S|Sr
      then
         X=S.Axis C=S.chosen in
         if S.chosen=0
         then {DoSplit Cut Sr LeftSquares RightSquares Axis}
         [] C=1 X+S.size=<:Cut
         then LSquares in
            LeftSquares=S|LSquares
            {DoSplit Cut Sr LSquares RightSquares Axis}
         [] C=1 X>=:Cut
         then RSquares in
            RightSquares=S|RSquares
            {DoSplit Cut Sr LeftSquares RSquares Axis}
         [] C=1 X+S.size>:Cut X<:Cut then fail
         else  fail
         end
      end
   end






                       */
