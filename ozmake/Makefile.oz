functor
export
   'class' : Makefile
import
   Path at 'Path.ozf'
   Utils at 'Utils.ozf'
prepare
   MakeByteString = ByteString.make

   %% a rule is represented by a record
   %% rule(tool:TOOL file:FILE options:[...])

   RULE_EXISTS = rule(tool:unit file:unit options:nil)

   VALID_MAKEFILE_FEATURES = [bin lib doc src depends rules uri mogul author released clean veryclean
			      blurb info_text info_html subdirs submakefiles requires topics version]

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
	       {self set_mogul({Path.toAtom U})}
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

	 if {HasFeature R author} then
	    if {IsVirtualString R.author} then
	       {self set_author([{MakeByteString R.author}])}
	    elseif {Not {IsList R.author}} orelse {Not {All R.author IsVirtualString}} then
	       raise ozmake(makefile:badauthor(R.author)) end
	    else
	       {self set_author({Map R.author MakeByteString})}
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
	 if {HasFeature R requires} then
	    if {IsVirtualString R.requires} then
	       {self set_requires([R.requires])}
	    elseif {IsList R.requires} andthen {All R.requires IsVirtualString} then
	       {self set_requires(R.requires)}
	    else
	       raise ozmake(makefile:badrequires(R.requires)) end
	    end
	 end

	 %% process topics feature
	 if {HasFeature R topics} then
	    if {IsVirtualString R.topics} then
	       {self set_topics([R.topics])}
	    elseif {IsList R.topics} andthen {All R.topics IsVirtualString} then
	       {self set_topics(R.topics)}
	    else
	       raise ozmake(makefile:badtopics(R.topics)) end
	    end
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
	 %% note that we assume that T is an atom
	 if Makefile,target_is_src(T $) andthen {Not {self get_fullbuild($)}} then nil
	 else
	    L = {CondSelect @Target2Depends T nil}
	    R = Makefile,get_rule(T $)
	 in
	    if R.tool==unit orelse {Member R.file L} then L
	    elseif R.tool==ozg then Table={NewDictionary} in
	       for D in L do Table.D := unit end
	       for D in {self get_depends(R.file $)} do Table.D := unit end
	       {Dictionary.keys Table}
	    else R.file|L end
	 end
      end

      %% turn the makefile back into a record for inclusion in a package

      meth makefile_to_record($)
	 MAK={NewDictionary}
	 Clean     = {self get_clean($)}
	 Veryclean = {self get_veryclean($)}
	 Author    = {self get_author($)}
	 Blurb     = {self get_blurb($)}
	 InfoText  = {self get_info_text($)}
	 InfoHtml  = {self get_info_html($)}
	 Requires  = {self get_requires($)}
	 Topics    = {self get_topics($)}
	 Version   = {self get_version($)}
      in
	 MAK.bin     := {self get_bin_targets($)}
	 MAK.lib     := {self get_lib_targets($)}
	 MAK.doc     := {self get_doc_targets($)}
	 MAK.src     := {self get_src_targets($)}
	 MAK.depends := {Dictionary.toRecord o @Target2Depends}
	 MAK.rules   := {Record.map {Dictionary.toRecord o @Target2Rule}
			 fun {$ R} Tool=R.tool in
			    Tool(R.file R.options)
			 end}
	 MAK.uri     := {self get_uri($)}
	 MAK.mogul   := {self get_mogul($)}
	 if Clean    \=unit then MAK.clean     := Clean     end
	 if Veryclean\=unit then MAK.veryclean := Veryclean end
	 if Author   \=unit then MAK.author    := Author    end
	 if Blurb    \=unit then MAK.blurb     := Blurb     end
	 if InfoText \=unit then MAK.info_text := InfoText  end
	 if InfoHtml \=unit then MAK.info_html := InfoHtml  end
	 if Requires \=unit then MAK.requires  := Requires  end
	 if Topics   \=nil  then MAK.topics    := Topics    end
	 if Version  \=unit then MAK.version   := Version   end
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
