functor
import
   Gdbm at 'x-oz://contrib/gdbm'
   Admin(manager:Manager)
   Message(
      parse : Parse
      slurp : Slurp
      parseNodup: ParseNodup
      )
   Text(
      toLower : ToLower
      )
   Section('class')
   Contact('class')
   Package('class')
   Except('raise':Raise)
export
   'class' : Database
define
   NOT_FOUND = {NewName}
   class Database
      prop final
      attr db cache
      meth init(F)
	 db    <- {Gdbm.new create(F)}
	 cache <- {NewDictionary}
      end
      meth Get(K V) {Gdbm.get @db K V} end
      meth get(K V)
	 if {Dictionary.member @cache K} then
	    {Dictionary.get @cache K V}
	 else
	    Database,Intern(Database,Get(K $) V)
	    {Dictionary.put @cache K V}
	 end
      end
      meth Put(K V) {Gdbm.put @db K V} end
      meth put(K V)
	 %% note: we do not write back to the gdbm database
	 %% we should only do this as an explicit action
	 %% triggered by a command line option
	 {Dictionary.put @cache K V}
      end
      meth CondGet(K D V) {Gdbm.condGet @db K D V} end
      meth condGet(K D V)
	 if {Dictionary.member @cache K} then
	    {Dictionary.get @cache K V}
	 else W=Database,CondGet(K NOT_FOUND $) in
	    if W==NOT_FOUND then V=D
	    else
	       Database,Intern(W V)
	       {Dictionary.put @cache K V}
	    end
	 end
      end
      meth save_db
	 for E in {Dictionary.entries @cache} do
	    case E of K#V then
	       {Manager trace('Writing back entry '#K)}
	       Database,Put(K Database,Extern(V $))
	    end
	 end
      end
      meth close {Gdbm.close @db} end
      meth Intern(R $)
	 case R
	 of 'section'(...) then {New Section.'class' internalize(R)}
	 [] 'contact'(...) then {New Contact.'class' internalize(R)}
	 [] 'package'(...) then {New Package.'class' internalize(R)}
	 [] 'value'(E)     then E
	 end
      end
      meth Extern(E $)
	 if {IsObject E} then {E externalize($)} else 'value'(E) end
      end
      %%
      meth updateInfo(Id Url Pid)=M
	 {Manager incTrace('--> updateInfo '#Id)}
	 if {Manager ignoreID(Id $)} then
	    {Manager trace('Ignoring ID='#Id)}
	 elseif {Manager ignoreURL(Url $)} then
	    {Manager trace('Ignoring URL='#Url)}
	 else
	    try
	       {Manager trace('Fetching info from '#Url)}
	       Msg   = try {Parse {Slurp Url}}
		       catch _ then {Raise mogul(notFound(Url))} unit end
	       {Msg check_id_expected(Id Pid)}
	       {Msg check_keys(['type'])}
	       Type  = {VirtualString.toAtom {ToLower {Msg get1('type' $)}}}
	       Class = case Type
		       of 'contact' then Contact.'class'
		       [] 'package' then Package.'class'
		       [] 'section' then Section.'class'
		       [] T         then {Raise mogul(wrong_type(T))} unit
		       end
	       Prev  = Database,condGet(Id unit $)
	       Entry = {New Class init(Msg Id Url Pid Prev)}
	    in
	       Database,put({Entry getSlot('id' $)} Entry)
	       if Type=='section' then
		  {Manager trace('Processing entries')}
		  {Record.forAllInd {Entry getSlot('toc' $)}
		   proc {$ K V}
		      Database,updateInfo(K V Id)
		   end}
	       end
	    catch mogul(...)=E then
	       {Manager addReport(M E)}
	    finally
	       {Manager decTrace('<-- updateInfo '#Id)}
	    end
	 end
      end
      %%
      meth printOut(Id Margin Out)
	 case Database,condGet(Id unit $)
	 of unit then 
	    {Out write(vs:Margin#Id#' not found!!!\n')}
	 [] E then
	    {E printOut(Margin Out self)}
	 end
      end
      %%
      meth updatePub(Id)=M
	 if {Manager ignoreID(Id $)} then
	    {Manager trace('Ignoring ID='#Id)}
	 else
	    try
	       case Database,condGet(Id unit $)
	       of unit then
		  {Raise mogul(entry_not_found(Id))}
	       [] E then {E updatePub(self)} end
	    catch mogul(...)=E then
	       {Manager addReport(M E)}
	    end
	 end
      end
      %%
      meth updateHtml(Id)=M
	 try
	    case Database,condGet(Id unit $)
	    of unit then
	       {Raise mogul(entry_not_found(Id))}
	    [] E then {E updateHtml(self)} end
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	 end
      end
      %%
      meth updateProvided(ID D)=M
	 try
	    case Database,condGet(ID unit $)
	    of unit then
	       {Raise mogul(entry_not_found(ID))}
	    [] E then
	       {E updateProvided(self D)}
	       Database,put('*provided*' {Dictionary.toRecord o D})
	    end
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	 end
      end
      %%
      meth updateCategories(U)=M
	 try
	    Msg = {ParseNodup {Slurp U}}
	    R={List.toRecord o
	       {Map {Msg entries($)}
		fun {$ C#[V]}
		   C#o(normalized:{NormalizeCat C}
		       description:V)
		end}}
	 in
	    {Manager set_categories(R)}
	    Database,put('*categories*' R)
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	 end
      end
      %%
      meth updateAuthorList(ID)=M
	 try
	    case Database,condGet(ID unit $)
	    of unit then
	       {Raise mogul(entry_not_found(ID))}
	    [] E then
	       Ls={E updateAuthorList(self nil $)}
	       DB={NewDictionary}
	       proc{InsertDB X} Id=X.id in
		  for A in X.authors do
		     Key={String.toAtom {ByteString.toString A}}
		  in
		     {Dictionary.put DB Key Id|{Dictionary.condGet DB Key nil}}
		  end
	       end
	       {ForAll Ls InsertDB}
	       L={Dictionary.toRecord authors DB}
	    in
	       Database,put('*author list*' L)
	    end
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	 end
      end
      meth updateAuthorListFor(ID L $)=M
	 try
	    case Database,condGet(ID unit $)
	    of unit then
	       {Raise mogul(entry_not_found(ID))} unit
	    [] E then {E updateAuthorList(self L $)} end
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	    L
	 end
      end
      %%
      meth updatePkgList(ID)=M
	 try
	    case Database,condGet(ID unit $)
	    of unit then
	       {Raise mogul(entry_not_found(ID))}
	    [] E then L={E updatePkgList(self nil $)} in
	       Database,put('*package list*' L)
	    end
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	 end
      end
      meth updatePkgListFor(ID L $)=M
	 try
	    case Database,condGet(ID unit $)
	    of unit then
	       {Raise mogul(entry_not_found(ID))} unit
	    [] E then {E updatePkgList(self L $)} end
	 catch mogul(...)=E then
	    {Manager addReport(M E)}
	    L
	 end
      end
      %%
      meth cat2Packages(C $)
	 {Filter
	  {Map Database,condGet('*package list*' nil $)
	   fun {$ Id} Database,condGet(Id unit $) end}
	  fun {$ X} X\=unit end}
      end
      %%
      meth get_ozpm_info($)
	 info(
	    authors:
		{Filter
		 {Map {Arity Database,condGet('*author list*' nil $)}
		  fun {$ ID} Database,CondGet(ID unit $) end}
		 fun {$ X} X\=unit end}
	    packages:
	       {Filter
		{Map Database,condGet('*package list*' nil $)
		 fun {$ ID} Database,CondGet(ID unit $) end}
		fun {$ X} X\=unit end})
      end
   end
   %%
   local
      fun {Loop S}
	 case S of nil then nil
	 [] H|T then
	    if {Char.isAlNum H} orelse H==&- orelse H==&_ orelse H==&.
	    then H else &: end
	    |{Loop T}
	 end
      end
   in
      fun {NormalizeCat C}
	 {Loop {VirtualString.toString C}}
      end
   end
end
