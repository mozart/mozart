functor
import
   Application Pickle URL Resolve
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Browser(browse:Browse)
define
   MOGUL = "./" %"http://www.mozart-oz.org/mogul/"
   INFO  = "ozpm.info"
   Args = {Application.getArgs
	   record(mogul(single type:string default:MOGUL))}
   Info = {Pickle.load {URL.resolve {URL.toBase {Resolve.expand Args.mogul}} INFO}}
   %%
   L1 = {QTk.newLook}
   {L1.set button(relief:groove bg:ivory activebackground:ivory activeforeground:red)}
   {L1.set label(bg:ivory)}
   {L1.set empty(bg:ivory)}
   L2 = {QTk.newLook}
   {L2.set label(fg:darkorange bg:ivory)}
   L3 = {QTk.newLook}
   {L3.set label(fg:darkred bg:ivory)}
   L4 = {QTk.newLook}
   {L4.set label(font: bg:ivory)}
   %%
   proc {GuiPkg P}
      {Browse P}
      %%
      Top = {New Tk.toplevel tkInit}
      Close = {New Tk.button tkInit(parent:Top action:proc {$} {Top tkClose} end)}
      Id = {New Tk.label tkInit(parent:Top text:P.id)}
      local
	 O={QTk.build
	    lr(look:L1
	       label(text:P.id glue:nswe look:L3) continue newline
	       button(text:'Close' action:proc {$} {O tkClose} end glue:nwe)
	       continue newline
	       label(text:{VirtualString.toString P.blurb} glue:nswe look:L4) continue newline
	       label(text:'author' anchor:nw glue:nsew look:L2)
	       td(glue:nsew
		  label(text:'Denys Duchier' glue:nswe anchor:w)
		  label(text:'Nils Franzen' glue:nswe anchor:w))
	      )}
      in
	 {O show}
      end
   end
   %%
   fun {PkgCmp P1 P2}
      {VirtualString.toAtom P1.id}<{VirtualString.toAtom P2.id}
   end
   Buttons =
   {Map {Sort Info.packages PkgCmp}
    fun {$ P} button(text:P.id anchor:w glue:new action:proc {$} {GuiPkg P} end) end}
   {{QTk.build
     {Adjoin td(look:L1)
      {List.toTuple td
       button(text:'Quit' action:proc {$} {Application.exit 0} end glue:new)|Buttons}}}
    show}
end

		 