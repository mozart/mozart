functor
import
   Application(getCmdArgs exit)
   Tk XML Browser
define
   Args = {Application.getCmdArgs
           record('in'(single type:string optional:false))}
   Parser = {New XML.'class' init}
   Doc    = {Parser parseFile(Args.'in' $)}
   {Browser.browse Doc}
   Quit
   Top={New Tk.toplevel tkInit(title:'Oz XSL Processor')}
   But={New Tk.button   tkInit(parent:Top text:'Quit'
                               action:proc {$} Quit=unit end)}
   {Tk.send pack(But)}
   {Wait Quit}
   {Application.exit 0}
end
