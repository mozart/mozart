functor
export
   'class' : Attribs
import
   Property
   Resolve
   OS
   Path  at 'Path.ozf'
   Utils at 'Utils.ozf'
   URL
prepare
   DB_OZMAKE = 'apps/ozmake/ozmake.db'
   DB_MOGUL  = 'apps/ozmake/mogul.db'
   DB_CONFIG = 'apps/ozmake/config.db'
   DB_CONFIG_OLDSTYLE = 'apps/ozmake/DATABASE'
define
   class Attribs
      attr
	 %% the format is pickle-version dependent only
	 %% for earlier versions (i.e. ones without an
	 %% an explicit pickle format)
	 Format     : '1.3.0'
	 Prefix     : unit
	 Dir        : unit
	 BuildDir   : unit
	 SrcDir     : unit
	 BinDir     : unit
	 LibDir     : unit
	 DocDir     : unit
	 LibRoot    : unit
	 DocRoot    : unit
	 TmpDir     : unit
	 UseMakePkg : unit
	 MakeFile   : unit
	 MakeFileGiven : false
	 Uri        : unit
	 Mogul      : unit
	 ExtractDir : unit
	 Database   : unit
	 DatabaseGiven : false
	 DatabaseIgnore : false
	 Released   : unit
	 Clean      : unit
	 Veryclean  : unit
	 Author     : unit
	 BinTargets : nil
	 LibTargets : nil
	 DocTargets : nil
	 SrcTargets : nil
	 ProvidesTargets : unit
	 FullBuild  : false	% if true, also build targets in src
	 Blurb      : unit
	 InfoText   : unit
	 InfoHtml   : unit
	 Requires   : unit
	 Version    : unit
	 Verbose    : false
	 VeryVerbose: false
	 Quiet      : false
	 JustPrint  : false
	 OptLevel   : optimize
	 Grade      : none
	 ReplaceFiles : false
	 KeepZombies: false
	 SaveDB     : true
	 IncludeDocs: true
	 IncludeLibs: true
	 IncludeBins: true
	 ExtendPkg  : false
	 GNU        : unit
	 Package    : unit
	 PackageGiven:false
	 Archive    : 'http://www.mozart-oz.org/mogul/pkg'
	 LineWidth  : 70
	 NoMakefile : true
	 Local      : false
	 %% support for recursing into subdirs
	 Subdirs    : nil	% list of subdirs
	 AsSubdir   : unit	% name of current directory
	 Submans    : unit	% managers for subdirectories
	 Superman   : unit	% manager for parent directory
	 Submakefiles:unit
	 IncludeDirs: nil
	 LibraryDirs: nil
	 SysIncludeDirsOK:true
	 SysLibraryDirsOK:true
	 SysIncludeDirs:unit
	 SysLibraryDirs:unit
	 %% tools
	 OzEngine   : unit
	 OzC        : unit
	 OzL        : unit
	 OzTool     : unit
	 ResolverExtended : false
	 SubResolverStack : nil
	 FromPackage: false
	 XMogul     : unit
	 MogulDatabase : unit
	 MogulDatabaseGiven : false
	 MogulPkgURL : unit
	 MogulDocURL : unit
	 MogulDBURL  : unit
	 ConfigFile  : unit
	 MogulPkgDir : unit
	 MogulDocDir : unit
	 MogulDBDir  : unit
	 Categories      : nil
	 Exe         : default
	 Makepkgfile : unit
	 MustExtract : true
	 MogulAction : unit
	 Contact     : unit
	 MogulRootID : unit
	 MogulDir    : unit
	 MogulUrl    : unit
	 ConfigAction: unit
	 TarTargets  : nil
	 Fast        : true
	 WantVersion : unit
	 AutoDepend  : true
	 Args        : unit
	 Optlist     : unit
	 DoRequires  : true
	 Binary      : false

      meth set_binary(V) Binary<-V end
      meth get_binary($) @Binary end

      meth set_args(V) Args<-V end
      meth get_args($) @Args end

      meth set_optlist(V) Optlist<-V end
      meth get_optlist($) @Optlist end

      meth set_dorequires(V) DoRequires<-V end
      meth get_dorequires($) @DoRequires end

      meth get_format($) @Format end

      meth set_prefix(D) Prefix<-{Path.expand D} end
      meth get_prefix($)
	 if @Prefix==unit then
	    if @Superman\=unit then
	       Prefix<-{@Superman get_prefix($)}
	    else DOTOZ={Property.condGet 'oz.dotoz' unit} in
	       if DOTOZ==unit then
		  Prefix<-{Path.expand
			   {Path.resolve
			    {Property.get 'user.home'} '.oz'}}
	       else
		  Prefix<-{Path.expand DOTOZ}
	       end
	    end
	 end
	 @Prefix
      end

      meth set_dir(D) Dir<-{Path.expandInCWD D} end

      meth set_tmpdir(D) TmpDir<-{Path.expandInCWD D} end
      meth get_tmpdir($) @TmpDir end
      meth get_tmpnam($) F={OS.tmpnam} in
	 if @TmpDir==unit then F else U={Path.toURL F} in
	    {Path.resolve @TmpDir {Path.toString {AdjoinAt U absolute false}}}
	 end
      end

      meth set_builddir(D) BuildDir<-{Path.expandInCWD D} end
      meth get_builddir($)
	 if @BuildDir==unit then
	    if @Superman\=unit then
	       BuildDir<-{Path.resolve {@Superman get_builddir($)} @AsSubdir}
	    elseif @Dir\=unit then
	       BuildDir<-@Dir
	    else
	       BuildDir<-{Path.expandInCWD nil}
	    end
	 end
	 @BuildDir
      end

      meth set_srcdir(D) SrcDir<-{Path.expandInCWD D} end
      meth get_srcdir($)
	 if @SrcDir==unit then
	    if @Superman\=unit then
	       SrcDir<-{Path.resolve {@Superman get_srcdir($)} @AsSubdir}
	    elseif @Dir\=unit then
	       SrcDir<-@Dir
	    elseif @MakeFile\=unit then
	       SrcDir<-{Path.dirname @MakeFile}
	    else
	       SrcDir<-{Path.expandInCWD nil}
	    end
	 end
	 @SrcDir
      end

      meth set_libroot(D) LibRoot<-{Path.expand D} end
      meth get_libroot($)
	 if @LibRoot==unit then
	    if @Superman\=unit then
	       LibRoot<-{@Superman get_libroot($)}
	    else
	       LibRoot<-{Path.resolve Attribs,get_prefix($) 'cache'}
	    end
	 end
	 @LibRoot
      end

      meth set_libdir(D) LibDir<-{Path.expand D} end
      meth get_libdir($)
	 if @LibDir==unit then
	    if @Uri\=unit then
	       LibDir<-{Path.resolve Attribs,get_libroot($)
			{Path.toCache Attribs,get_uri($)}}
	    elseif @Superman\=unit then
	       LibDir<-{Path.resolve {@Superman get_libdir($)} @AsSubdir}
	    else
	       LibDir<-{Path.resolve Attribs,get_libroot($)
			{Path.toCache Attribs,get_uri($)}}
	    end
	 end
	 @LibDir
      end

      meth set_bindir(D) BinDir<-{Path.expand D} end
      meth get_bindir($)
	 if @BinDir==unit then
	    if @Superman\=unit then
	       BinDir<-{@Superman get_bindir($)}
	    else
	       BinDir<-{Path.resolve Attribs,get_prefix($) 'bin'}
	    end
	 end
	 @BinDir
      end

      meth set_docroot(D) DocRoot<-{Path.expand D} end
      meth get_docroot($)
	 if @DocRoot==unit then
	    if @Superman\=unit then
	       DocRoot<-{@Superman get_docroot($)}
	    else
	       DocRoot<-{Path.resolve Attribs,get_prefix($) 'doc'}
	    end
	 end
	 @DocRoot
      end

      meth set_docdir(D) DocDir<-{Path.expand D} end
      meth get_docdir($)
	 if @DocDir==unit then
	    if @Superman\=unit then
	       DocDir<-{Path.resolve {@Superman get_docdir($)} @AsSubdir}
	    else
	       DocDir<-{Path.resolve Attribs,get_docroot($)
			{Utils.mogulToFilename Attribs,get_mogul($)}}
	    end
	 end
	 @DocDir
      end

      meth set_uri(U) Uri<-U end
      meth get_uri($)
	 if @Uri==unit andthen @Superman\=unit then
	    Uri<-{Path.resolveAtom {@Superman get_uri($)} @AsSubdir}
	 end
	 if @Uri==unit then raise ozmake(get_uri) end else @Uri end
      end
      meth maybe_get_uri($)
	 if @Uri==unit andthen @Superman\=unit then
	    U = {@Superman maybe_get_uri($)}
	 in
	    if U\=unit then Uri<-{Path.resolveAtom U @AsSubdir} end
	 end
	 @Uri
      end

      meth set_mogul(M) Mogul<-M end
      meth get_mogul($)
	 if @Mogul==unit andthen @Superman\=unit then
	    Mogul<-{@Superman get_mogul($)}
	 end
	 if @Mogul==unit then raise ozmake(get_mogul) end
	 else @Mogul end
      end
      meth get_mogul_relax($)
	 try {self get_mogul($)}
	 catch E then {Value.byNeedFail E} end
      end

      meth set_xmogul(ID) XMogul<-ID end
      meth get_xmogul($) @XMogul end

      meth set_extractdir(D) ExtractDir<-{Path.expand D} end
      meth get_extractdir($)
	 if @ExtractDir==unit then
	    if @BuildDir==unit andthen @Dir==unit andthen @XMogul\=unit then
	       D={Utils.mogulToFilename @XMogul}
	    in
	       if {Path.exists D} then
		  raise ozmake(extract:dir(D)) end
	       end
	       ExtractDir<-D
	    else
	       ExtractDir<-Attribs,get_builddir($)
	    end
	 end
	 @ExtractDir
      end

      meth get_package_or_guess($)
	 if {self get_package_given($)} then
	    {self get_package($)}
	 else
	    MOG = {self get_mogul($)}
	    VER = {self get_version($)}
	    PKG = {Path.resolveAtom
		   {self get_builddir($)}
		   {Utils.mogulToFilename MOG}#if VER==unit then nil else '-'#VER end#'.pkg'}
	 in
	    {self trace('package argument not given, using: '#PKG)}
	    PKG
	 end
      end

      meth set_default_use_makepkg(B)
	 if @UseMakePkg==unit then
	    UseMakePkg<-(B==true)
	 end
      end
      meth set_use_makepkg(B)
	 UseMakePkg<-(B==true)
      end

      meth set_makefile(F)
	 MakeFile<-{Path.expandInCWD F}
	 MakeFileGiven<-true
      end
      meth get_makefile($)
	 if @MakeFile==unit andthen @Superman\=unit then
	    M={@Superman get_makefile($)}
	    D={Path.dirname M}
	    B={Path.basename M}
	 in
	    MakeFile<-{Path.resolve {Path.resolve D @AsSubdir} B}
	 end
	 if @MakeFile==unit andthen @UseMakePkg\=false then
	    %% get_srcdir cannot call get_makefile but must look at
	    %% @MakeFile directly
	    F = {Path.resolve Attribs,get_srcdir($) 'MAKEPKG.oz'}
	 in
	    if {Path.exists F} then MakeFile<-F end
	 end
	 if @MakeFile==unit then
	    MakeFile<-{Path.resolve Attribs,get_srcdir($) 'makefile.oz'}
	 end
	 @MakeFile
      end
      meth get_makefile_given($)
	 if @Superman\=unit then
	    {@Superman get_makefile_given($)}
	 else @MakeFileGiven end
      end

      meth set_contact(L) Contact<-L end
      meth get_contact($) @Contact end

      meth set_database(F)
	 Database<-{Path.expand F}
	 DatabaseGiven<-true
      end
      meth get_database($)
	 if @Database==unit then
	    if @Superman\=unit then
	       Database<-{@Superman get_database($)}
	    else
	       Database<-{Path.resolve Attribs,get_prefix($) DB_OZMAKE}
	    end
	 end
	 @Database
      end
      meth get_database_given($)
	 if @Superman\=unit then
	    {@Superman get_database_given($)}
	 else @DatabaseGiven end
      end

      meth get_database_oldstyle($)
	 {Path.resolve Attribs,get_prefix($) 'DATABASE'}
      end

      meth set_released(D) Released<-D end
      meth get_released($)
	 if @Released==unit andthen @Superman\=unit then
	    Released<-{@Superman get_released($)}
	 end
	 @Released
      end

      meth set_clean(L) Clean<-L end
      meth get_clean($)
	 if @Clean==unit andthen @Superman\=unit then
	    Clean<-{@Superman get_clean($)}
	 end
	 @Clean
      end

      meth set_veryclean(L) Veryclean<-L end
      meth get_veryclean($)
	 if @Veryclean==unit andthen @Superman\=unit then
	    Veryclean<-{@Superman get_veryclean($)}
	 end
	 @Veryclean
      end

      meth set_author(L) Author<-L end
      meth get_author($)
	 if @Author==unit andthen @Superman\=unit then
	    Author<-{@Superman get_author($)}
	 end
	 @Author
      end

      meth get_oz_home($) {Path.expand {Property.get 'oz.home'}} end
      meth get_oz_bindir($) {Path.resolve Attribs,get_oz_home($) 'bin'} end
      meth get_oz_engine($)
	 if @OzEngine==unit then
	    P={Path.resolveAtom Attribs,get_oz_bindir($) 'ozengine.exe'}
	 in
	    if {Path.exists P} then OzEngine<-P else
	       OzEngine<-{Path.resolveAtom Attribs,get_oz_bindir($) 'ozengine'}
	    end
	 end
	 @OzEngine
      end
      meth get_oz_ozc($)
	 if @OzC==unit then
	    P={Path.resolveAtom Attribs,get_oz_bindir($) 'ozc.exe'}
	 in
	    if {Path.exists P} then OzC<-P else
	       OzC<-{Path.resolveAtom Attribs,get_oz_bindir($) 'ozc'}
	    end
	 end
	 @OzC
      end
      meth get_oz_ozl($)
	 if @OzL==unit then
	    P={Path.resolveAtom Attribs,get_oz_bindir($) 'ozl.exe'}
	 in
	    if {Path.exists P} then OzL<-P else
	       OzL<-{Path.resolveAtom Attribs,get_oz_bindir($) 'ozl'}
	    end
	 end
	 @OzL
      end
      meth get_oz_oztool($)
	 if @OzTool==unit then
	    P={Path.resolveAtom Attribs,get_oz_bindir($) 'oztool.exe'}
	 in
	    if {Path.exists P} then OzTool<-P else
	       OzTool<-{Path.resolveAtom Attribs,get_oz_bindir($) 'oztool'}
	    end
	 end
	 @OzTool
      end

      meth set_bin_targets(L) BinTargets<-L end
      meth get_bin_targets($) @BinTargets end
      meth set_lib_targets(L) LibTargets<-L end
      meth get_lib_targets($) @LibTargets end
      meth set_doc_targets(L) DocTargets<-L end
      meth get_doc_targets($) @DocTargets end
      meth set_src_targets(L) SrcTargets<-L end
      meth get_src_targets($) @SrcTargets end

      meth set_fullbuild(B) FullBuild<-B end
      meth get_fullbuild($)
	 if @Superman\=unit
	 then {@Superman get_fullbuild($)}
	 else @FullBuild end
      end

      meth set_blurb(S) Blurb<-S end
      meth get_blurb($) @Blurb end
      meth set_info_text(S) InfoText<-S end
      meth get_info_text($) @InfoText end
      meth set_info_html(S) InfoHtml<-S end
      meth get_info_html($) @InfoHtml end

      meth get_requires($) @Requires end
      meth set_requires(L) Requires<-L end

      meth set_verbose(L)
	 case {Reverse L}
	 of true|true|_ then VeryVerbose<-true Verbose<-true
	 [] true|_ then Verbose<-true
	 else Verbose<-false end
      end
      meth get_verbose($)
	 if @Superman\=unit
	 then {@Superman get_verbose($)}
	 else @Verbose end
      end
      meth get_veryVerbose($)
	 if @Superman\=unit
	 then {@Superman get_veryVerbose($)}
	 else @VeryVerbose end
      end
      meth set_quiet(B) Quiet<-B end
      meth get_quiet($)
	 if @Superman\=unit
	 then {@Superman get_quiet($)}
	 else @Quiet end
      end
      meth set_justprint(B) JustPrint<-B end
      meth get_justprint($)
	 if @Superman\=unit
	 then {@Superman get_justprint($)}
	 else @JustPrint end
      end
      meth set_optlevel(O) OptLevel<-O end
      meth get_optlevel($)
	 if @Superman\=unit
	 then {@Superman get_optlevel($)}
	 else @OptLevel end
      end

      meth set_grade(G)
	 Grade<-G
	 if G==freshen then
	    {self set_must_extract(false)}
	 end
      end
      meth get_grade($)
	 if @Superman\=unit
	 then {@Superman get_grade($)}
	 else @Grade end
      end

      meth set_replacefiles(B) ReplaceFiles<-B end
      meth get_replacefiles($)
	 if @Superman\=unit
	 then {@Superman get_replacefiles($)}
	 else @ReplaceFiles end
      end
      meth set_keepzombies(B) KeepZombies<-B end
      meth get_keepzombies($)
	 if @Superman\=unit
	 then {@Superman get_keepzombies($)}
	 else @KeepZombies end
      end
      meth set_savedb(B) SaveDB<-B end
      meth get_savedb($)
	 if @Superman\=unit
	 then {@Superman get_savedb($)}
	 else @SaveDB end
      end

      meth set_includedocs(B) IncludeDocs<-B end
      meth get_includedocs($)
	 if @Superman\=unit
	 then {@Superman get_includedocs($)}
	 else @IncludeDocs end
      end
      meth set_includelibs(B) IncludeLibs<-B end
      meth get_includelibs($)
	 if @Superman\=unit
	 then {@Superman get_includelibs($)}
	 else @IncludeLibs end
      end
      meth set_includebins(B) IncludeBins<-B end
      meth get_includebins($)
	 if @Superman\=unit
	 then {@Superman get_includebins($)}
	 else @IncludeBins end
      end

      meth set_extendpackage(B) ExtendPkg<-B end
      meth get_extendpackage($)
	 if @Superman\=unit
	 then {@Superman get_extendpackage($)}
	 else @ExtendPkg end
      end

      meth set_gnu(B) GNU<-B end
      meth get_gnu($)
	 if @GNU==unit then
	    if @Superman\=unit
	    then GNU<-{@Superman get_gnu($)}
	    else GNU<-{self exec_check_for_gnu($)} end
	 end
	 @GNU
      end

      meth set_package(F)
	 PackageGiven<-true
	 Package<-{Path.expand F}
      end
      meth get_package($)
	 if @Package==unit andthen @Superman\=unit then
	    Package<-{@Superman get_package($)}
	 end
	 if @Package==unit then
	    raise ozmake(get_package) end
	 else @Package end
      end
      meth get_package_given($)
	 if @Superman\=unit
	 then {@Superman get_package_given($)}
	 else @PackageGiven end
      end

      meth set_archive(U) Archive<-U end
      meth get_archive($)
	 if @Superman\=unit
	 then {@Superman get_archive($)}
	 else @Archive end
      end

      meth set_linewidth(N) LineWidth<-N end
      meth get_linewidth($)
	 if @Superman\=unit
	 then {@Superman get_linewidth($)}
	 else @LineWidth end
      end

      meth set_no_makefile(B) NoMakefile<-B end
      meth get_no_makefile($)
	 if @Superman\=unit
	 then {@Superman get_no_makefile($)}
	 else @NoMakefile end
      end

      meth set_subdirs(L) Subdirs<-L end
      meth get_subdirs($) @Subdirs end

      meth set_assubdir(F) AsSubdir<-F end
      meth get_assubdir($) @AsSubdir end
      meth set_superman(M) Superman<-M end
      meth get_superman($) @Superman end

      meth get_submans($)
	 if @Submans==unit then
	    Submans<-{self subdirs_to_managers({self get_subdirs($)} $)}
	 end
	 @Submans
      end

      meth set_local(B) Local<-B end
      meth get_local($)
	 if @Superman\=unit
	 then {@Superman get_local($)}
	 else @Local end
      end

      meth set_submakefiles(R) Submakefiles<-R end
      meth get_submakefiles($) @Submakefiles end
      meth has_submakefile(D $)
	 {HasFeature @Submakefiles {Path.toAtom D}}
      end
      meth get_submakefile(D $)
	 @Submakefiles.{Path.toAtom D}
      end

      meth set_includedirs(L) IncludeDirs<-L end
      meth get_includedirs($) @IncludeDirs end
      meth set_librarydirs(L) LibraryDirs<-L end
      meth get_librarydirs($) @LibraryDirs end

      %% `system' directories to pass to oztool
      meth set_sysincludedirsok(B) SysIncludeDirsOK<-(B==true) end
      meth set_syslibrarydirsok(B) SysLibraryDirsOK<-(B==true) end
      meth get_sysincludedirsok($)
	 if @Superman\=unit
	 then {@Superman get_sysincludedirsok($)}
	 else @SysIncludeDirsOK end
      end
      meth get_syslibrarydirsok($)
	 if @Superman\=unit
	 then {@Superman get_syslibrarydirsok($)}
	 else @SysLibraryDirsOK end
      end
      meth get_sysincludedirs($)
	 if @SysIncludeDirs==unit then
	    if {self get_sysincludedirsok($)} then
	       if @Superman\=unit then
		  SysIncludeDirs<-{@Superman get_sysincludedirs($)}
	       else
		  SysIncludeDirs<-
		  [{Path.expand
		    {Path.resolve
		     {self get_prefix($)}
		     'platform/'#{Property.get 'platform.name'}#'/include'}}
		   {Path.expand
		    {Path.resolve {self get_prefix($)} 'include'}}
		   {Path.expand
		    {Path.resolve
		     {Property.get 'oz.home'}
		     'platform/'#{Property.get 'platform.name'}#'/include'}}
		   {Path.expand
		    {Path.resolve {Property.get 'oz.home'} 'include'}}]
	       end
	    else
	       SysIncludeDirs<-nil
	    end
	 end
	 @SysIncludeDirs
      end
      meth get_syslibrarydirs($)
	 if @SysLibraryDirs==unit then
	    if {self get_syslibrarydirsok($)} then
	       if @Superman\=unit then
		  SysLibraryDirs<-{@Superman get_syslibrarydirs($)}
	       else
		  SysLibraryDirs<-
		  [{Path.expand
		    {Path.resolve
		     {self get_prefix($)}
		     'platform/'#{Property.get 'platform.name'}#'/lib'}}
		   {Path.expand
		    {Path.resolve
		     {Property.get 'oz.home'}
		     'platform/'#{Property.get 'platform.name'}#'/lib'}}]
	       end
	    else
	       SysLibraryDirs<-nil
	    end
	 end
	 @SysLibraryDirs
      end

      meth extend_resolver
	 if @ResolverExtended then skip else
	    ResolverExtended<-true
	    if @Superman \= unit then
	       {@Superman extend_resolver}
	    else
	       SRC={self get_srcdir($)}
	       BLD={self get_builddir($)}
	       SEP=[{Property.get 'path.separator'}]
	       OZLOAD={Property.get 'oz.search.load'}
	    in
	       {OS.putEnv 'OZ_SEARCH_LOAD'
		OZLOAD                  #SEP#
		'root='#SRC             #SEP#
		'root='#BLD}
	       {Resolve.addHandler
		{Resolve.handler.root SRC}}
	       {Resolve.addHandler
		{Resolve.handler.root BLD}}
	    end
	 end
      end

      meth subresolver_push(DST SRC)
	 PATH = {OS.getEnv 'OZ_SEARCH_LOAD'}
	 SEP  = [{Property.get 'path.separator'}]
	 DST_DIR = {Path.dirname DST}
	 SRC_DIR = {Path.dirname SRC}
	 DST_ENV = {OS.getEnv 'OZMAKE_BUILD_DIR'}
	 SRC_ENV = {OS.getEnv 'OZMAKE_SOURCE_DIR'}
      in
	 SubResolverStack <- (PATH#{Resolve.getHandlers}#DST_ENV#SRC_ENV)|@SubResolverStack
	 {OS.putEnv 'OZ_SEARCH_LOAD'
	  DST_DIR#SEP#
	  SRC_DIR#SEP#PATH}
	 {Resolve.addHandler {Resolve.handler.root DST_DIR}}
	 {Resolve.addHandler {Resolve.handler.root SRC_DIR}}
	 {OS.putEnv 'OZMAKE_BUILD_DIR'  DST_DIR}
	 {OS.putEnv 'OZMAKE_SOURCE_DIR' SRC_DIR}
      end

      meth subresolver_pop()
	 case @SubResolverStack of (PATH#Handlers#DST_ENV#SRC_ENV)|L then
	    SubResolverStack<-L
	    {OS.putEnv 'OZ_SEARCH_LOAD' PATH}
	    {Resolve.setHandlers Handlers}
	    {OS.putEnv 'OZMAKE_BUILD_DIR'  if DST_ENV==false then nil else DST_ENV end}
	    {OS.putEnv 'OZMAKE_SOURCE_DIR' if SRC_ENV==false then nil else SRC_ENV end}
	 end
      end

      meth set_fromPackage(B) FromPackage<-B end
      meth get_fromPackage($)
	 if @Superman\=unit then
	    {@Superman get_fromPackage($)}
	 else
	    @FromPackage
	 end
      end

      meth set_moguldatabase(F)
	 MogulDatabase<-{Path.expand F}
	 MogulDatabaseGiven<-true
      end
      meth get_moguldatabase($)
	 if @MogulDatabase==unit then
	    if @Superman\=unit then
	       MogulDatabase<-{@Superman get_moguldatabase($)}
	    else
	       MogulDatabase<-{Path.resolve Attribs,get_prefix($) DB_MOGUL}
	    end
	 end
	 @MogulDatabase
      end
      meth get_moguldatabase_given($) @MogulDatabaseGiven end

      meth set_configfile(F)
	 ConfigFile <- {Path.resolve Attribs,get_prefix($) F}
      end
      meth get_configfile($)
	 if @ConfigFile==unit then
	    {self set_configfile(DB_CONFIG)}
	 end
	 @ConfigFile
      end
      meth get_configfile_oldstyle($)
	 {Path.resolve Attribs,get_prefix($) DB_CONFIG_OLDSTYLE}
      end

      meth set_mogulurl(U)
	 MogulUrl <- {URL.toAtom {URL.toBase U}}
      end
      meth set_mogulpkgurl(U)
	 MogulPkgURL <- {URL.toAtom {URL.toBase U}}
      end
      meth set_moguldocurl(U)
	 MogulDocURL <- {URL.toAtom {URL.toBase U}}
      end
      meth set_moguldburl(U)
	 MogulDBURL <- {URL.toAtom {URL.toBase U}}
      end

      meth get_mogulpkgurl($)
	 if @MogulPkgURL==unit then
	    if @MogulUrl==unit then
	       raise ozmake(mogul:nopkgurl) end
	    else
	       MogulPkgURL <- {Path.resolveAtom @MogulUrl 'pkg'}
	    end
	 end
	 @MogulPkgURL
      end
      meth get_moguldocurl($)
	 if @MogulDocURL==unit then
	    if @MogulUrl==unit then
	       raise ozmake(mogul:nodocurl) end
	    else
	       MogulDocURL <- {Path.resolveAtom @MogulUrl 'doc'}
	    end
	 end
	 @MogulDocURL
      end
      meth get_moguldburl($)
	 if @MogulDBURL==unit then
	    if @MogulUrl==unit then
	       raise ozmake(mogul:nodburl) end
	    else
	       MogulDBURL <- {Path.resolveAtom @MogulUrl 'db'}
	    end
	 end
	 @MogulDBURL
      end

      meth set_moguldir(D) MogulDir<-{Path.expand D} end
      meth set_mogulpkgdir(D) MogulPkgDir<-{Path.expand D} end
      meth set_moguldocdir(D) MogulDocDir<-{Path.expand D} end
      meth set_moguldbdir(D) MogulDBDir<-{Path.expand D} end

      meth get_mogulpkgdir($)
	 if @MogulPkgDir==unit then
	    if @MogulDir==unit then
	       raise ozmake(mogul:nomogulpkgdir) end
	    else
	       MogulPkgDir<-{Path.expand {Path.resolve @MogulDir 'pkg'}}
	    end
	 end
	 @MogulPkgDir
      end
      meth get_moguldocdir($)
	 if @MogulDocDir==unit then
	    if @MogulDir==unit then
	       raise ozmake(mogul:nomoguldocdir) end
	    else
	       MogulDocDir<-{Path.expand {Path.resolve @MogulDir 'doc'}}
	    end
	 end
	 @MogulDocDir
      end
      meth get_moguldbdir($)
	 if @MogulDBDir==unit then
	    if @MogulDir==unit then
	       raise ozmake(mogul:nomoguldbdir) end
	    else
	       MogulDBDir<-{Path.expand {Path.resolve @MogulDir 'db'}}
	    end
	 end
	 @MogulDBDir
      end

      meth set_categories(L) Categories<-L end
      meth get_categories($) @Categories end

      meth set_version(V) Version<-V end
      meth get_version($) @Version end

      meth set_exe(A) Exe<-A end
      meth get_exe($) @Exe end

      meth set_makepkgfile(F)
	 if {Path.dirname F}\=nil then
	    raise ozmake(makepkgfile({Path.toString F})) end
	 else
	    Makepkgfile<-F
	 end
      end
      meth get_makepkgfile($) @Makepkgfile end

      meth set_must_extract(B) MustExtract<-B end
      meth get_must_extract($) @MustExtract end

      meth set_mogul_action(A) MogulAction<-{self mogul_validate_action(A $)} end
      meth get_mogul_action($) @MogulAction end

      meth set_mogulrootid(ID)
	 if {Utils.isMogulID ID} then
	    MogulRootID <- ID
	 else
	    raise ozmake(mogul:badrootid(ID)) end
	 end
      end
      meth get_mogulrootid($) @MogulRootID end

      meth set_config_action(A) ConfigAction<-{self config_validate_action(A $)} end
      meth get_config_action($) @ConfigAction end

      meth set_database_ignore(B) DatabaseIgnore<-B end
      meth get_database_ignore($) @DatabaseIgnore end

      meth set_tar_targets(L) TarTargets<-L end
      meth get_tar_targets($) @TarTargets end

      meth set_provides_targets(L) ProvidesTargets<-L end
      meth get_provides_targets($) @ProvidesTargets end

      meth set_fast(B) Fast<-B end
      meth get_fast($) @Fast end

      meth set_want_version(S)
	 if {Utils.isVersion S} then
	    WantVersion<-S
	 else
	    raise ozmake(badwantversion(S)) end
	 end
      end
      meth get_want_version($) @WantVersion end

      meth set_autodepend(B) AutoDepend<-B end
      meth get_autodepend($) @AutoDepend end
   end
end
