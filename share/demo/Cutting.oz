%  Programming Systems Lab, DFKI Saarbruecken,
%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5315
%  Author: Joerg Wuertz
%  Email: wuertz@dfki.uni-sb.de
%  Last modified: $Date$ by $Author$
%  Version: $Revision$

% A Cutting Problem:
% A list of squares of given sizes must be placed into a rectangle.
% Two constraints have to be obeyed (observe that not all squares might
% fit into the rectangle) if mode 'opt' is chosen:
% - The waste has to be minimalized.
% - It must be possible to cut the squares out of the rectangle by
%   a series of straight cuts.
% A simplification of the problem occurs, if all squares must fit
% into the rectangle, ie., mode 'all' is chosen.

\ifndef ALONEDEMO
declare Cutter in
\endif

local
   Cutting
\ifndef ALONEDEMO
   Zoom=15
\else
   Zoom=13
\endif
   Off=2

   proc {Square Squares Sizes Mode SX SY}
      Squares = {Sort {StateDomains Sizes SX SY}
                 fun{$ A B}
                    A.size<B.size
                 end}
      {Covered Squares 0} =<: SX*SY
      %% find at least one
      {FoldL Squares fun{$ I Sq} {FD.plus I Sq.chosen} end  0} >: 0
      %% Squares must not overlap pairwise
      {NoOverlap Squares}
      %% Redundant constraints
      {Capacity Squares SX SY x}
      {Capacity Squares SY SX y}
      %% The master rectangle must be splittable
      {Splitting Squares}
      %% Remove symmetries
      {NoPermutation Squares SY}
      %% Enumerate squares
      {Enumerate Squares Mode}
   end


   fun {StateDomains Sizes SX SY}
      {Map Sizes fun{$ S}
                    square(x: {FD.int 0#{Max SX-S 0}}
                           y: {FD.int 0#{Max SY-S 0}}
                           chosen: {FD.int 0#1}
                           size:   S)
                 end}
   end

   %% Remove symmetries by ordering equal-sized squares
   proc {NoPermutation Squares SY}
      fun{MakeR Squares InR}
         case Squares of nil then InR
         [] S|Sr
         then
            SA={String.toAtom {Int.toString S.size}}
         in
            thread
               case S.chosen of 1 then
                  case {HasFeature InR SA}
                  then {MakeR Sr {Record.adjoinAt InR SA (S.x#S.y)|(InR.SA)}}
                  else {MakeR Sr {Record.adjoinAt InR SA [S.x#S.y]}}
                  end
               else {MakeR Sr InR}
               end
            end
         end
      end
      PosTuple={MakeR Squares rec}
   in
      thread
         {Record.forAll PosTuple
          proc{$ Ls}
             {ForAllTail Ls proc{$ L|Lr}
                               case Lr of nil then skip
                               [] H|T then L.1*SY+L.2 <: H.1*SY+H.2
                               end
                            end}
          end}
      end
   end

   % Area covered by the chosen squares.
   fun {Covered Squares Sum}
      case Squares of nil then Sum
      [] S|Sr
      then {Covered Sr {FD.plus {FD.times S.chosen S.size*S.size} Sum}}
      end
   end

   local
      fun {Sum Pos Squares Axis}
      % the sum of all the heights of rectangles over this position must
      % be less or equal than SY
         {Map Squares fun{$ S}
                         LeftPos = Pos-S.size+1
                      in
                         case LeftPos < 0 then
                            S.Axis :: 0#Pos
                         else
                            S.Axis :: LeftPos#Pos
                         end
                      end}
      end
   in
   % The sum of the squares's sizes at an arbitrary  position
   % must be less or equal than the corresponding rectangle size.
      proc {Capacity Squares SX SY Axis}
         {Loop.for 0 SX-1 1 proc{$ Pos}
                               {FD.sumCN
                                {Map Squares fun{$ S} S.size end}
                                {List.zip
                                 {Sum Pos Squares Axis}
                                 {Map Squares fun{$ S} S.chosen end}
                                 fun{$ A B} [A B] end}
                                '=<:' SY}
                            end}
      end
   end

   % No rectangle must overlap another one. Nonoverlapping
   % may caused by four relations.
   proc {NoOverlap Squares}
      % No rectangles must overlap
      {List.forAllTail Squares proc{$ S1|Sr}
                                  {ForAll Sr proc{$ S2}
                                                {NoOverlap1 S1 S2}
                                             end}
                               end}
   end

   proc {NoOverlap1 Sq1 Sq2}
      thread
         case (Sq1.chosen+Sq2.chosen=:2)==1
         then
            (Sq1.x+Sq1.size =<: Sq2.x) +
            (Sq2.x+Sq2.size =<: Sq1.x) +
            (Sq1.y+Sq1.size =<: Sq2.y) +
            (Sq2.y+Sq2.size =<: Sq1.y) >=: 1
         else skip
         end
      end
   end

   proc {Splitting Squares}
      thread
         case ({FoldL {Map Squares fun{$ S} S.chosen end} FD.plus 0}=:1)==1
         then skip
         else
            {Split Squares Squares Squares}
         end
      end
   end


   % The whole rectangle must be cuttable with straight cuts.
   % If a cut is possible (checked by CheckSplit), cut the rectangle
   % into two pieces and call Split recursively with both halves.
   % First it is tested whether cuttable by x-coordinate.
   % The variant with deep guards is slightly more efficient than a variant
   % with CheckSplit returning a value and flat guards.
   proc {Split XSquares YSquares AllSquares}
      if XSquares=[_] YSquares=[_]
      then skip %One square is cuttable per definition
      else
         case XSquares
         of Square|SquareRest
         then
            thread
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
            end
         [] nil
         then
            case YSquares
            of Square|SquareRest
            then
               thread
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


   proc {Enumerate Squares Mode}
      case Mode of opt then
         {FD.distribute generic(order:naive value:max)
          {Map Squares fun{$ S} S.chosen end}}
      else {ForAll Squares proc{$ S} S.chosen=1 end}
      end
      {SpecialEnum {Reverse Squares}}
   end

   % Enumerate the coordinates.Chosens which square to select are already made.
   proc {SpecialEnum Squares}

      case Squares
      of nil then skip
      [] S|Sr
      then
         case S.chosen of 1 then
            {FD.distribute naive [S.y S.x]}
            {SpecialEnum Sr}
         else S.x=0 S.y=0
            {SpecialEnum Sr}
         end
      end
   end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Graphics stuff

   fun {DrawSquares W S}
      Frame = {New Tk.frame tkInit(parent:     W
                                   background: case Tk.isColor then blue
                                               else black
                                               end
                                   width:      S*Zoom
                                   height:     S*Zoom)}
   in
      {Tk.send pack(Frame pady:'1m' padx:'1m' side:top)}
      Frame
   end

   proc {MarkSquares Squares Recs}
      case Squares#Recs of nil#nil then skip
      [] (S|Sr)#(R|Rr)
      then case S.chosen of 1 then {Tk.send pack(forget R)}
           else skip
           end
         {MarkSquares Sr Rr}
      end
   end


   proc {ShowSolution SX SY Squares V}
      case Squares
      of S|Sr
      then case S.chosen of 1
           then
              Sx = S.x
              Sy = S.y
              Ss = S.size
           in
              {V tk(crea rectangle Sx*Zoom+1 Sy*Zoom+1
                      (Sx+Ss)*Zoom+1 (Sy+Ss)*Zoom+1)}
              {V tk(crea rectangle Sx*Zoom+Off+1
                    Sy*Zoom+Off+1
                    (Sx+Ss)*Zoom-Off+1
                    (Sy+Ss)*Zoom-Off+1
                    fill: case Tk.isColor then red
                          else black
                          end)}
           else skip
           end
         {ShowSolution SX SY Sr V}
      [] nil then skip
      end
   end

   proc {DeleteSolvedOnes Squares Rs NSs NRs}
      case Squares#Rs of nil#nil then NSs=nil NRs=nil
      [] (S|Sr)#(R|Rr)
      then case S.chosen of 1 then {DeleteSolvedOnes Sr Rr NSs NRs}
           else  NS NR in
              NSs=S.size|NS
              NRs=R|NR
              {DeleteSolvedOnes Sr Rr NS NR}
           end
      end
   end

   fun {MakeButton P T C M}
      {New Tk.button tkInit(parent: P
                            text:   T
                            pady:   '1m'
                            action: proc{$} {C M} end)}
   end

   fun {MakeEntry F E}
      {New Tk.entry tkInit(relief:       sunken
                           parent:       F
                           width:        2
                           textvariable: E )}
   end

   fun {MakeFrame P}
      {New Tk.frame tkInit(parent: P)}
   end

   fun {MakeLabel P T}
      {New Tk.label tkInit(parent: P
                           text:   T)}
   end

   InfoText = 'This demo is about cutting a large plate made of glass into a number of smaller squares. One has either to cut all the smaller squares from the large plate or has to find such a set of small squares such that the waste is minimized.\n
There are two constraints for this problem:\n
\t The small squares must not overlap\n
\t The resulting pattern must be cuttable by so-called guillotine-cuts. This means that the plate must be cuttable by straight cuts (no cutting over edge). This constraint holds also for the resulting two parts.\n
After starting the demo, two windows pop up: One window for specifying the problem and one containing the target plate where the small squares should be placed. Under the menu\n
\t \'Problems\'\n
some predefined problems can be chosen. The chosen problem can be solved by pressing the button\n
\t \'Solve\'\n
A toggle button allows to swtich between the case that all squares must fit into the target (indicated by \'All\') and the case where the amount of waste should be minimized (indicated by \'Opt\').\n
The target square can be resized either by dragging it by the mouse or by specifying the appropriated sizes in the entries labeled by
\t\'x-Size\'\n and\n\t \'y-Size\'.\n
The entry labeled by\n
\t\'Square Size\'\n
allows to specify the size of the next square to add. The square is added by either pressing return in this entry or by pressing the button\n
\t\'Add Square\'.\n
The status of the search is indicated in the centered display.\n\n
\t\t\t          Jörg Würtz\n
\t\t\t(wuertz@dfki.uni-sb.de)'


   proc {CreateCutter Cutter}
      class CutterClass from BaseObject
         attr ss:nil sx:5 sy:5 window recs:nil button mode:opt label
            solutionCanvas squareSize xSize ySize solW
         meth init
            W = {New Tk.toplevel tkInit(delete: self # Close
                                        title:  'Cutting Problems')}
            {Tk.send wm(resizable W 0 0)}

            Frame = {MakeFrame W}

            B     = {MakeButton Frame "Solve" self Solver}
            BT    = {MakeButton Frame "Opt" self Toggle}
            AB    = {MakeButton Frame "Add Square" self AddSquare}
            Info  = {MakeButton Frame "Info" self InfoPressed}

            Label = {New Tk.label tkInit(parent: Frame
                                         text:   ""
                                         width:  Zoom
                                         relief: sunken)}

            Frame3 = {MakeFrame W}
            Frame2 = {MakeFrame Frame3}
            Label2 = {MakeLabel Frame2 'Square Size: '}

            Frame4 = {MakeFrame Frame3}
            VEntry = {New Tk.variable tkInit}
            Entry  = {MakeEntry Frame2 VEntry}

            Label4 = {MakeLabel Frame4 'x-Size: '}

            VEntry4 = {New Tk.variable tkInit}
            Entry4  = {MakeEntry Frame4 VEntry4}

            Frame5  = {MakeFrame Frame3}
            Label5  = {MakeLabel Frame5 'y-Size: '}
            VEntry5 = {New Tk.variable tkInit}
            Entry5  = {MakeEntry Frame5 VEntry5}

            MenusEs = {Map [['Problem1' 5#5 [3 2 2]]
                            ['Problem2' 5#5 [5 3]]
                            ['Problem3' 7#8 [2 2 2 5 2 1 3]]
                            ['Problem4' 8#9 [4 4 3 3 2 2 2 2 2 1 1 1]]
                            ['Problem5' 13#15 [5 5 4 4 4 3 3 3 2 2 2 2 2 1 1 1 1]]]
                       fun {$ [Text SX#SY Ss]}
                          command(label: Text
                                  action: proc{$}
                                             {self Resize(sx:SX sy:SY)}
                                             {self AddSquares(Ss)}
                                          end)
                       end}

            Menus   = {TkTools.menubar Frame3 Frame3
                       [
                         menubutton(text:   'Problems'
                                    relief: raised
                                    menu:   MenusEs)
                       ]
                       nil }

         in
            {Tk.batch [pack(AB B Label Info BT padx:'5m' side:left)
                       pack(Frame)
                       pack(Label2 Entry side:left)
                       pack(Label4 Entry4 side:left)
                       pack(Label5 Entry5 side: left)
                       pack(Frame4 Frame5 Frame2 Menus side:left
                            padx:'5m' pady:'2m')
                       pack(Frame3 side:bottom)
                      ]}

            {Entry tkBind(event: '<Return>'
                          action: self#AddSquare)}

            {Entry4 tkBind(event: '<Return>'
                           action: self#ResizeX)}
            {Entry5 tkBind(event: '<Return>'
                           action: self#ResizeY)}

            window     <- W
            button     <- BT
            label      <- Label
            squareSize <- VEntry
            xSize      <- VEntry4
            ySize      <- VEntry5
            {@xSize tkSet(@sx)}
            {@ySize tkSet(@sy)}

            local %target plate
               W1 = {New Tk.toplevel tkInit(parent: @window
                                            title: 'Target Plate')}
               {Tk.send wm(resizable W1 1 1)}
               Canvas = {New Tk.canvas tkInit(parent:W1
                                              width:@sx*Zoom
                                              height:@sy*Zoom)}
            in
               {Tk.batch [wm(maxsize W1 10000 10000)
                          pack(Canvas expand:yes fill:both)
                          wm(grid W1 @sx @sy Zoom Zoom)]}

               {W1 tkBind(event:  '<Configure>'
                          action: proc{$} {self SetSizes} end)}
               solW<-W1
               solutionCanvas<-Canvas
            end
         end

         meth InfoPressed
            W = {New Tk.toplevel tkInit(parent: @window
                                        title:   'Cutting Info'
                                        )}
            {Tk.send wm(resizable W 0 0)}
            Mess = {New Tk.message tkInit(parent: W
                                          width:  500
                                          text: InfoText
                                         )}
         in
            {Tk.send pack(Mess)}
         end

         meth Close
            {@window tkClose}
         end

         meth Toggle
            case @mode of opt
            then mode<-all {@button tk(configure(text:"All"))}
            else mode<-opt {@button tk(configure(text:"Opt"))}
            end
         end

         meth AddSquare
            Entry={@squareSize tkReturn($)} in
            case Entry=="" orelse  {Not {All Entry Char.isDigit}}
            then {@label tk(conf(text: "Square size incorrect!"))}
            else {self AddSquares([{String.toInt {@squareSize tkReturn($)}}])}
            end
         end

         meth ResizeX
            Entry={@xSize tkReturn($)} in
            case Entry=="" orelse {Not {All Entry Char.isDigit} }
            then {@label tk(conf(text: "x-Size incorrect!"))}
            else
               {self Resize(sx:{String.toInt {@xSize tkReturn($)}} sy:@sy)}
            end
         end

         meth ResizeY
            Entry={@ySize tkReturn($)} in
            case  Entry=="" orelse {Not {All Entry Char.isDigit}}
            then {@label tk(conf(text: "y-Size incorrect!"))}
            else
               {self Resize(sy:{String.toInt {@ySize tkReturn($)}} sx:@sx)}
            end
         end

         meth AddSquares(Sizes)
            case Sizes of nil then skip
            [] S|Sr then
               case {IsInt S}
               then Rec in
                  {@squareSize tkSet({Int.toString S})}
                  ss<-S|@ss
                  {DrawSquares @window S Rec}
                  recs<-Rec|@recs
                  {self AddSquares(Sr)}
               else
                  {@label tk(conf(text: "Sizes must be integers"))}
               end
            end
         end

         meth Resize(sx:SX sy:SY)
            case {IsInt SX} andthen {IsInt SY}
            then
               sx<-SX sy<-SY
               {@xSize tkSet({Int.toString SX})}
               {@ySize tkSet({Int.toString SY})}
               {Tk.send wm(geometry @solW SX#"x"#SY)}
            else  {@label tk(conf(text: "Sizes must be integers"))}
            end
         end

         meth SetSizes
            {Tk.send update(idletasks)}
            sx<-{Tk.returnInt winfo(width @solW) $} div Zoom
            sy<-{Tk.returnInt winfo(height @solW) $} div Zoom
            {@xSize tkSet(@sx)}
            {@ySize tkSet(@sy)}
         end

         meth Solver
            case @ss of nil
            then {@label tk(conf(text: "No solution found"))}
            else
               S
               SX = @sx
               SY = @sy
               F  = SX*SY
               %% Order the sizes. The largest square is enumerated first.
               Zipped = {Sort {List.zip @ss @recs fun{$ A B} A#B end}
                         fun {$ S1#R1 S2#R2}
                            S1<S2
                         end}
               Ss   = {Map Zipped fun {$ Z} Z.1 end}
               Recs = {Map Zipped fun {$ Z} Z.2 end}
            in
               {@label tk(conf(text: "Solving ..."))}
               case @mode of opt
               then
                  S = {SearchBest
                       proc{$ Squares}
                          {Square Squares Ss opt SX SY}
                       end
                       proc{$ OldSquares NewSquares}
                          F = SX*SY
                       in
                          F-{Covered NewSquares 0} <: F-{Covered OldSquares 0}
                       end
                      }
               else
                  S = {SearchOne
                       proc{$ Squares}
                          {Square Squares Ss all SX SY}
                       end}
               end
               case S of nil
               then {@label tk(conf(text: "No solution found"))}
               else
                  Squares = S.1
                  NewSS NewRecs W1 Canvas
               in
                  {ForAll Recs Wait}

                  {ShowSolution SX SY Squares @solutionCanvas}
                  {MarkSquares Squares Recs}
                  {DeleteSolvedOnes Squares Recs NewSS NewRecs}
                  ss   <- NewSS
                  recs <- NewRecs
                  {@label tk(conf(text: "Solved, left: "#{Length NewRecs}))}
                  %% create a new solution canvas
                  W1 = {New Tk.toplevel tkInit(parent: @window)}
                  {Tk.send wm(resizable W1 1 1)}
                  {Tk.send wm(title(W1 "Target Plate"))}
                  {Tk.send wm(grid(W1 @sx @sy Zoom Zoom))}
                  {Tk.send wm(maxsize(W1 10000 10000))}
                  {W1 tkBind(event:'<Configure>'
                             action:proc{$}
                                       {self SetSizes}
                                    end)}
                  Canvas = {New Tk.canvas tkInit(parent:W1
                                                 width:@sx*Zoom
                                                 height:@sy*Zoom)}
                  {Tk.send pack(Canvas o(expand:yes
                                         fill:both))}
                  solutionCanvas <- Canvas
                  solW <- W1
               end
            end
         end
      end
   in
      Cutter = {New CutterClass init}
   end
in
   proc {Cutting}
      thread {CreateCutter _} end
   end
\ifndef ALONEDEMO
   {CreateCutter Cutter}
\else
   {DM newDemo(call:Cutting group:fd name:'Cutting Stock')}
\endif
end
