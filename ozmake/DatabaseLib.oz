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
	 {self exec_mkdir({Path.dirname F})}
	 {self trace('writing pickled database '#F#'.ozf')}
	 {self incr}
	 {self exec_save_to_file(L F#'.ozf')}
	 {self decr}
	 {self trace('writing textual database '#F#'.txt')}
	 {self incr}
	 {self exec_writeTextDB({Map L ToTxt} F#'.txt')}
	 {self decr}
      end
	     
   end
end
