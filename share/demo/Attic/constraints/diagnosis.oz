%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5315
%%%  Author: Joerg Wuertz
%%%  Email: wuertz@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

% diagnosis of a n-bit adder

\ifndef ALONEDEMO
   declare Diagnose in
\endif


local

   BackgroundBW = black
   ForegroundBW = white
   FillOffBW    = white
   FillOnBW     = black
   FillC        = 'IndianRed'
   Font         = 'lucidasanstypewriter-bold-18'
   [CanvasColor DefectC UnmarkC ButtonC ButtonActiveC]
                = case Tk.isColor
                  then [lightskyblue3 'IndianRed' seagreen3 lightsteelblue lightsteelblue1]
                  else [white black white white white]
                  end
   [DefectMess UnmarkMessB UnmarkMessT MarkMess]
                = case Tk.isColor
                  then [tk(itemconfigure background: DefectC)
                        tk(itemconfigure background: UnmarkC)
                        tk(itemconfigure fill: UnmarkC)
                        tk(itemconfigure fill: DefectC)]
                  else [tk(itemconfigure background: BackgroundBW
                           foreground: ForegroundBW)
                        tk(itemconfigure foreground: BackgroundBW
                           background: ForegroundBW)
                        tk(itemconfigure fill:white)
                        tk(itemconfigure fill: BackgroundBW)]
                  end

   fun {MkBitMap Name}
      '@'#{System.get home}#'/demo/bitmaps/diagnosis/'#Name
   end

   % N: Number of bits
   % X,Y,Z: Input values in decimal
   % C1: Bit for input carry
   % C: Bit for output carry
   % Ds: Diagnosis vector
   % F: Number of faults in decimal
   proc {Diagnosis N X Y Z C1 C Ds F}
      Tmps
   in
      F :: 0#5*N
      Tmps = {Nadder N X Y Z C1 C Ds}       %N-bit adder
      {FD.sum Ds '=:' F}  % Sum of failures must be F
      {FD.distribute generic(value:min order:naive) [F]}
      {FD.distribute ff Ds}
      {FD.distribute naive Tmps}
   end

   fun {Nadder N X Y Z C1 C Ds}
      Xs Ys Zs
   in
      [X Y Z] ::: 0#{Pow 2 N}-1
      [C1 C] ::: 0#1
      % transform X,Y,Z in bit vectors or vice versa.
      % Observe: list starts with least significant bit, eg. 6 = 011
      {Bits N X Xs}
      {Bits N Y Ys}
      {Bits N Z Zs}
      Ds = {FD.list 5*N 0#1}
      {Adder Xs Ys Zs C1 C Ds nil}   % Adding with bit vectors
   end

   proc {Bits N Dec Xs}
      Xs = {FD.list N 0#1}
      {List.foldLInd Xs proc{$ Ind In X Out}
                           % Out = X*2^I + In
                           Out = {FD.decl}
                           Out =: X*{Pow 2 Ind-1} + In
                        end 0 Dec}
   end


   fun {Adder Xs Ys Zs Cin Cout Ds Dss}
      % retrieve bits from vectors and call the fulladder
      case Xs
      of X|Xr
      then
         Y|Yr = Ys
         Z|Zr = Zs
         D0|D1|D2|D3|D4|DS1 = Ds
         CtmpOut
         FA
      in
         CtmpOut :: 0#1
         FA = {FullAdder X Y Cin Z CtmpOut D0 D1 D2 D3 D4}
         {Append CtmpOut|FA {Adder Xr Yr Zr CtmpOut Cout DS1 Dss}}
      [] nil then Cin = Cout
         Ds = Dss
         nil
      end
   end

   local
      And=FD.conj  Or=FD.disj Not=FD.nega
      Xor=FD.exor Equiv=FD.equi Implies=FD.impl
   in
      fun {FullAdder X Y Cin Z Cout D0 D1 D2 D3 D4}
      % logical description of a fulladder and the diagnosistic variables
      % D=1 means that the gate is defect
         [U1 U2 U3] = {FD.list 3 0#1}
      in
         {Equiv {Not D0} {Equiv U1 {And X Y}} 1}
         {Equiv {Not D1} {Equiv U2 {And U3 Cin}} 1}
         {Equiv {Not D2} {Equiv Cout {Or U1 U2}} 1}
         {Equiv {Not D3} {Equiv U3 {Xor X Y}} 1}
         {Equiv {Not D4} {Equiv Z {Xor U3 Cin}} 1}
         [U1 U2 U3]
      end
   end

   fun {WrapSearch S}
      case S == nil orelse S == stopped then S
      else S.1
      end
   end

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %%% graphic stuff
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   InfoText = 'This demo is about hardware diagnosis for an n-bit adder. The default is a two-bit adder.\n
Each n-bit adder consists of n full adders. A full adder has two input bits x and y and a carry input ci. Its output is the bit z and the output carry co which is fed into the next carry input. Each full adder consists of a number of logical gates.\n
The user can specify a certain input/output pattern by clicking on the corresponding circles. By pressing the button\n
\t\'Diagnose\'\n
a diagnosis is computed and displayed. A logical gate is marked if the specified pattern can be produced when this gate is defective. Defective means that the gate has the opposite behavior than its definition, i.e., a defective and-gate behaves like a nand-gate. The number of defective gates is indicated in the field with label \'Failures\'.\n
By pressing the buttons\n
\t\'Next\' and \'Previous\'\n
one can obtain the next resp. previous diagnosis. The number of defective gates may change.\n
Error and status messages are displayed in the centered display.\n
If the demo is not compiled stand-alone, it is possible to diagnose others than only two-bit adders. To this aim, send the message \'numberOfBits(N)\' to the object \'Diagnose\'. For example,\n
\t{Diagnose numberOfBits(3)}\n
will provide you with a 3-bit adder.\n
The input/output pattern can also be set by the user. For example,\n
\t{Diagnose setBits(input:[ci x2 y0 y2] output:[z1 z2])}\n
will set the input bits ci, x2, y0, and y2 as well as the output bits z1 and z2.\n\n
\t\t\t          Jörg Würtz\n
\t\t\t(wuertz@dfki.uni-sb.de)'

   proc {ShowDefects Defects Bitmaps}
      {ForAll {List.zip Defects Bitmaps fun{$ D B} D#B end}
       proc {$ D#B}
          case D of 1 then {B DefectMess}
          else skip
          end
       end}
   end

   proc {ShowMarked Pins Texts}
      {ForAll Pins proc{$ P} {Texts.P.1 MarkMess} end}
   end

   proc {UnmarkBitMaps BitMaps}
      {ForAll BitMaps proc{$ B} {B UnmarkMessB} end}
   end

   proc {UnmarkTexts Texts}
      {Record.forAll Texts proc{$ T#_}{T UnmarkMessT} end}
   end


   fun {IntToAtom I} {String.toAtom {Int.toString I}} end

   proc{CreateDiagnose Diagnose}
      proc{DrawAdder N Limit Off XO YO V Texts BitMaps}
         % Y   3   4   Z
         % CI    1
         % X   0   2   C
         case N<Limit then
            BPos0 BPos1 BPos2 BPos3 BPos4 BM0 BM1 BM2 BM3 BM4
            Th2F Th2O O2Tw Z2Tw
            Y2Th Y2O C12O C12F X2O X2Th F2Z Tw2C
            YN CI XN ZN CO TmpTexts TTT
            YOff=YO+N*Off
         in
            BPos0 = XO#(YOff+100)
            BPos1 = (XO+120)#(YOff+50)
            BPos2 = (XO+240)#(YOff+90)
            BPos3 = XO#YOff
            BPos4 = (XO+240)#(YOff+10)

            {ForAll [BM0 BM1 BM2 BM3 BM4]
             fun {$}
                {New Tk.canvasTag tkInit(parent:V)}
             end}
            {V tk(crea bitmap BPos0.1 BPos0.2
                  bitmap:{MkBitMap 'and.xbm'}
                  background: UnmarkC
                  tag:BM0)}
            {V tk(crea bitmap BPos1.1 BPos1.2
                  bitmap:{MkBitMap 'and.xbm'}
                  background: UnmarkC
                  tag:BM1)}
            {V tk(crea bitmap BPos2.1 BPos2.2
                  bitmap:{MkBitMap 'or.xbm'}
                  background: UnmarkC
                  tag:BM2)}
            {V tk(crea bitmap BPos3.1 BPos3.2
                  bitmap:{MkBitMap 'xor.xbm'}
                  background: UnmarkC
                  tag:BM3)}
            {V tk(crea bitmap BPos4.1 BPos4.2
                  bitmap:{MkBitMap 'xor.xbm'}
                  background: UnmarkC
                  tag:BM4)}

            Th2F = crea(line XO+20 YOff XO+220 YOff)
            Th2O = crea(line XO+20 YOff XO+80 YOff XO+80 YOff+40
                         XO+100 YOff+40)
            O2Tw = crea(line XO+140 YOff+50 XO+160 YOff+50
                        XO+160 YOff+80 XO+220 YOff+80)
            Z2Tw = crea(line XO+20 YOff+100 XO+220 YOff+100)

            {ForAll [Th2F Th2O O2Tw Z2Tw] proc{$ X} {V tk(X)} end}

            Y2Th = crea(line XO-60 YOff-10 XO-20 YOff-10)
            Y2O  = crea(line XO-60 YOff-10 XO-50 YOff-10 XO-50 YOff+90
                        XO-20 YOff+90)
            C12O = crea(line XO-60 YOff+60 XO+100 YOff+60)
            C12F = crea(line XO-60 YOff+60 XO+70 YOff+60 XO+70 YOff+20
                        XO+220 YOff+20)
            X2O  = crea(line XO-60 YOff+110 XO-20 YOff+110)
            X2Th = crea(line XO-60 YOff+110 XO-30 YOff+110 XO-30 YOff+10
                        XO-20 YOff+10)
            F2Z  = crea(line XO+260 YOff+10 XO+300 YOff+10)
            case N==Limit-1 then
               Tw2C = crea(line XO+260 YOff+90 XO+300 YOff+90)
               {V tk(Tw2C)}
            else skip
            end
            {ForAll [Y2Th Y2O C12O C12F X2O X2Th F2Z] proc{$ X}
                                                         {V tk(X)}
                                                      end}

            {V tk(crea oval XO-52 YOff-8 XO-48 YOff-12    fill:black)}
            {V tk(crea oval XO-32 YOff+112 XO-28 YOff+108 fill:black)}
            {V tk(crea oval XO+68 YOff+62 XO+72 YOff+58   fill:black)}
            {V tk(crea oval XO+78 YOff+2 XO+82 YOff-2     fill:black)}

            {ForAll [XN YN ZN] fun {$}
                                  {New Tk.canvasTag tkInit(parent:V)}
                               end}

            local
               T1 = {String.toAtom &x|{Int.toString N}}
               T2 = {String.toAtom &y|{Int.toString N}}
               T3 = {String.toAtom &z|{Int.toString N}}
            in
               TTT = t(T1: XN#off
                       T2: YN#off
                       T3: ZN#off)
            end

            case N==0 then
               CI={New Tk.canvasTag tkInit(parent:V)}
               {V tk(crea text XO-102 YOff+60
                     text: ci font:Font)}
               {V tk(crea oval XO-80 YOff+50 XO-60 YOff+70
                     tag:CI fill:UnmarkC)}
               {CI tkBind(event:'<1>' action: Diagnose # pressed(ci))}
            elsecase N==Limit-1 then
               CO={New Tk.canvasTag tkInit(parent:V)}
               {V tk(crea text XO+335 YOff+90
                     text: co font:Font)}
               {V tk(crea oval XO+290 YOff+80 XO+310 YOff+100
                     tag:CO fill:UnmarkC)}
               {CO tkBind(event:'<1>'
                          action: Diagnose # pressed(co))}
            else skip
            end

            {V tk(crea text XO-102 YOff-10  text:'y'#N font:Font)}
            {V tk(crea text XO-102 YOff+110 text:'x'#N font:Font)}
            {V tk(crea text XO+335 YOff+10  text:'z'#N font:Font)}
            {V tk(crea oval XO-80 YOff-20 XO-60 YOff
                  tag:YN fill:UnmarkC)}
            {V tk(crea oval XO-80 YOff+100 XO-60 YOff+120
                  tag:XN fill:UnmarkC)}
            {V tk(crea oval XO+290 YOff XO+310 YOff+20
                  tag:ZN fill:UnmarkC)}
            {YN tkBind(event:'<1>'
                       action: Diagnose # pressed({String.toAtom
                                                   &y|{Int.toString N}}))}
            {XN tkBind(event:'<1>'
                       action: Diagnose # pressed({String.toAtom
                                                   &x|{Int.toString N}}))}
            {ZN tkBind(event:'<1>'
                       action: Diagnose # pressed({String.toAtom
                                                   &z|{Int.toString N}}))}
            case N<Limit-1
            then {V tk(crea line XO+260 YOff+90 XO+280 YOff+90
                       XO+280 YOff+130 XO-90 YOff+130
                       XO-90 YOff+220 XO-60 YOff+220)}
            else skip
            end

            BitMaps = BM0|BM1|BM2|BM3|BM4
                      |{DrawAdder N+1 Limit Off XO YO V TmpTexts}
            case N==0 then
               Texts={Adjoin {Adjoin t(ci:CI#off) TTT} TmpTexts}
            elsecase N==Limit-1 then
               Texts={Adjoin {Adjoin t(co:CO#off) TTT} TmpTexts}
            else Texts = {Adjoin TTT TmpTexts}
            end
         else BitMaps=nil Texts=t
         end
      end
      fun {BuildBits N Pins Pin}
         {Loop.forThread N-1 0 ~1
          fun{$ I X}
             case {Member
                   {String.toAtom
                    {Append {Atom.toString Pin} {Int.toString X}}} Pins}
             then 1|I
             else 0|I
             end
          end nil}
      end
      fun {MakeButton Parent Text Self Message}
         {New Tk.button tkInit(parent:Parent
                               text: Text
                               highlightthickness:0
                               background:ButtonC
                               activebackground:ButtonActiveC
                               pady: '1m'
                               action: Self # Message)}
      end

      class DiagnoseClass from BaseObject
         attr texts:_
            bitMaps:_
            n x:nil y ci co z c w:nil l pc:1  solList:nil
            solL solved:no failed:no numberSet:false failures
            solver
         meth numberOfBits(NBit)
            x<-nil
            case {IsInt NBit}
            then
               solved<-no
               solList<-nil
               failed<-no
               pc<-1
               numberSet<-true
               case @w==nil then {self DrawInit(NBit)}
               else {self DeleteWindow}
                  {self DrawInit(NBit)}
               end
               {@l tk(conf text:'')}
            else case @w==nil then {Browse 'Wrong argument type'}
                 else {@l tk(conf text: 'Wrong argument type')}
                 end
            end
         end
         meth DrawInit(NBit)
            W V B Texts BitMaps Label Frame BNext BPrevious
            Failures Label2 Frame2 Info
         in
            W = {New Tk.toplevel tkInit(background: CanvasColor)}
            {Tk.send wm(resizable W 0 0)}
            {Tk.send wm(title W 'Oz Hardware Diagnosis')}
            V = {New Tk.canvas tkInit(parent:W
                                      width:500
                                      background: CanvasColor
                                      highlightthickness: 0
                                      height:180*NBit)}
            Frame  = {New Tk.frame tkInit(parent:W
                                      background: CanvasColor)}
            Frame2 = {New Tk.frame tkInit(parent:Frame)}
            Label  = {New Tk.label tkInit(parent:Frame
                                          text:'' width:20
                                          background:CanvasColor
                                          relief: sunken)}
            Label2 = {New Tk.label tkInit(parent:Frame2
                                          background:CanvasColor
                                          highlightthickness:0
                                          text:'Failures: ')}
            Failures = {New Tk.label tkInit(relief: sunken
                                         parent: Frame2
                                         width: 2
                                         highlightthickness:0
                                         background:CanvasColor)}
            {Tk.send pack(Label2 Failures side: left)}

            B         = {MakeButton Frame 'Diagnose' self Solvee}
            BNext     = {MakeButton Frame 'Next' self Next}
            BPrevious = {MakeButton Frame 'Previous' self Previous}
            Info      = {MakeButton Frame 'Info' self InfoPressed}

            {Tk.batch [pack(B Frame2 Label BNext BPrevious Info
                            padx:'5m' side:left)
                       pack(Frame  V pady:'2m' side:top)]}
            {DrawAdder 0 NBit 160 130 40 V Texts BitMaps}
            bitMaps <- BitMaps
            texts <- Texts
            n<-NBit
            c<-V
            w<-W
            l<-Label
            failures<-Failures
         end
         meth DeleteWindow
            {@w tkClose}
            texts<-nil
         end
         meth pressed(Pin)
            case @texts.Pin
            of V#off
            then
               texts<-{AdjoinAt @texts Pin V#on}
               {V tk(itemconfigure fill: case Tk.isColor then DefectC
                                         else FillOnBW end)}
            [] V#on
            then
               texts<-{AdjoinAt @texts Pin V#off}
               {V tk(itemconfigure fill: case Tk.isColor then UnmarkC
                                         else FillOffBW end)}
            end
         end
         meth InfoPressed
            W = {New Tk.toplevel tkInit(parent: @w)}
            {Tk.send wm(resizable W 0 0)}
            Mess = {New Tk.message tkInit(parent:W
                                          width:500
                                          background:CanvasColor
                                          text: InfoText)}
         in
            {Tk.batch [wm(title W 'Diagnosis Info')
                       pack(Mess)]}
         end
         meth setBits(input:Input<=nil output:Output<=nil)
            case @numberSet==false
            then {@l tk(conf text: 'Set number of Bits')} x<-nil
            else Texts = @texts
            in
               solved<-no
               solList<-nil
               solved<-no
               pc<-1
               {@l tk(conf text:'')}
               case
                  {All Input fun{$ X} {HasFeature Texts X} end} andthen
                  {All Output fun{$ X} {HasFeature Texts X} end}
               then
                  case @w==nil
                  then {@l tk(configure text: 'Define number of bits!')}
                  else
                     {UnmarkTexts @texts}
                     {UnmarkBitMaps @bitMaps}
                     {ForAll {Map {Arity @texts} fun{$ X} Unset(X) end}
                      proc{$ M} {self M} end}
                     {ForAll {Map Input fun{$ X} Set(X) end}
                      proc{$ M} {self M} end}
                     {ForAll {Map Output fun{$ X} Set(X) end}
                      proc{$ M} {self M} end}
                     {ShowMarked Input @texts}
                     {ShowMarked Output @texts}
                  end
               else {@l tk(conf text: 'Wrong arguments')} x<-nil
               end
            end
         end
         meth Set(X)
            V = @texts.X.1 in
            texts<-{AdjoinAt @texts X V#on}
         end
         meth Unset(X)
            V=@texts.X.1 in
            texts<-{AdjoinAt @texts X V#off}
         end
         meth Solvee
            S X Y Z CI CO N NextS Input Output
            Texts=@texts
            InputTail = {FoldL {Arity Texts}
                         fun{$ I X}
                            case Texts.X.2==on andthen
                               ({Atom.toString X}.1==&x orelse
                                {Atom.toString X}.1==&y)
                            then X|I
                            else I
                            end
                         end nil}
            OutputTail = {FoldL {Arity Texts}
                          fun{$ I X}
                             case Texts.X.2==on andthen
                                {Atom.toString X}.1==&z
                             then X|I
                             else I
                             end
                          end nil}
         in
            Input = case Texts.ci.2==on
                    then ci|InputTail
                    else InputTail
                    end
            Output = case Texts.co.2==on
                     then co|OutputTail
                     else OutputTail
                     end
            x <- {Bits @n $ {BuildBits @n Input x}}
            y <- {Bits @n $ {BuildBits @n Input y}}
            case {Member ci Input} then ci<-1 else ci<-0 end
            z <- {Bits @n $ {BuildBits @n Output z}}
            case {Member co Output} then co<-1 else co<-0 end
            failed<-no
            {@l tk(configure text:diagnosing)}
            X=@x Y=@y Z=@z CI=@ci CO=@co N=@n
            {UnmarkBitMaps @bitMaps}
            solver <- {New Search.object
                       script(proc{$ Sol}
                                 Ds#F = !Sol in
                                 {Diagnosis N X Y Z CI CO Ds F}
                              end)}
            S = {WrapSearch {@solver next($)}}
            case S of nil then
               {@l tk(configure text:'No solution found')}
               {@failures tk(configure text:'')}
            else
               {@l tk(configure text:diagnosed)}
               {@failures tk(configure text: S.2)}
               {ShowDefects S.1 @bitMaps}
               solved<-yes
               solList<-[(S.1)#(S.2)]
               pc<-1
            end
         end
         meth Next
            case @solved of no
            then {@l tk(configure text:'Diagnose first!')}
            else
               case @pc == {Length @solList}
               then
                  case @failed of yes
                  then {@l tk(configure text:'No further solution')}
                  else
                     S = {WrapSearch {@solver next($)}}
                  in
                     {@l tk(configure text:diagnosing)}
                     case S of nil
                     then
                        solved<-no
                        failed<-yes
                        {@l tk(configure text:'No further solution')}
                        {@failures tk(configure text:'')}
                     else
                        {@l tk(configure text:diagnosed)}
                        {UnmarkBitMaps @bitMaps}
                        {ShowDefects S.1 @bitMaps}
                        {@failures tk(configure text:S.2)}
                        pc <- @pc+1
                        solList<-(S.1)#(S.2)|@solList
                     end
                  end
               else SolN in
                  pc<-@pc+1
                  SolN = {Nth @solList {Length @solList}-@pc+1}
                  {@l tk(configure text:diagnosed)}
                  {UnmarkBitMaps @bitMaps}
                  {ShowDefects SolN.1 @bitMaps}
                  {@failures tk(configure text:SolN.2)}
               end
            end
         end
         meth Previous
            case @pc of 1
            then {@l tk(configure text:'No previous solution!')}
            else SolN in
               solved<-yes
               pc <- @pc-1
               SolN = {Nth @solList {Length @solList}-@pc+1}
               {UnmarkBitMaps @bitMaps}
               {@l tk(configure text:diagnosed)}
               {ShowDefects SolN.1 @bitMaps}
               {@failures tk(configure text:SolN.2)}
            end
         end
      end
   in
      Diagnose = {New DiagnoseClass noop}
   end % of proc

   proc {DiagnosisDemo}
      Diagnose in
      {CreateDiagnose Diagnose}
      {Diagnose numberOfBits(2)}
      {Diagnose setBits(input:[ci y1] output:[z1])}
   end

   in
\ifndef ALONEDEMO
   {CreateDiagnose Diagnose}
   {Diagnose numberOfBits(2)}
   {Diagnose setBits(input:[ci y1] output:[z1])}
\else
   {DM newDemo(call:DiagnosisDemo group:fd name:'Diagnosis')}
\endif

end % of local




/*

% a two bit adder
{Diagnose numberOfBits(2)}

{Diagnose setBits(input:[ci] output:[z1])}

{Diagnose setBits(input:[ci y1] output:[z1])}

{Diagnose setBits(input:[ci y1] output:[z1])}

% a four bit adder

{Diagnose numberOfBits(4)}
{Diagnose setBits(input:[ci x2 y0 y2] output:[z1 z2])}

% a five bit adder

{Diagnose numberOfBits(5)}
{Diagnose setBits(output:[z0 z1 z2 z3 z4])}
*/
