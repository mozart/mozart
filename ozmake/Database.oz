functor
export
   'class' : Database
import
   Pickle
   Path  at 'Path.ozf'
   Utils at 'Utils.ozf'
prepare
   VSToString = VirtualString.toString
   MakeByteString = ByteString.make
define
   class Database

      attr
	 Mogul2Package : unit
	 File2Package  : unit

      meth file_to_package(F $)
	 {CondSelect @File2Package F unit}
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
		      {Map {Utils.readTextDB F#'.txt'}
		       fun {$ E} Database,ByteStringify(E $) end}
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
	 Mogul2Package<-{NewDictionary}
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

      meth database_mutable_entry(MOG $)
	 Pkg={CondSelect @Mogul2Package MOG unit}
      in
	 if Pkg==unit then D={NewDictionary} in
	    D.mogul := MOG
	    @Mogul2Package.MOG := D
	    D
	 elseif {IsDictionary Pkg} then Pkg
	 else D={Record.toDictionary Pkg} in
	    @Mogul2Package.MOG := D
	    D
	 end
      end

      %% replace byte strings by strings for textual output

      meth Stringify(E $)
	 Author = {CondSelect E author    unit}
	 Blurb  = {CondSelect E blurb     unit}
	 IText  = {CondSelect E info_text unit}
	 IHtml  = {CondSelect E info_html unit}
      in
	 if (Author\=unit andthen Author\=nil) orelse
	    Blurb\=unit orelse IText\=unit orelse IHtml\=unit
	 then
	    D={Record.toDictionary E}
	 in
	    if Author\=unit andthen Author\=nil then
	       D.author := {Map Author VSToString}
	    end
	    if Blurb\=unit then D.blurb     := {VSToString Blurb} end
	    if IText\=unit then D.info_text := {VSToString IText} end
	    if IHtml\=unit then D.info_html := {VSToString IHtml} end
	    {Dictionary.toRecord package D}
	 else
	    E
	 end
      end

      %% replace strings by byte strings when reading textual input

      meth ByteStringify(E $)
	 Author = {CondSelect E author    unit}
	 Blurb  = {CondSelect E blurb     unit}
	 IText  = {CondSelect E info_text unit}
	 IHtml  = {CondSelect E info_html unit}
      in
	 if (Author\=unit andthen Author\=nil) orelse
	    Blurb\=unit orelse IText\=unit orelse IHtml\=unit
	 then
	    D={Record.toDictionary E}
	 in
	    if Author\=unit andthen Author\=nil then
	       D.author := {Map Author MakeByteString}
	    end
	    if Blurb\=unit then D.blurb     := {MakeByteString Blurb} end
	    if IText\=unit then D.info_text := {MakeByteString IText} end
	    if IHtml\=unit then D.info_html := {MakeByteString IHtml} end
	    {Dictionary.toRecord package D}
	 else
	    E
	 end
      end

      meth database_save
	 if {self get_savedb($)} then
	    Entries =
	    {Map {Dictionary.items @Mogul2Package}
	     fun {$ E}
		if {IsDictionary E}
		then {Dictionary.toRecord package E} else E end
	     end}
	    DB = {self get_database($)}
	 in
	    {self trace('writing pickled database '#DB#'.ozf')}
	    if {self get_justprint($)} then skip else
	       {Pickle.save Entries DB#'.ozf'}
	    end
	    {self trace('writing textual database '#DB#'.txt')}
	    if {self get_justprint($)} then skip else
	       {Utils.writeTextDB
		{Map Entries fun {$ E} Database,Stringify(E $) end}
		DB#'.txt'}
	    end
	 end
      end

      meth database_get_packages($)
	 Database,database_read
	 {Dictionary.items @Mogul2Package}
      end

      meth database_get_package(MOG $)
	 {CondSelect @Mogul2Package MOG unit}
      end
   end
end
