%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Joerg Wuertz
%%%  Email: wuertz@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare DrawGantt in
local
local
   %% sizes
   TaskHeight      = 8
   TaskOffset      = 5
   TaskDistY       = 1
   LineHeight      = TaskHeight+2*TaskDistY
   ResourceOffsetX = 5
   ResourceOffsetY = 60
   TextOffsetY     = ~(TaskHeight div 2)-TaskDistY
   TextOffsetX     = 2
   OffsetX         = 1
   OffsetY         = 2
   CanvasWidth     = 1000
   BottomScroll    = 4000
   RightScroll     = 5000
   ScaleOffset     = 20
   ScaleLength     = RightScroll
   ScaleHeight     = 10

   %% colors
   FromLineColor   = lightseagreen
   ToLineColor     =  lightseagreen
   AllowedColor    = red
   Colors          = 'deep sky blue'|'dark orange'|green3|cyan4|tomato2|firebrick4|darkorchid|wheat3|cornsilk3|palegoldenrod|pink2|burlywood3|Colors


in
   class GanttClass from BaseObject
      attr
         positions
         initialized: false
         canvas
         resources
         colors
         tags
         taskStretch
      feat
         toplevel
      meth draw(Sol Node)
         case @initialized then skip
         else {self init(Sol Node)}
         end
         {Tk.send wm(title self.toplevel "Gantt Chart ("#Node#')')}
         {self go(Sol)}
      end
      meth init(Spec Node)
         TaskSpec#Start = Spec
         Resources = {FoldL TaskSpec
                      fun {$ In _#_#_#Resource}
                         case {Member Resource In}
                         then In else Resource|In end
                      end
                      nil}

         NbTasks = {Length TaskSpec}

         Tasks = {FoldL
                  {Sort Resources fun{$ A B} A>B end}
                  fun{$ I Resource}
                     {FoldR TaskSpec fun{$ Name#_#_#R In}
                                        case R==Resource then
                                           Name|In
                                        else In
                                        end
                                     end I}
                  end nil}

         Positions = {List.foldLInd Tasks
                      fun{$ Ind I Task}
                         {AdjoinAt I Task (Ind-1)*LineHeight+ResourceOffsetY+TaskDistY*2}
                      end positions}

         SumDur = {FoldL TaskSpec fun {$ In _#D#_#_} D+In end 0}

         Unit = ((10+(SumDur div 30)) div 10)*10
         TaskStretch = 7 % (RightScroll div 2) div SumDur

         UnitLength = TaskStretch*Unit

         W = {New Tk.toplevel tkInit(title:'Gantt Chart ('#Node#')')}
         = self.toplevel

         Canvas = {New Tk.canvas tkInit(parent: W
                                        bg:     ivory
                                        width: CanvasWidth
                                        height: {Min (1+NbTasks)*LineHeight+ResourceOffsetY 1000}
                                        scrollregion:q(0 0
                                                       RightScroll
                                                       BottomScroll))}

         VS = {New Tk.scrollbar tkInit(parent:W
                                       relief:sunken)}
         HS = {New Tk.scrollbar tkInit(parent:W
                                       relief:sunken orient:horiz)}

         {Tk.addYScrollbar Canvas VS}
         {Tk.addXScrollbar Canvas HS}

         {Tk.batch [grid(Canvas row:0 column:0 sticky:nsew)
                    grid(HS     row:1 column:0 sticky:we)
                    grid(VS     row:0 column:1 sticky:ns)
                    grid(columnconfigure W 0 weight:1)
                    grid(rowconfigure    W 0 weight:1)]}

      in
         canvas <- Canvas
         positions <- Positions
         colors <- {List.foldLInd Resources
                    fun{$ Ind I Resource}
                       {AdjoinAt I Resource {Nth Colors Ind}}
                    end colors}
         initialized<-true
         tags <- nil

         %% draw scale
         {Canvas tk(crea(line
                         TaskOffset
                         ScaleOffset
                         TaskOffset+ScaleLength
                         ScaleOffset))}
         {Loop.for 0 (ScaleLength div UnitLength) 1
          proc{$ C}
             SH2 = ScaleHeight div 2
             CU = C*UnitLength+TaskOffset in
             {Canvas tk(crea(line CU ScaleOffset+SH2 CU ScaleOffset-SH2))}
             {Canvas tk(crea(text CU ScaleOffset+SH2 o(text: {Int.toString C*Unit}
                                                       anchor: nw)))}
             {Canvas tk(crea(line CU ResourceOffsetY CU BottomScroll))}
          end}

         taskStretch <- TaskStretch
      end

      meth go(Spec)
         TaskSpec#Start = Spec
         Positions = @positions
         Canvas = @canvas
         Colors = @colors
         TaskStretch = @taskStretch
         ScheduleEnd = {FoldL TaskSpec
                        fun{$ I Name#Dur#_#_}
                           {Max I {FD.reflect.max Start.Name}+Dur}
                        end 0}
         EndTTag = {New Tk.canvasTag tkInit(parent:Canvas)}
         EndLTag = {New Tk.canvasTag tkInit(parent:Canvas)}
         ScheduleEndX = ScheduleEnd*TaskStretch+TaskOffset
      in
         {ForAll @tags proc{$ Tag} {Tag tk(delete)} end}
         {Canvas tk(crea(line
                         ScheduleEndX
                         ScaleOffset+ScaleHeight*2
                         ScheduleEndX
                         BottomScroll
                         o(fill: red
                           tag: EndLTag)))}
         {Canvas tk(crea(text
                         ScheduleEndX
                         ScaleOffset+ScaleHeight*2+OffsetX+TextOffsetX
                         o(text: {Int.toString ScheduleEnd}
                           tag: EndTTag
                           anchor: nw)))}

         tags <- EndLTag|EndTTag|
           {FoldL TaskSpec
            fun{$ In Name#Dur#_#Resource}
               LinePosition = Positions.Name+(TaskHeight div 2)+TaskDistY
               LFTag={New Tk.canvasTag tkInit(parent:Canvas)}
               LTTag={New Tk.canvasTag tkInit(parent:Canvas)}
               RTag={New Tk.canvasTag tkInit(parent:Canvas)}
               LTag={New Tk.canvasTag tkInit(parent:Canvas)}
            in
               {Canvas tk(crea(line
                               TaskOffset
                               LinePosition
                               TaskOffset+{FD.reflect.min Start.Name}*TaskStretch
                               LinePosition
                               o(fill: FromLineColor
                                 tag: LFTag)))}
               {Canvas tk(crea(line
                               TaskOffset+({FD.reflect.max Start.Name})*TaskStretch
                               LinePosition
                               RightScroll
                               LinePosition
                               o(fill: ToLineColor
                                 tag: LTTag)))}
               case {FD.reflect.size Start.Name}==1 then
                  RX1 = Start.Name*TaskStretch+TaskOffset
                  RY1 = Positions.Name+TaskDistY
                  RX2 = (Start.Name+Dur)*TaskStretch+TaskOffset
                  RY2 = Positions.Name+TaskHeight
               in
                  {Canvas tk(crea(rectangle RX1 RY1 RX2 RY2
                                  o(fill:Colors.Resource
                                    tag: RTag)))}
                  {Canvas tk(crea(text
                                  RX2+OffsetX+TextOffsetX
                                  RY1+OffsetY+TextOffsetY
                                  o(text: Name
                                    tag: LTag
                                    anchor: nw)))}
               else
                  {Canvas tk(crea(line
                                  TaskOffset+{FD.reflect.min Start.Name}*TaskStretch
                                  LinePosition
                                  TaskOffset+({FD.reflect.max Start.Name})*TaskStretch
                                  LinePosition
                                  o(fill: AllowedColor
                                    tag: RTag)))}
                  {Canvas tk(crea(text
                                  {FD.reflect.min Start.Name}*TaskStretch+TaskOffset
                                  Positions.Name+TaskDistY+OffsetY+TextOffsetY
                                  o(text: Name
                                    tag: LTag
                                    anchor: nw)))}
               end
               LFTag|LTTag|RTag|LTag|In
            end nil}
      end
   end
end

in

proc {DrawGantt Node Sol}
   Gantt = {New GanttClass noop}
in
   {Gantt draw(Bridge.tasks#Sol Node)}
end

end
