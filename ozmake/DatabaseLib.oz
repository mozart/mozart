functor
export
   'class' : DatabaseLib
import
   Pickle
   Path  at 'Path.ozf'
   Utils at 'Utils.ozf'
define
   class DatabaseLib
      
      meth databaselib_read(F FromTxt $)
	 if {Path.exists F#'.txt'} then
	    try
	       {self trace('reading pickled database '#F#'.ozf')}
	       {Pickle.load F#'.ozf'}
	    catch error(...) then
	       {self trace('...failed')}
	       {self trace('retrying with textual database '#F#'.txt')}
	       {Map {Utils.readTextDB F#'.txt'} FromTxt}
	    end
	 else unit end
      end

      meth databaselib_save(F ToTxt L)
	 {Path.makeDirRec {Path.dirname F}}
	 {self trace('writing pickled database '#F#'.ozf')}
	 if {self get_justprint($)} then skip else
	    {Pickle.save L F#'.ozf'}
	 end
	 {self trace('writing textual database '#F#'.txt')}
	 if {self get_justprint($)} then skip else
	    {Utils.writeTextDB {Map L ToTxt} F#'.txt'}
	 end
      end
	     
   end
end
