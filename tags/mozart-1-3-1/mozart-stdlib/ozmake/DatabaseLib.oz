functor
export
   'class' : DatabaseLib
import
   Pickle
   Pickler at 'Pickler.ozf'
   Path    at 'Path.ozf'
   Utils   at 'Utils.ozf'
define
   class DatabaseLib
      
      meth databaselib_read(F $)
	 if {Path.exists F} then
	    {self trace('reading database '#F)}
	    {Pickler.fromFile F}
	 else unit end
      end

      meth databaselib_read_oldstyle(F FromTxt L) Done in
	 if {Path.exists F#'.ozf'} then
	    try
	       {self trace('reading old-style pickled database '#F#'.ozf')}
	       LL={Pickle.load F#'.ozf'}
	    in
	       Done=unit
	       L=LL
	    catch _ then
	       {self trace('...failed')}
	    end
	 end
	 if {IsFree Done} andthen {Path.exists F#'.txt'} then
	    try
	       {self trace('reading old-style textual database '#F#'.txt')}
	       LL={Map {Utils.readTextDB F#'.txt'} FromTxt}
	    in
	       Done=unit
	       L=LL
	    catch _ then
	       {self trace('...failed')}
	    end
	 end
	 if {IsFree Done} then
	    {self trace('ignoring old-style database')}
	    L=unit
	 end
      end

      meth databaselib_save(F L)
	 {self exec_pickle_to_file(L F)}
      end
	     
   end
end
