functor
export
   'class' : Database
import
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
	    fun {FromTxt E}
	       Database,ByteStringify(E $)
	    end
	    L={self databaselib_read(F FromTxt $)}
	 in
	    if L\=unit then
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

      meth database_check_grade(?Skip)
	 {self database_read}
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
	       CurNext = {AdjoinAt CurDate sec 1+CurDate.sec}
	       OldReleased0 =
	       local D={CondSelect PKG released unit} in
		  if D\=unit then D else
		     D={CondSelect PKG installed unit}
		  in
		     if D\=unit then D else CurDate end
		  end
	       end
	       %% oops! OS.localTime returns time(...) with more features than
	       %% we need.  Here is a fix to accommodate dates recorded before
	       %% I noticed the problem.
	       OldReleased =
	       if {Label OldReleased0}==date then OldReleased0 else
		  date(
		     year : {CondSelect OldReleased0 year 0}
		     mon  : {CondSelect OldReleased0 mon  1}
		     mDay : {CondSelect OldReleased0 mDay 1}
		     hour : {CondSelect OldReleased0 hour 0}
		     min  : {CondSelect OldReleased0 min  0}
		     sec  : {CondSelect OldReleased0 sec  0})
	       end
	       NewReleased =
	       local D={self get_released($)} in
		  if D\=unit then D else CurNext end
	       end
	       NewOldCmp =
	       if NewReleased==OldReleased then eq
	       elseif {Utils.dateLess NewReleased OldReleased} then lt
	       else gt end
	       OldString = {Utils.dateToUserVS OldReleased}
	       NewString = {Utils.dateToUserVS NewReleased}
	    in
	       case Grade
	       of none then
		  raise ozmake(database:nograde(OldString NewString NewOldCmp)) end
	       [] same then
		  if NewOldCmp\=eq then
		     raise ozmake(database:samegrade(OldString NewString NewOldCmp)) end
		  end
	       [] up then
		  if NewOldCmp==lt then
		     raise ozmake(database:upgrade(OldString NewString)) end
		  end
	       [] down then
		  if NewOldCmp==gt then
		     raise ozmake(datebase:downgrade(OldString NewString)) end
		  end
	       [] freshen then Skip=(NewOldCmp\=gt)
	       end
	    end
	 end
	 if {IsFree Skip} then Skip=false end
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

      meth database_stringify(E $)
	 Database,Stringify(E $)
      end
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
	    fun {ToTxt E}
	       Database,Stringify(E $)
	    end
	 in
	    {self databaselib_save(DB ToTxt Entries)}
	 end
      end

      meth database_get_packages($)
	 Database,database_read
	 {Dictionary.items @Mogul2Package}
      end

      meth database_get_package(MOG $)
	 {CondSelect @Mogul2Package MOG unit}
      end

      meth database_remove_package(MOG)
	 {Dictionary.remove @Mogul2Package MOG}
      end
   end
end
