functor
export
   'class' : Database
import
   Utils at 'Utils.ozf'
   Path  at 'Path.ozf'
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

      meth database_read()
	 if @Mogul2Package==unit then
	    if {self get_database_ignore($)} then
	       Mogul2Package <- {NewDictionary}
	       File2Package  <- {NewDictionary}
	    else
	       %%
	       %% old-style database:
	       %%     ~/.oz/DATABASE.ozf
	       %%     ~/.oz/DATABASE.txt
	       %%
	       %% new-style database:
	       %%     ~/.oz/apps/ozmake/ozmake.db
	       %%
	       %% we need some AI to switch automatically from the old-style
	       %% database to the new style.  If the new-style database is
	       %% not found, then we look for an old-style database, convert
	       %% it to new style, write the new one, and remove the old one.
	       %%
	       F1 = {self get_database($)}
	    in
	       if {Path.exists F1} then
		  Database,Init({self databaselib_read(F1 $)})
	       elseif {self get_database_given($)} then
		  raise ozmake(database:filenotfound(F1)) end
	       else
		  F2 = {self get_database_oldstyle($)}
	       in
		  if {Path.exists F2#'.ozf'} orelse {Path.exists F2#'.txt'} then
		     {self trace('detected old-style database')}
		     {self incr}
		     fun {FromTxt E}
			Database,ByteStringify(E $)
		     end
		     L={self databaselib_read_oldstyle(F2 FromTxt $)}
		  in
		     if L\=unit then
			Database,Init(L)
			if {self get_savedb($)} then
			   {self trace('replacing by new-style database')}
			   {self database_save}
			   if {Path.exists F2#'.ozf'} then
			      {self exec_rm(F2#'.ozf')}
			   end
			   if {Path.exists F2#'.txt'} then
			      {self exec_rm(F2#'.txt')}
			   end
			end
		     else
			{self trace('oh hum... ignoring database')}
			Database,Init(nil)
		     end
		     {self decr}
		  else
		     {self trace('no database')}
		     Database,Init(nil)
		  end
	       end
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
	 if {self get_database_ignore($)} then Skip=false else
	    {self database_read}
	    Grade={self get_grade($)}
	 in
	    if Grade==any then skip else
	       MOG = {self get_mogul($)}
	       VER = {self get_version($)}
	       PKG = {CondSelect @Mogul2Package MOG unit}
	       PVR = {CondSelect PKG version unit}
	    in
	       %% is the package already installed
	       if PKG\=unit then 
		  VComp = case PVR#VER
			  of unit#unit then eq
			  [] unit#_    then gt
			  [] _   #unit then lt
			  else {Utils.versionCompare VER PVR} end
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
		  UsingVersion
		  FullCmp = case VComp#NewOldCmp
			    of lt#_ then UsingVersion=true lt
			    [] gt#_ then UsingVersion=true gt
			    [] _ #C then UsingVersion=false C end
	       in
		  case Grade
		  of none then
		     raise ozmake(database:nograde(OldString NewString PVR VER FullCmp UsingVersion)) end
		  [] same then
		     if FullCmp\=eq then
			raise ozmake(database:samegrade(OldString NewString PVR VER FullCmp UsingVersion)) end
		     end
		  [] up then
		     if FullCmp==lt then
			raise ozmake(database:upgrade(OldString NewString PVR VER UsingVersion)) end
		     end
		  [] down then
		     if FullCmp==gt then
			raise ozmake(database:downgrade(OldString NewString PVR VER UsingVersion)) end
		     end
		  [] freshen then Skip=(FullCmp\=gt)
		  end
	       end
	    end
	    if {IsFree Skip} then Skip=false end
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
	 if {self get_database_ignore($)} then skip else
	    if {self get_savedb($)} then
	       {self databaselib_save(
			{self get_database($)}
			{Map {Dictionary.items @Mogul2Package}
			 fun {$ E}
			    if {IsDictionary E}
			    then {Dictionary.toRecord package E} else E end
			 end})}
	    end
	 end
      end

      meth database_get_packages($)
	 Database,database_read
	 {Dictionary.items @Mogul2Package}
      end

      meth database_get_package(MOG $)
	 Database,database_read
	 {CondSelect @Mogul2Package MOG unit}
      end

      meth database_remove_package(MOG)
	 Database,database_read
	 {Dictionary.remove @Mogul2Package MOG}
      end
   end
end
