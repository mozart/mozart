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

      meth file_to_package(F $)
	 {CondSelect @File2Package F unit}
      end

      meth package_to_files(Mog $)
	 {CondSelect {CondSelect @Mogul2Package Mog unit} files nil}
      end

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
	       OldString = {Utils.dateToUserVS OldReleased}
	       NewString = {Utils.dateToUserVS NewReleased}
	    in
	       case Grade
	       of none then
		  raise ozmake(database:nograde(OldString NewString)) end
	       [] same then
		  if OldReleased\=NewReleased then
		     raise ozmake(database:samegrade(OldString NewString)) end
		  end
	       [] up then
		  if {Utils.dateLess NewReleased OldReleased} then
		     raise ozmake(database:upgrade(OldString NewString)) end
		  end
	       [] down then
		  if {Utils.dateLess OldReleased NewReleased} then
		     raise ozmake(datebase:downgrade(OldString NewString)) end
		  end
	       end
	    end
	 end
      end

      meth database_lost(F)
	 Mog=@File2Package.F
	 Pkg=@Mogul2Package.Mog
	 Files={CondSelect Pkg files nil}
	 Lost={CondSelect Pkg lost nil}
	 NewFiles={Filter Files fun {$ FF} FF\=F end}
	 NewLost=F|Lost
	 NewPkg={AdjointAt {AdjoinAt Pkg files NewFiles} lost NewLost}
      in
	 Pkg.files := NewFiles
	 Pkg.lost := NewLost
      end

      meth database_entry(MOG $)
	 Pkg={CondSelect @Mogul2Package MOG unit}
      in
	 if Pkg==unit then D={NewDictionary} in
	    @Mogul2Package.MOG := D
	    D
	 else Pkg end
      end
   end
end
