functor
export
   'class' : Attribs
import
   Property
   Path  at 'Path.ozf'
   Utils at 'Utils.ozf'
define
   class Attribs
      attr
	 Prefix     : unit
	 Dir        : unit
	 BuildDir   : unit
	 SrcDir     : unit
	 BinDir     : unit
	 LibDir     : unit
	 DocDir     : unit
	 LibRoot    : unit
	 DocRoot    : unit
	 MakeFile   : unit
	 MakeFileGiven : false
	 Uri        : unit
	 Mogul      : unit
	 ExtractDir : unit
	 Database   : unit
	 DatabaseGiven : false
	 Released   : unit
	 Clean      : unit
	 Veryclean  : unit
	 Author     : unit
	 BinTargets : nil
	 LibTargets : nil
	 DocTargets : nil
	 SrcTargets : nil
	 FullBuild  : false	% if true, also build targets in src
	 Blurb      : unit
	 InfoText   : unit
	 InfoHtml   : unit
	 Verbose    : false
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
	 PublishDir : unit
	 Archive    : 'http://www.mozart-oz.org/mogul/pkg'
	 LineWidth  : 70
	 NoMakefile : true

      meth set_prefix(D) Prefix<-{Path.expand D} end
      meth get_prefix($)
	 if @Prefix==unit then
	    Prefix<-{Path.expand
		     {Path.resolve
		      {Property.get 'user.home'} '.oz'}}
	 end
	 @Prefix
      end

      meth set_dir(D) Dir<-{Path.expand D} end

      meth set_builddir(D) BuildDir<-{Path.expand D} end
      meth get_builddir($)
	 if @BuildDir==unit then
	    if @Dir\=unit then
	       BuildDir<-@Dir
	    else
	       BuildDir<-nil
	    end
	 end
	 @BuildDir
      end

      meth set_srcdir(D) SrcDir<-{Path.expand D} end
      meth get_srcdir($)
	 if @SrcDir==unit then
	    if @Dir\=unit then
	       SrcDir<-@Dir
	    elseif @MakeFile\=unit then
	       SrcDir<-{Path.dirname @MakeFile}
	    else
	       SrcDir<-nil
	    end
	 end
	 @SrcDir
      end

      meth set_libroot(D) LibRoot<-{Path.expand D} end
      meth get_libroot($)
	 if @LibRoot==unit then
	    LibRoot<-{Path.resolve Attribs,get_prefix($) 'cache'}
	 end
	 @LibRoot
      end

      meth set_libdir(D) LibDir<-{Path.expand D} end
      meth get_libdir($)
	 if @LibDir==unit then
	    LibDir<-{Path.resolve Attribs,get_libroot($)
		     {Path.toCache Attribs,get_uri($)}}
	 end
	 @LibDir
      end

      meth set_bindir(D) BinDir<-{Path.expand D} end
      meth get_bindir($)
	 if @BinDir==unit then
	    BinDir<-{Path.resolve Attribs,get_prefix($) 'bin'}
	 end
	 @BinDir
      end

      meth set_docroot(D) DocRoot<-{Path.expand D} end
      meth get_docroot($)
	 if @DocRoot==unit then
	    DocRoot<-{Path.resolve Attribs,get_prefix($) 'doc'}
	 end
	 @DocRoot
      end

      meth set_docdir(D) DocDir<-{Path.expand D} end
      meth get_docdir($)
	 if @DocDir==unit then
	    DocDir<-{Path.resolve Attribs,get_docroot($)
		     {Utils.mogulToFilename Attribs,get_mogul($)}}
	 end
	 @DocDir
      end

      meth set_uri(U) Uri<-U end
      meth get_uri($)
	 if @Uri==unit then raise ozmake(get_uri) end
	 else @Uri end
      end

      meth set_mogul(M) Mogul<-M end
      meth get_mogul($)
	 if @Mogul==unit then raise ozmake(get_mogul) end
	 else @Mogul end
      end

      meth set_extractdir(D) ExtractDir<-{Path.expand D} end
      meth get_extractdir($)
	 if @ExtractDir==unit then
	    ExtractDir<-Attribs,get_builddir($)
	 end
	 @ExtractDir
      end

      meth set_makefile(F)
	 MakeFile<-{Path.expand F}
	 MakeFileGiven<-true
      end
      meth get_makefile($)
	 if @MakeFile==unit then
	    %% get_srcdir cannot call get_makefile but must look at
	    %% @MakeFile directly
	    MakeFile<-{Path.resolve Attribs,get_srcdir($) 'makefile.oz'}
	 end
	 @MakeFile
      end
      meth get_makefile_given($) @MakeFileGiven end

      meth set_database(F)
	 Database<-{Path.expand F}
	 DatabaseGiven<-true
      end
      meth get_database($)
	 if @Database==unit then
	    Database<-{Path.resolve Attribs,get_prefix($) 'DATABASE'}
	 end
	 @Database
      end
      meth get_database_given($) @DatabaseGiven end

      meth set_released(D) Released<-D end
      meth get_released($) @Released end

      meth set_clean(L) Clean<-L end
      meth get_clean($) @Clean end

      meth set_veryclean(L) Veryclean<-L end
      meth get_veryclean($) @Veryclean end

      meth set_author(L) Author<-L end
      meth get_author($) @Author end

      meth get_oz_home($) {Path.expand {Property.get 'oz.home'}} end
      meth get_oz_bindir($) {Path.resolve Attribs,get_oz_home($) 'bin'} end
      meth get_oz_engine($) {Path.resolveAtom Attribs,get_oz_bindir($) 'ozengine'} end
      meth get_oz_ozc($) {Path.resolveAtom Attribs,get_oz_bindir($) 'ozc'} end
      meth get_oz_ozl($) {Path.resolveAtom Attribs,get_oz_bindir($) 'ozl'} end
      meth get_oz_oztool($) {Path.resolveAtom Attribs,get_oz_bindir($) 'oztool'} end

      meth set_bin_targets(L) BinTargets<-L end
      meth get_bin_targets($) @BinTargets end
      meth set_lib_targets(L) LibTargets<-L end
      meth get_lib_targets($) @LibTargets end
      meth set_doc_targets(L) DocTargets<-L end
      meth get_doc_targets($) @DocTargets end
      meth set_src_targets(L) SrcTargets<-L end
      meth get_src_targets($) @SrcTargets end

      meth set_fullbuild(B) FullBuild<-B end
      meth get_fullbuild($) @FullBuild end

      meth set_blurb(S) Blurb<-S end
      meth get_blurb($) @Blurb end
      meth set_info_text(S) InfoText<-S end
      meth get_info_text($) @InfoText end
      meth set_info_html(S) InfoHtml<-S end
      meth get_info_html($) @InfoHtml end

      meth set_verbose(B) Verbose<-B end
      meth get_verbose($) @Verbose end
      meth set_quiet(B) Quiet<-B end
      meth get_quiet($) @Quiet end
      meth set_justprint(B) JustPrint<-B end
      meth get_justprint($) @JustPrint end
      meth set_optlevel(O) OptLevel<-O end
      meth get_optlevel($) @OptLevel end

      meth set_grade(G) Grade<-G end
      meth get_grade($) @Grade end

      meth set_replacefiles(B) ReplaceFiles<-B end
      meth get_replacefiles($) @ReplaceFiles end
      meth set_keepzombies(B) KeepZombies<-B end
      meth get_keepzombies($) @KeepZombies end
      meth set_savedb(B) SaveDB<-B end
      meth get_savedb($) @SaveDB end

      meth set_includedocs(B) IncludeDocs<-B end
      meth get_includedocs($) @IncludeDocs end
      meth set_includelibs(B) IncludeLibs<-B end
      meth get_includelibs($) @IncludeLibs end
      meth set_includebins(B) IncludeBins<-B end
      meth get_includebins($) @IncludeBins end

      meth set_extendpackage(B) ExtendPkg<-B end
      meth get_extendpackage($) @ExtendPkg end

      meth set_gnu(B) GNU<-B end
      meth get_gnu($)
	 if @GNU==unit then
	    GNU<-{self exec_check_for_gnu($)}
	 end
	 @GNU
      end

      meth set_package(F)
	 PackageGiven<-true
	 Package<-{Path.expand F}
      end
      meth get_package($)
	 if @Package==unit then
	    raise ozmake(get_package) end
	 else @Package end
      end
      meth get_package_given($) @PackageGiven end

      meth set_publishdir(D) PublishDir<-{Path.expand D} end
      meth get_publishdir($)
	 if @PublishDir==unit then
	    PublishDir<-{Path.expand
			 {Path.resolve {self get_prefix($)} 'pkg'}}
	 end
	 @PublishDir
      end

      meth set_archive(U) Archive<-U end
      meth get_archive($) @Archive end

      meth set_linewidth(N) LineWidth<-N end
      meth get_linewidth($) @LineWidth end

      meth set_no_makefile(B) NoMakefile<-B end
      meth get_no_makefile($) @NoMakefile end
   end
end