functor
import
   Application
   Property
   System
   Pickle
   SAX at 'SAX.ozf'
   Parser at 'Parser.ozf'
define
   Args={Application.getArgs
	 record(
	    sax(single type:bool default:false)
	    output(single type:string optional:true char:&o))}
   proc {TimeIt Label P}
      T1={Property.get 'time.total'}
      {P}
      T2={Property.get 'time.total'}
   in
      {System.showInfo Label#T2-T1}
   end
   DOC
   {TimeIt
    'Total parse time: '
    proc {$}
       if Args.sax then
	  DOC={New SAX.parser initLazyFromURL(Args.1.1)}.root
       else
	  DOC={Parser.makeLazyFromURL Args.1.1}
       end
    end}
   if {HasFeature Args output} then
      {TimeIt
       'Total save time: '
       proc {$} {Pickle.save DOC Args.output} end}
   end
   {Application.exit 0}
end