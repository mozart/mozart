functor
import
   Application(getCmdArgs exit)
   Tk Browser XSL_READ at 'XSL-READ.ozf'
define
   Args = {Application.getCmdArgs
	   record('in'(single type:string optional:false))}
   Doc = {XSL_READ.parseFile Args.'in'}
   {Browser.browse Doc}
   Quit
   Top={New Tk.toplevel tkInit(title:'Oz XSL Processor')}
   But={New Tk.button   tkInit(parent:Top text:'Quit'
			       action:proc {$} Quit=unit end)}
   {Tk.send pack(But)}
   {Wait Quit}
   {Application.exit 0}
end
