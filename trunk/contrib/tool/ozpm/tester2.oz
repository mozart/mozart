functor
import
   Application(getArgs exit)
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Pickle(load)
   URL(resolve toBase)
define
   Args = {Application.getArgs
	   record('mogul'(single type:string
			  default:'http://www.mozart-oz.org/mogul/'))}
   Info = {Pickle.load {URL.resolve {URL.toBase Args.mogul} 'ozpm.info'}}
   L1 = {QTk.newLook}
   {L1.set button(relief:groove bg:ivory activebackground:ivory activeforeground:red)}
   {L1.set label(bg:ivory)}
   {L1.set td(bg:ivory)}
   {L1.set toplevel(bg:ivory)}
   {{QTk.build
     scrollframe(
	{List.toTuple td
	 button(text:'Quit' action:proc {$} {Application.exit 0} end glue:nwe)
	 |{Map Info.packages
	   fun {$ P} button(title:P.id text:P.id glue:nswe anchor:w) end}}
	look:L1
	tdscrollbar:true
	glue:nswe)}
    show}
end
