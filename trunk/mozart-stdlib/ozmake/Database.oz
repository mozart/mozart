functor
export
   'class' : Database
import
   Utils at 'Utils.ozf'
define
   class Database

      attr
	 Mogul2Package : unit
	 File2Package  : unit

      meth database_read
	 if @Mogul2Package==unit then
	    F={self get_database($)}
	 in
	    if {Path.exists F#'.txt'} then
	       L = try
		      {self trace('reading pickled database '#F#'.ozf')}
		      {Pickle.load F#'.ozf'}
		   catch error(...) then
		      {self trace('...failed')}
		      {self trace('retrying with text database '#F#'.txt')}
		      {Utils.readTextDB F#'.txt'}
		   end
	    in
	       Database,Init(L)
	    elseif {self get_database_given($)} then
	       raise ozmake(database:filenotfound(F#'.txt')) end
	    else
	       {self trace('no database')}
	       Database,Init(nil)
	    end
	 end
      end

      meth Init(L)
	 Mogul2PackageList<-{NewDictionary}
	 File2Package<-{NewDictionary}
	 for PKG in L do MOG=PKG.mogul in
	    @Mogul2Package.MOG := PKG
	    for F in PKG.files do
	       @File2Package.F := MOG
	    end
	 end
      end

      meth database_check_grade
	 Grade={self get_grade($)}
      in
	 if Grade==any then skip else
	    Database,database_read
	    MOG = {self get_mogul($)}
	    PKG = {CondSelect @Mogul2Package MOG unit}
	 in
	    %% is the package already installed
	    if PKG\=unit then
	       %% make sure we actually get dates (not unit)
	       CurDate = {Utils.dateCurrent}
	       OldReleased =
	       local D={CondSelect PKG released unit} in
		  if D\=unit then D else
		     D={CondSelect PKG installed unit}
		  in
		     if D\=unit then D else CurDate end
		  end
	       end
	       NewReleased =
	       local D={self get_released($)} in
		  if D\=unit then D else CurDate end
	       end
	    in
	       case Grade
	       of same then
		  if OldReleased\=NewReleased then
		     raise ozmake(database:samegrade(OldReleased NewReleased)) end
		  end
	       [] up then
		  if NewReleased==unit orelse
		     (OldReleased\=unit andthen {Utils.dateLess NewReleased OldReleased})
		  then
		     raise ozmake(database:upgrade(OldReleased NewReleased)) end
		  end
	       [] down then
		  if 
      end
   end
end
