functor
export
   'class' : Makefile
import
   Property
   Path at 'Path.ozf'
   Utils at 'Utils.ozf'
prepare
   MakeByteString = ByteString.make

   %% a rule is represented by a record
   %% rule(tool:TOOL file:FILE options:[...])

   RULE_EXISTS = rule(tool:unit file:unit options:nil)

   VALID_MAKEFILE_FEATURES = [bin lib doc src depends rules uri mogul author released clean veryclean
			      blurb info_text info_html subdirs submakefiles requires categories version
			      contact tar provides platform]

define

   fun {MakeRuleDefault Target}
      case {Path.extensionAtom Target}
      of 'exe' then rule(tool:ozl file:{Path.replaceExtensionAtom Target 'ozf'} options:[executable])
      [] 'ozf' then rule(tool:ozc file:{Path.replaceExtensionAtom Target 'oz' } options:nil)
      [] 'o'   then rule(tool:cc  file:{Path.replaceExtensionAtom Target 'cc' } options:nil)
      [] 'so'  then rule(tool:ld  file:{Path.replaceExtensionAtom Target 'o'  } options:nil)
      else RULE_EXISTS end
   end

   class Makefile

      attr
	 Target2Section : unit
	 Target2Rule    : unit
	 Target2Depends : unit
	 TargetIsSource : unit

      meth target_to_section(T $)
	 {CondSelect @Target2Section {Path.toAtom T} unit}
      end

      meth makefile_from_record(R)
	 Target2Section<-{NewDictionary}
	 Target2Rule   <-{NewDictionary}
	 Target2Depends<-{NewDictionary}
	 TargetIsSource<-{NewDictionary}

	 %% R should be a record representing a valid makefile.
	 %% we extract from it the information it contains and
	 %% make very careful sanity checks of everything

	 if {Not {IsRecord R}} then
	    raise ozmake(makefile:notrecord(R)) end
	 end

	 %% check that all features are expected

	 for F in {Arity R} do
	    if {Not {Member F VALID_MAKEFILE_FEATURES}} then
	       raise ozmake(makefile:illegalfeature(F VALID_MAKEFILE_FEATURES)) end
	    end
	 end

	 %% process uri feature

	 if {HasFeature R uri} then
	    U = try {Path.toURL R.uri} catch _ then
		   raise ozmake(makefile:baduri(R.uri)) end
		end
	 in
	    if {CondSelect U scheme unit}==unit then
	       raise ozmake(makefile:urinoscheme(R.uri)) end
	    else
	       {self set_uri({Path.toAtom U})}
	    end
	 end

	 %% process mogul feature

	 if {HasFeature R mogul} then
	    U = try {Path.toURL R.mogul} catch _ then
		   raise ozmake(makefile:badmogul(R.mogul)) end
		end
	 in
	    if {CondSelect U scheme unit}\="mogul" then
	       raise ozmake(makefile:mogulbadscheme(R.mogul)) end
	    else
	       %% normalize MOGUL ID, i.e. no trailing slash
	       {self set_mogul({Utils.cleanMogulID U})}
	    end
	 end

	 %% process released feature

	 if {HasFeature R released} then
	    try {self set_released({Utils.dateParse R.released})} catch _ then
	       raise ozmake(makefile:badreleased(R.released)) end
	    end
	 end

	 %% process clean feature

	 if {HasFeature R clean} then
	    if {Not {IsList R.clean}} orelse {Not {All R.clean IsVirtualString}} then
	       raise ozmake(makefile:badclean(R.clean)) end
	    else
	       {self set_clean(R.clean)}
	    end
	 end

	 %% process veryclean feature

	 if {HasFeature R veryclean} then
	    if {Not {IsList R.veryclean}} orelse {Not {All R.veryclean IsVirtualString}} then
	       raise ozmake(makefile:badveryclean(R.veryclean)) end
	    else
	       {self set_veryclean(R.veryclean)}
	    end
	 end

	 %% process version feature

	 if {HasFeature R version} then
	    if {Utils.isVersion R.version} then
	       {self set_version(R.version)}
	    else
	       raise ozmake(makefile:badversion(R.version)) end
	    end
	 end

	 %% process author feature

	 local S={CondSelect R author nil} in
	    if S==nil then skip
	    elseif {IsVirtualString S} then
	       if {Utils.authorOK S} then
		  %% normalize MOGUL ID, i.e. no trailing slash
		  S2 = if {Utils.isMogulID S}
		       then {Utils.cleanMogulID S}
		       else S end
	       in
		  {self set_author([{MakeByteString S2}])}
	       else
		  raise ozmake(makefile:authornotok(S)) end
	       end
	    elseif {Not {IsList S}} orelse {Not {All S IsVirtualString}} then
	       raise ozmake(makefile:badauthor(S)) end
	    else
	       {self set_author(
			for X in S collect:COL do
			   if {Utils.authorOK X} then
			      {COL
			       {MakeByteString
				%% normalize MOGUL ID, i.e. no trailing slash
				if {Utils.isMogulID X}
				then {Utils.cleanMogulID X}
				else X end}}
			   else
			      raise ozmake(makefile:authornotok(X)) end
			   end
			end)}
	    end
	 end

	 %% process blurb feature

	 if {HasFeature R blurb} then
	    if {Not {IsVirtualString R.blurb}} then
	       raise ozmake(makefile:badblurb(R.blurb)) end
	    else
	       {self set_blurb({MakeByteString R.blurb})}
	    end
	 end

	 %% process info_text feature

	 if {HasFeature R info_text} then
	    if {Not {IsVirtualString R.info_text}} then
	       raise ozmake(makefile:badinfotext(R.info_text)) end
	    else
	       {self set_info_text({MakeByteString R.info_text})}
	    end
	 end

	 %% process info_html feature

	 if {HasFeature R info_html} then
	    if {Not {IsVirtualString R.info_html}} then
	       raise ozmake(makefile:badinfohtml(R.info_html)) end
	    else
	       {self set_info_html({MakeByteString R.info_html})}
	    end
	 end

	 %% process requires feature

	 case {CondSelect R requires nil}
	 of nil then skip
	 [] S then
	    if {IsVirtualString S} then
	       {self set_requires([S])}
	    elseif {IsList S} andthen {All S IsVirtualString} then
	       {self set_requires(S)}
	    else
	       raise ozmake(makefile:badrequires(S)) end
	    end
	 end

	 %% process categories feature
	 if {HasFeature R categories} then
	    if {IsVirtualString R.categories} then
	       {self set_categories([R.categories])}
	    elseif {IsList R.categories} andthen {All R.categories IsVirtualString} then
	       {self set_categories(R.categories)}
	    else
	       raise ozmake(makefile:badcategories(R.categories)) end
	    end
	 end

	 %% process platform feature
	 if {HasFeature R platform} then
	    if {IsVirtualString R.platform} then
	       {self set_platform(R.platform)}
	    else
	       raise ozmake(makefile:badplatform(R.platform)) end
	    end
	 end

	 %% process contact feature
	 if {HasFeature R contact} then
	    L = case R.contact
		of _|_ then R.contact
		else [R.contact] end
	 in
	    for C in L do
	       if {IsRecord C} then
		  for F#V in {Record.toListInd C} do
		     case F
		     of mogul then
			if {IsVirtualString V} andthen {Utils.isMogulID V} then skip
			else raise ozmake(makefile:contactmogul(V)) end end
		     [] name  then
			if {Not {IsVirtualString V}} then
			   raise ozmake(makefile:contactname(V)) end
			end
		     [] name_for_index then
			if {Not {IsVirtualString V}} then
			   raise ozmake(makefile:contactnameforindex(V)) end
			end
		     [] email then
			if {Not {IsVirtualString V}} then
			   raise ozmake(makefile:contactemail(V)) end
			end
		     [] www   then
			if {Not {IsVirtualString V}} then
			   raise ozmake(makefile:contactwww(V)) end
			end
		     else
			raise ozmake(makefile:illegalfeature(F V)) end
		     end
		  end
		  if {Not {HasFeature C mogul}} then
		     raise ozmake(makefile:contactmissingmogul(C)) end
		  end
		  if {Not {HasFeature C name}} then
		     raise ozmake(makefile:contactmissingname(C)) end
		  end
	       else
		  raise ozmake(makefile:badcontact(C)) end
	       end
	    end
	    {self set_contact(
		     %% make sure that MOGUL IDs have been normalized
		     %% i.e. no trailing slahes
		     {Map L
		      fun {$ C}
			 if {HasFeature C mogul} then
			    {AdjoinAt C mogul {Utils.cleanMogulID C.mogul}}
			 else C end
		      end})}
	 end

	 %% process subdirs feature

	 if {HasFeature R subdirs} then
	    if {IsList R.subdirs} then
	       for D in R.subdirs do
		  if {Not {IsVirtualString D}} then
		     raise ozmake(makefile:subdirnotvs(D)) end
		  elseif {Not {Path.isBasename D}} then
		     raise ozmake(makefile:subdirnotbasename(D)) end
		  end
	       end
	    else
	       raise ozmake(makefile:badsubdirs(R.subdirs)) end
	    end
	    {self set_subdirs({Map R.subdirs Path.toAtom})}
	 end

	 %% process submakefiles feature

	 if {HasFeature R submakefiles} then
	    if {Not {IsRecord R.submakefiles}} then
	       raise ozmake(makefile:badsubmakefiles(R.submakefiles)) end
	    elseif {Width R.submakefiles}\=0
	       andthen {Not {self get_fromPackage($)}}
	    then
	       raise ozmake(makefile:submakefilesnotallowed) end
	    else
	       {self set_submakefiles(R.submakefiles)}
	    end
	 end

	 %% process bin feature

	 local Stack={Utils.newStack} BIN={CondSelect R bin nil} in
	    if {Not {IsList BIN}} then
	       raise ozmake(makefile:badsectionvalue(bin BIN)) end
	    end
	    for F in BIN do U A in
	       if {Not {IsVirtualString F}} then
		  raise ozmake(makefile:badsectionentry(bin F)) end
	       end
	       %% a bin target must be just a filename without directory
	       U={Path.toURL F}
	       if {Path.isAbsolute U} orelse
		  {CondSelect U path unit}==unit orelse
		  {Length U.path}\=1
	       then
		  raise ozmake(makefile:badsectiontarget(bin F)) end
	       end
	       %% a bin target needs a 'exe' extension
	       case {Path.extensionAtom F}
	       of 'exe' then skip
	       else raise ozmake(makefile:badbinextension(F)) end end
	       %% check that it is unique
	       A = {Path.toAtom U}
	       if {HasFeature @Target2Section A} then
		  raise ozmake(makefile:duplicate(A)) end
	       end
	       @Target2Section.A := bin
	       {Stack.push A}
	    end
	    {self set_bin_targets({Reverse {Stack.toList}})}
	 end

	 %% process lib feature

	 local Stack={Utils.newStack} LIB={CondSelect R lib nil} in
	    if {Not {IsList LIB}} then
	       raise ozmake(makefile:badsectionvalue(lib LIB)) end
	    end
	    for F in LIB do U A in
	       if {Not {IsVirtualString F}} then
		  raise ozmake(makefile:badsectionentry(lib F)) end
	       end
	       %% must be a relative pathname
	       U={Path.toURL F}
	       if {Path.isAbsolute U} then
		  raise ozmake(makefile:badsectiontarget(lib F)) end
	       end
	       %% check that it is unique
	       A={Path.toAtom U}
	       if {HasFeature @Target2Section A} then
		  raise ozmake(makefile:duplicate(A)) end
	       end
	       @Target2Section.A := lib
	       {Stack.push A}
	    end
	    {self set_lib_targets({Reverse {Stack.toList}})}
	 end

	 %% process provides feature

	 if {HasFeature R provides} then
	    local Stack={Utils.newStack} PRO={CondSelect R provides nil} in
	       if {Not {IsList PRO}} then
		  raise ozmake(makefile:badsectionvalue(provides PRO)) end
	       end
	       for F in PRO do U A in
		  if {Not {IsVirtualString F}} then
		     raise ozmake(makefile:badsectionentry(provides F)) end
		  end
		  %% must be a relative pathname
		  U={Path.toURL F}
		  if {Path.isAbsolute U} then
		     raise ozmake(makefile:badsectiontarget(lib F)) end
		  end
		  %% check that it is actually a bin or lib target
		  A={Path.toAtom U}
		  case {CondSelect @Target2Section A unit}
		  of unit then
		     raise ozmake(makefile:unknownprovides(A)) end
		  [] bin  then {Stack.push A}
		  [] lib  then {Stack.push A}
		  [] S    then
		     raise ozmake(makefile:badprovides(S A)) end
		  end
	       end
	       {self set_provides_targets({Reverse {Stack.toList}})}
	    end
	 end

	 %% process doc feature

	 local Stack={Utils.newStack} DOC={CondSelect R doc nil} in
	    if {Not {IsList DOC}} then
	       raise ozmake(makefile:badsectionvalue(doc DOC)) end
	    end
	    for F in DOC do U A in
	       if {Not {IsVirtualString F}} then
		  raise ozmake(makefile:badsectionentry(doc F)) end
	       end
	       %% must be a relative pathname
	       U={Path.toURL F}
	       if {Path.isAbsolute U} then
		  raise ozmake(makefile:badsectiontarget(doc F)) end
	       end
	       %% check that it is unique
	       A={Path.toAtom U}
	       if {HasFeature @Target2Section A} then
		  raise ozmake(makefile:duplicate(A)) end
	       end
	       @Target2Section.A := doc
	       {Stack.push A}
	    end
	    {self set_doc_targets({Reverse {Stack.toList}})}
	 end

	 %% process src feature

	 local Stack={Utils.newStack} SRC={CondSelect R src nil} in
	    if {Not {IsList SRC}} then
	       raise ozmake(makefile:badsectionvalue(src SRC)) end
	    end
	    for F in SRC do U A in
	       if {Not {IsVirtualString F}} then
		  raise ozmake(makefile:badsectionentry(src F)) end
	       end
	       %% must be a relative pathname
	       U={Path.toURL F}
	       if {Path.isAbsolute U} then
		  raise ozmake(makefile:badsectiontarget(src F)) end
	       end
	       %% a file may be listed here eventhough it is also listed
	       %% in bin, lib, or doc: this means that the file is to be
	       %% included in the distribution rather than those necessary
	       %% to build it
	       A={Path.toAtom U}
	       @TargetIsSource.A := true
	       {Stack.push A}
	    end
	    {self set_src_targets({Reverse {Stack.toList}})}
	 end

	 %% process tar feature

	 local
	    TAR0={CondSelect R tar unit}
	    TAR =if TAR0==unit then nil
		 elseif {IsVirtualString TAR0} then [TAR0]
		 elseif {IsList TAR0} andthen {All TAR0 IsVirtualString} then TAR0
		 else raise ozmake(makefile:badtarvalue(TAR0)) end end
	 in
	    for X in TAR do
	       if {Not {Member X ['tgz' 'tar.Z' 'tar.gz']}} then
		  raise ozmake(makefile:badtarext(X)) end
	       end
	    end
	    if TAR\=nil then
	       {self set_tar_targets({Map TAR Path.toAtom})}
	    end
	 end

	 %% process depends feature

	 local DEP={CondSelect R depends o} in
	    if {Not {IsRecord DEP}} then
	       raise ozmake(makefile:baddepends(DEP)) end
	    end
	    for F#L in {Record.toListInd DEP} do
	       Stack={Utils.newStack}
	       LL = if {IsVirtualString L} then [L]
		    elseif {IsList L} andthen {All L IsVirtualString} then L
		    else raise ozmake(makefile:baddependslist(F L)) end end
	       A
	    in
	       if {Path.isAbsolute F} then
		  raise ozmake(makefile:baddependstarget(F)) end
	       end
	       A = {Path.toAtom F}
	       %% check that all files are relative
	       for D in LL do
		  if {Not {IsVirtualString D}} then
		     raise ozmake(makefile:baddependsvalue(A D)) end
		  end
		  if {Path.isAbsolute D} then
		     raise ozmake(makefile:baddependsentry(A D)) end
		  end
		  {Stack.push {Path.toAtom D}}
	       end
	       @Target2Depends.A := {Reverse {Stack.toList}}
	    end
	 end

	 %% process rule feature

	 local RUL={CondSelect R rules nil} in
	    if {Not {IsRecord RUL}} then
	       raise ozmake(makefile:badrules(RUL)) end
	    end
	    for F#R in {Record.toListInd RUL} do A Rule in
	       if {Path.isAbsolute F} then
		  raise ozmake(makefile:badrulestarget(F)) end
	       end
	       A = {Path.toAtom F}
	       case {Adjoin o(2:nil) R}
	       of ozc(T Options) then Rule=rule(tool:ozc file:T options:Options)
	       [] ozl(T Options) then Rule=rule(tool:ozl file:T options:Options)
	       [] cc( T Options) then Rule=rule(tool:cc  file:T options:Options)
	       [] ld( T Options) then Rule=rule(tool:ld  file:T options:Options)
	       [] ozg(T Options) then Rule=rule(tool:ozg file:{Path.replaceExtensionAtom T 'ozf'} options:Options)
	       else raise ozmake(makefile:badrule(F R)) end
	       end
	       if {Path.isAbsolute Rule.file} then
		  raise ozmake(makefile:badrulefile(Rule.file)) end
	       end
	       %% check that the options make sense for the tool
	       case Rule.tool
	       of ozc then
		  for O in Rule.options do
		     case O
		     of executable then skip
		     [] 'define'(S) then
			if {Not {IsVirtualString S}} then
			   raise ozmake(makefile:illegaltooloptionarg(ozc O)) end
			end
		     else raise ozmake(makefile:illegaltooloption(ozc O)) end end
		  end
	       [] ozl then
		  for O in Rule.options do
		     case O
		     of executable then skip
			%% we could allow more here e.g. include/exclude
		     else raise ozmake(makefile:illegaltooloption(ozl O)) end end
		  end
	       [] cc then
		  for O in Rule.options do
		     case O
		     of include(S) then
			if {Not {IsVirtualString S}} then
			   raise ozmake(makefile:illegaltooloptionarg(cc O)) end
			end
		     [] 'define'(S) then
			if {Not {IsVirtualString S}} then
			   raise ozmake(makefile:illegaltooloptionarg(cc O)) end
			end
		     else
			raise ozmake(makefile:illegaltooloption(cc O)) end
		     end
		  end
	       [] ld then
		  for O in Rule.options do
		     case O
		     of library(S) then
			if {Not {IsVirtualString S}} orelse {Path.isAbsolute S} then
			   raise ozmake(makefile:badtooloption(cc O)) end
			end
		     else raise ozmake(makefile:illegaltooloption(ld O)) end end
		  end
	       [] ozg then skip
	       end
	       %% record the rule
	       @Target2Rule.A := Rule
	    end
	 end
      end

      meth makefile_read
	 if @Target2Section==unit then S={self get_superman($)} in
	    if S\=unit andthen {S has_submakefile({self get_assubdir($)} $)} then
	       Makefile,makefile_from_record({S get_submakefile({self get_assubdir($)} $)})
	    else
	       F={self get_makefile($)}
	    in
	       if {Path.exists F} then
		  {self trace('reading makefile: '#F)}
		  Makefile,makefile_from_record({Utils.compileFile F false})
		  {self set_no_makefile(false)}
	       elseif {self get_makefile_given($)} then
		  raise ozmake(makefile:filenotfound(F)) end
	       else
		  {self trace('using empty makefile')}
		  Makefile,makefile_from_record(o)
	       end
	    end
	 end
      end

      meth makefile_read_maybe_from_package
	 if @Target2Section==unit then S={self get_superman($)} in
	    if S\=unit andthen {S has_submakefiles({self get_assubdir($)} $)} then
	       Makefile,makefile_from_record({S get_submakefile({self get_assubdir($)} $)})
	    elseif {Not {self get_makefile_given($)}} andthen {self get_package_given($)} then
	       {self extract_makefile(_)}
	    else
	       {self makefile_read}
	    end
	 end
      end

      %% is this target included in src? we need to know this to avoid
      %% build src targets unless during a fullbuild

      meth target_is_src(T $)
	 {CondSelect @TargetIsSource {Path.toAtom T} false}
      end

      %% return the build rule for target T.  first look if the user specified
      %% one.  if not, use a default rule appropriate for the file extension

      meth get_rule(T $)
	 R={CondSelect @Target2Rule T unit}
      in
	 if R\=unit then R else {MakeRuleDefault T} end
      end

      %% return a list of all dependencies for target T
      %% 1. if the target is in src and we are not performing a `full' build
      %%    then return nil
      %% 2. otherwise return the union of the explicitly stated dependencies
      %%    and the implicit one induced by the rule for the target

      meth get_depends(T $)
	 {self makefile_read}
	 %% note that we assume that T is an atom
	 if Makefile,target_is_src(T $) andthen {Not {self get_fullbuild($)}} then nil
	 else
	    Table = {NewDictionary}
	    for D in {CondSelect @Target2Depends T nil} do Table.D := unit end
	    for D in {self get_autodepend_build(T $)}   do Table.D := unit end
	    R = Makefile,get_rule(T $)
	 in
	    if     R.tool==unit then skip
	    elseif R.tool==ozg  then
	       for D in {self get_depends(R.file $)} do Table.D := unit end
	    else Table.(R.file) := unit end
	    {Dictionary.keys Table}
	 end
      end

      %% turn the makefile back into a record for inclusion in a package

      meth makefile_to_record($ relax:RELAX<=false)
	 {self makefile_read}
	 MAK={NewDictionary}
	 Clean     = {self get_clean($)}
	 Veryclean = {self get_veryclean($)}
	 Author    = {self get_author($)}
	 Blurb     = {self get_blurb($)}
	 InfoText  = {self get_info_text($)}
	 InfoHtml  = {self get_info_html($)}
	 Requires  = {self get_requires($)}
	 Categories= {self get_categories($)}
	 Version   = {self get_version($)}
	 Contact   = {self get_contact($)}
	 Tar       = {self get_tar_targets($)}
	 Provides  = {self get_provides_targets($)}
	 Platform  = if {self get_binary($)} then
			{Property.get 'platform.name'}
		     else unit end
      in
	 MAK.bin     := {self get_bin_targets($)}
	 MAK.lib     := {self get_lib_targets($)}
	 MAK.doc     := {self get_doc_targets($)}
	 MAK.src     := if {self get_binary($)}
			then {self get_needed_locally($)}
			else {self get_src_targets($)} end
	 if Tar\=nil then MAK.tar := Tar end
	 MAK.depends := {Utils.toRecord @Target2Depends}
	 MAK.rules   := {Record.map {Utils.toRecord @Target2Rule}
			 fun {$ R} Tool=R.tool in
			    Tool(R.file R.options)
			 end}
	 if Provides\=unit then MAK.provides := Provides end
	 %% if there are no targets, the uri is unnecessary
	 if {self maybe_get_uri($)}\=unit
	    orelse MAK.bin\=nil
	    orelse MAK.lib\=nil
	    orelse MAK.doc\=nil
	 then
	    MAK.uri  := {self get_uri($)}
	 end
	 MAK.mogul   := if RELAX
			then {self get_mogul_relax($)}
			else {self get_mogul($)} end
	 if Clean    \=unit then MAK.clean     := Clean     end
	 if Veryclean\=unit then MAK.veryclean := Veryclean end
	 if Author   \=unit then MAK.author    := Author    end
	 if Blurb    \=unit then MAK.blurb     := Blurb     end
	 if InfoText \=unit then MAK.info_text := InfoText  end
	 if InfoHtml \=unit then MAK.info_html := InfoHtml  end
	 if Requires \=unit then MAK.requires  := Requires  end
	 if Categories\=nil then MAK.categories:= Categories end
	 if Version  \=unit then MAK.version   := Version   end
	 if Contact  \=unit then MAK.contact   := Contact   end
	 if Platform \=unit then MAK.platform  := Platform  end
	 MAK.released:= {Utils.dateCurrentToAtom}
	 %% grab also all the recursive makefiles
	 MAK.subdirs := {self get_subdirs($)}
	 MAK.submakefiles :=
	 {List.toRecord o
	  for
	     D in {self get_subdirs($)}
	     M in {self get_submans($)}
	     collect : Collect
	  do
	     {Collect D#{M makefile_to_record($)}}
	  end}
	 {Dictionary.toRecord makefile MAK}
      end

   end
end
