%%%
%%% Authors:
%%%   Christian Schulte <schulte@dfki.de>
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

declare

   local
      HelvFamily        = '-*-helvetica-medium-r-normal--*-'
      HelvBoldFamily    = '-*-helvetica-bold-r-normal--*-'
      CourierFamily     = '-*-courier-medium-r-normal--*-'
      CourierBoldFamily = '-*-courier-bold-r-normal--*-'
      FontMatch         = '-*-*-*-*-*-*'

      FontSize          = 120
      SmallSize         = 100
   in
      [HelvBold Helv Courier SmallCourierBold] =
      {Map [HelvBoldFamily    # FontSize  # FontMatch
            HelvFamily        # FontSize  # FontMatch
            CourierFamily     # FontSize  # FontMatch
            CourierBoldFamily # SmallSize # FontMatch]
       VirtualString.toAtom}
   end

NoPlates   = 5
Quantity   = 60
PlateWidth = 7
PlateColor = 'SteelBlue1' % c(191 239 255) %c(142 188 143)
Pad = 2
EntryColor = PlateColor
TextFont

class PlateBar
   from Tk.canvas
   prop locking

   feat d
   attr Pos:0

   meth init(parent:P size:S)
      lock
         D  = S * PlateWidth
         N  = Quantity div (S + 1)
         XH = ~ Quantity * PlateWidth
      in
         Tk.canvas,tkInit(parent: P
                          width:  (Quantity - 1) * PlateWidth
                          height: D
                          bg:     ivory)
         {For 0 N-1 1
          proc {$ I}
             X0 = XH + I*(D+PlateWidth) + 1
             X1 = X0 + D - 1
             Y0 = 1
             Y1 = Y0 + D - 1
          in
             {self tk(create rectangle X0 Y0 X1 Y1
                      fill:    PlateColor
                      outline: black)}
          end}
         self.d = D
      end
   end

   meth set(N)
      lock
         M = N - @Pos
      in
         Pos <- N
         Tk.canvas,tk(move all M*(self.d + PlateWidth) 0)
      end
   end

end

class EditDialog
   from TkTools.note
%   from Tk.frame
   feat spec

   meth init(parent:P)
      TkTools.note,tkInit(parent:P text:'Edit')
      PlateFrame  = {New TkTools.textframe
                     tkInit(parent: self
                            font:   Helv
                            text:   'Glass Plates')}
      self.spec = {Dictionary.new}

      TicklePackPlates =
      {ForThread 1 NoPlates 1
       fun {$ Tcls D}
          L = {New Tk.label
               tkInit(parent: PlateFrame.inner
                      font:   Helv
                      text:   D#'x'#D)}
          E = {New TkTools.numberentry
               tkInit(parent: PlateFrame.inner
                      font:   HelvBold
                      min:    0
                      max:    Quantity div (D + 1)
                      width:  2
                      action: proc {$ I}
                                 {G set(I)}
                                 {Dictionary.put self.spec D I}
                              end)}
          {E.entry tk(configure bg:EntryColor)}
          G = {New PlateBar init(parent:PlateFrame.inner size:D)}
       in
          (grid(row:D column:1 L padx:Pad pady:Pad sticky:n) |
           grid(row:D column:2 E padx:Pad pady:Pad sticky:n) |
           grid(row:D column:3 G padx:Pad pady:Pad sticky:n) | Tcls)
       end nil}

      TargetFrame = {New TkTools.textframe
                     tkInit(parent: self
                            font:   Helv
                            text:  'Target Plate')}

      XL = {New Tk.label
            tkInit(parent: TargetFrame.inner
                   font:   Helv
                   text:   'X')}
      XE = {New TkTools.numberentry
            tkInit(parent: TargetFrame.inner
                   min:    1
                   val:    10
                   max:    50
                   width:  2
                   font:   HelvBold
                   action: proc {$ I}
                              {Dictionary.put self.spec x I}
                           end)}
      {XE.entry tk(configure bg:EntryColor)}
      YL = {New Tk.label tkInit(parent: TargetFrame.inner
                                font:   Helv
                                text:   'Y')}
      YE = {New TkTools.numberentry
            tkInit(parent: TargetFrame.inner
                   min:    1
                   val:    10
                   max:    50
                   width:  2
                   font:   HelvBold
                   action: proc {$ I}
                              {Dictionary.put self.spec y I}
                           end)}
      {YE.entry tk(configure bg:EntryColor)}
      {Dictionary.put self.spec x 10}
      {Dictionary.put self.spec y 10}
   in
      {Tk.batch {Append TicklePackPlates
                 [grid(row:1 column:1 XL padx:Pad pady:Pad sticky:n)
                  grid(row:1 column:2 XE padx:Pad pady:Pad sticky:n)
                  grid(row:1 column:3 {New Tk.canvas
                                       tkInit(parent:TargetFrame.inner
                                              width:30
                                              height:1)} sticky:w)
                  grid(row:1 column:4 YL padx:Pad pady:Pad sticky:w)
                  grid(row:1 column:5 YE padx:Pad pady:Pad sticky:w)
                  grid(row:1 column:1 padx:Pad pady:Pad PlateFrame)
                  grid(row:2 column:1 padx:Pad pady:Pad TargetFrame
                       sticky:ew)]}}
   end

   meth getSpec($)
      {Dictionary.toRecord spec self.spec}
   end

end

class ComputeDialog
   from TkTools.note

   meth init(parent:P)
      TkTools.note,tkInit(parent:P text:'Compute')

      ButtonFrame = {New Tk.frame tkInit(parent:self)}

      Reset = {New Tk.button tkInit(parent: ButtonFrame
                                    text:   'Reset'
                                    width:  8
                                    font:   Helv)}
      Next  = {New Tk.button tkInit(parent: ButtonFrame
                                    text:   'Next'
                                    width:  8
                                    font:   Helv)}
      Stop  = {New Tk.button tkInit(parent: ButtonFrame
                                    text:   'Stop'
                                    width:  8
                                    font:   Helv)}
      Anim  = {New Tk.button tkInit(parent: ButtonFrame
                                    text:   'Animate'
                                    width:  8
                                    font:   Helv)}

      GlassFrame = {New Tk.frame tkInit(parent:self)}

      PlateCanvas = {New Tk.canvas tkInit(parent: GlassFrame
                                          bg:     ivory
                                          width:  400
                                          height: 200)}
      H  = {New Tk.scrollbar      tkInit(parent:GlassFrame orient:horizontal
                                         width:13)}
      V  = {New Tk.scrollbar      tkInit(parent:GlassFrame orient:vertical
                                         width:13)}

   in
      {Tk.addXScrollbar PlateCanvas H}
      {Tk.addYScrollbar PlateCanvas V}
      {Tk.batch [grid(columnconfigure GlassFrame    0 weight:1)
                 grid(rowconfigure    GlassFrame    0 weight:1)
                 grid(PlateCanvas row:0 column:0 sticky:nsew)
                 grid(H row:1 column:0 sticky:we)
                 grid(V row:0 column:1 sticky:ns)
                 grid(columnconfigure self    2 weight:1)
                 grid(rowconfigure    self    1 weight:1)

                 grid(row:1 column:1 padx:Pad pady:Pad Reset)
                 grid(row:2 column:1 padx:Pad pady:Pad Next)
                 grid(row:3 column:1 padx:Pad pady:Pad Stop)
                 grid(row:4 column:1 padx:Pad pady:Pad
                      {New Tk.canvas tkInit(parent:ButtonFrame
                                            width:0 height:50)})
                 grid(row:5 column:1 padx:Pad pady:Pad Anim)

                 grid(row:1 column:1 ButtonFrame sticky:nw)
                 grid(row:1 column:2 GlassFrame sticky:ne)
                ]}
   end

end

T  = {New Tk.toplevel tkInit(title:'Glass Plates' withdraw:true)}
B  = {New TkTools.notebook tkInit(parent:T)}
N1 = {New EditDialog       init(parent:B)}
N2 = {New ComputeDialog    init(parent:B)}
{B add(N1)}
{B add(N2)}
{Tk.batch [pack(B)
           update(idletasks)
           wm(deiconify T)]}
%{Browse {G getSpec($)}}
