functor
export
   'class' : Extractor
import
   URL Resolve OS Pickle
   Utils   at 'Utils.ozf'
   Path    at 'Path.ozf'
   Pickler at 'Pickler.ozf'
prepare
   fun {UnByteStringify V}
      if {ByteString.is V} then {ByteString.toString V}
      elseif {IsString V} then V
      elseif {IsRecord V} then
	 {Record.map V UnByteStringify}
      else V end
   end
define
   class Extractor

      meth PreExtract(REC)
	 REC = Extractor,Load({self get_package($)} $)
	 local ID = {CondSelect {CondSelect REC info unit} mogul unit} in
	    if ID\=unit then
	       {self set_xmogul(ID)}
	    end
	 end
	 %% a few sanity checks.  however, the main sanity checks
	 %% will be done when reading in the makefile
	 if {Not {IsRecord REC}} then
	    raise ozmake(extract:badrecord(REC)) end
	 end
	 if {Not {HasFeature REC info}} then
	    raise ozmake(extract:noinfo(REC)) end
	 end
	 if {Not {HasFeature REC data}} then
	    raise ozmake(extract:nodata(REC)) end
	 end
	 if {Not {IsList REC.data}} then
	    raise ozmake(extract:baddata(REC.data)) end
	 end
	 %% perform sanity checks on the data files to make sure that
	 %% they have relative filenames, that their data is actually
	 %% a virtual string
	 for X in REC.data do
	    case X of F#D then
	       if {Not {IsVirtualString F}} then
		  raise ozmake(extract:filenotvs(F)) end
	       elseif {Not {Path.isRelative F}} then
		  raise ozmake(extract:filenotrelative(F)) end
	       elseif {Not {IsVirtualString D}} then
		  raise ozmake(extract:filedatanotvs(F D)) end
	       end
	    else raise ozmake(extract:baddatapair(X)) end end
	 end
      end

      meth extract_makefile(?REC)
	 {self PreExtract(REC)}
	 {self set_fromPackage(true)}
	 {self makefile_from_record(REC.info)}
	 {self set_no_makefile(false)}
      end

      meth extract_files(REC writeMakefile:MAK<=true writeFiles:WRITE<=true)
	 %% when we install from a package file, we don't actually
	 %% need to write the makefile because it is already read
	 %% in memory when we read the package file: that's the
	 %% reason for the MAK argument.
	 DIR = if WRITE then {self get_extractdir($)} else unit end
      in
	 %% write out the files
	 if WRITE then
	    for F#D in REC.data do
	       {self exec_write_to_file(D {Path.resolve DIR F})}
	    end
	 end
	 if MAK andthen WRITE then
	    Extractor,WriteMakefile(DIR REC.info)
	    {self set_submakefiles(unit)}
	 end
	 if {Not MAK} orelse {Not WRITE} orelse {self get_justprint($)} then
	    %% when installing from a package, we don't actually need to
	    %% write the makefile and read it again.  Also, during a dry
	    %% run, we won't do that anyway, but we don't want the errors
	    %% that come from _not_ having a makefile.  Therefore, in
	    %% both cases, we initialize the makefile info from the package
	    %% record we just read, but we don't actually write out the
	    %% makefile.
	    {self set_fromPackage(true)}
	    {self makefile_from_record(REC.info)}
	    {self set_no_makefile(false)}
	 end
      end

      meth extract(writeMakefile:MAK<=true writeFiles:WRITE<=true extracted:EX<=_)
	 REC
      in
	 {self extract_makefile(REC)}
	 if {self get_must_extract($)}
	    orelse {Not {self database_check_grade($)}}
	 then
	    {self extract_files(REC writeMakefile:MAK writeFiles:WRITE)}
	    EX=true
	 else
	    {self xtrace('No need to freshen: skipping extraction')}
	    EX=false
	 end
      end

      meth WriteMakefile(DIR R)
	 {self exec_write_to_file(
		  {Value.toVirtualString
		   {UnByteStringify
		    {Record.subtract R submakefiles}}
		   1000000 1000000}
		  {Path.resolve DIR "MAKEPKG.oz"})}
	 %% write a makefile.oz if none was provided in the package
	 %% this is for compatibility with older packages
	 if {Not {self exec_exists({Path.resolve DIR "makefile.oz"} $)}} then
	    {self exec_write_to_file(
		     {Value.toVirtualString
		      {UnByteStringify
		       {Record.subtract R submakefiles}}
		      1000000 1000000}
		     {Path.resolve DIR "makefile.oz"})}
	 end
	 for DD#RR in {Record.toListInd {CondSelect R submakefiles o}} do
	    Extractor,WriteMakefile({Path.resolve DIR DD} RR)
	 end
      end

      meth Load(PKG $)
	 IS_MOGULID
	 PKG_URL
      in
	 try
	    if {Utils.isMogulID PKG} then
	       !IS_MOGULID=unit
	       %% if the package is given as a mogul id, then we
	       %% download the appropriate file from the mogul
	       %% archive
	       Archive = {self get_archive($)}
	       Filename = {Utils.mogulToFilename PKG}
	       WantVersion = {self get_want_version($)}
	       FilenameVer =
	       if WantVersion==unit then Filename else Filename#'-'#WantVersion end
	       #'.pkg'
	       Url = {URL.resolve {URL.toBase Archive}
		      {Path.resolve
		       {Utils.mogulToRelative PKG} FilenameVer}}
	       UrlStr = {URL.toString Url}
	    in
	       PKG_URL=UrlStr
	       {self xtrace('downloading '#UrlStr)}
	       local
		  LOC =
		  {{Resolve.make 'ozmake'
		    init([Resolve.handler.default])}.localize
		   Url}
		  %% here we also need the same trick as described below
		  VAL
	       in
		  try
		     try local V={Pickle.load LOC.1} in VAL=V end
		     catch _ then local V={Pickler.fromFile LOC.1} in VAL=V end end
		  finally
		     case LOC of new(F) then
			try {OS.unlink F} catch _ then skip end
		     else skip end
		  end
		  VAL
	       end
	    else
	       %% otherwise, we read it in the usual way, except that
	       %% in order to minimize surprizes, we try the default
	       %% methods first rather than risk getting something
	       %% stale from some cache
	       if {CondSelect {URL.make PKG} scheme unit}\=unit then
		  {self xtrace('downloading package '#PKG)}
	       else
		  {self trace('reading package '#PKG)}
	       end
	       local
		  LOC =
		  {{Resolve.make 'ozmake'
		    init(Resolve.handler.default|
			 {Resolve.pickle.getHandlers})}.localize PKG}
		  %% we need a work around the bad compilation of try
		  %% without this trick the value returned from Pickler.fromFile
		  %% is somehow lost when exiting the try.
		  VAL
	       in
		  try
		     try local V={Pickle.load LOC.1} in VAL=V end
		     catch _ then local V={Pickler.fromFile LOC.1} in VAL=V end end
		  finally
		     case LOC of new(F) then
			try {OS.unlink F} catch _ then skip end
		     else skip end
		  end
		  VAL
	       end
	    end
	 catch error(url(localize U) ...) then
	    if {IsDet IS_MOGULID} then
	       raise ozmake(extract:localize(U PKG)) end
	    else
	       raise ozmake(extract:localize(U)) end
	    end
	 [] urlbad then
	    if {IsDet IS_MOGULID} andthen {IsDet PKG_URL} then
	       raise ozmake(extract:badurl(PKG PKG_URL)) end
	    else
	       raise ozmake(extract:badurl(PKG)) end
	    end
	 [] urlbug then
	    if {IsDet IS_MOGULID} andthen {IsDet PKG_URL} then
	       raise ozmake(extract:badurl(PKG PKG_URL)) end
	    else
	       raise ozmake(extract:badurl(PKG)) end
	    end
	 [] _ then
	    raise ozmake(extract:unpickle(PKG)) end
	 end
      end

      meth load_extract_mogulid(PKG $)
	 S={CondSelect
	    {CondSelect (Extractor,Load(PKG $)) info unit}
	    mogul unit}
      in
	 if S\=unit then {VirtualString.toAtom S} else unit end
      end

   end
end
