functor
import
   Application(exit)
   System(showError printError show)
   OS(getCWD getEnv)
   Except('raise':Raise)
   Database('class')
   Open(file)
   URL(toAtom resolve toBase toString make)
   Category(updateCatPages updatePkgListPage)
   MogulID(normalizeID:NormalizeID)
   HTML_ByAuthor(updatePage)
   Pickle(saveCompressed)
   Regex(make compile search) at 'x-oz://contrib/regex'
   Directory(mkDir)
export
   Manager Trace Indent Dedent RelativeTo Admin
define
   fun {RelativeTo Base Rel}
      {URL.toString {URL.resolve {URL.toBase Base} Rel}}
   end
   %%
   class Admin
      prop final
      attr db rootID rootURL reports verbose nerrors
	 indent wget css mogulDIR mogulURL provided
	 categories categoriesURL packages Authors mogulTOP
	 ignoreID ignoreURL manifest
      meth init
	 db      <- unit
	 rootID  <- {NormalizeID 'mogul' 'mogul:/'}
	 rootURL <- 'mogul.mogul'
	 reports <- nil
	 verbose <- false
	 nerrors <- 0
	 indent  <- ''
	 wget    <- 'wget'
	 css     <- 'mogul.css'
	 mogulDIR<- {OS.getCWD}
	 mogulURL<- ''
	 provided<- unit
	 categories<-unit
	 categoriesURL<-'mogul-categories.mogul'
	 packages<-unit
	 Authors<-unit
	 mogulTOP<- '/~'#{OS.getEnv 'USER'}#'/mogul'
	 ignoreID<-nil
	 ignoreURL<-nil
	 manifest<-{NewDictionary}
      end
      meth indent indent<-'  '#@indent end
      meth dedent
	 case @indent of _#V then indent<-V end
      end
      meth trace(Msg)
	 if @verbose then {System.showError @indent#Msg} end
      end
      meth incTrace(Msg)
	 {self trace(Msg)}
	 {self indent}
      end
      meth decTrace(Msg)
	 {self dedent}
	 {self trace(Msg)}
      end
      meth addReport(Msg Exc)
	 %% !!! later, the pair should be replaced by a Report object
	 reports <- (Msg#Exc)|@reports
	 nerrors <- @nerrors+1
      end
      meth exit
	 if @nerrors>0 then
	    {self trace('THERE WERE ERRORS')}
	 end
	 {self trace('THE END')}
	 {Application.exit if @nerrors==0 then 0 else 1 end}
      end
      %%
      meth 'verbose'(V) verbose<-V end
      meth 'quiet'(V) verbose<-{Not V} end
      meth 'root-id'(V) rootID<-{NormalizeID V 'mogul:/'} end
      meth 'root-url'(V) rootURL<-V end
      meth 'open-db'(V)
	 if @db\=unit then
	    {Raise mogul('open-db'(a_db_is_already_opened))}
	 end
	 db <- {New Database.'class' init(V)}
      end
      meth 'save-db'(V)
	 if V then
	    if @db==unit then
	       {Raise mogul('save-db'(no_db_is_opened))}
	    end
	    {@db save_db}
	 end
      end
      meth 'close-db'(V)
	 if V then
	    if @db==unit then
	       {Raise mogul('close-db'(no_db_is_opened))}
	    end
	    {@db close}
	    db <- unit
	 end
      end
      meth 'export-db'(V)
	 if @db==unit then
	    {Raise mogul('export-db'(no_db_is_opened))}
	 else
	    {@db export_db(V)}
	 end
      end
      meth 'import-db'(V)
	 {@db import_db(V)}
      end
      meth 'update-info'(V)
	 if V then
	    if @db==unit then
	       {Raise mogul('update-db'(no_db_is_opened))}
	    end
	    {@db updateInfo(@rootID {RelativeTo @mogulURL @rootURL} unit)}
	 end
      end
      meth 'print-db'(V)
	 if V then
	    if @db==unit then
	       {Raise mogul('print-db'(no_db_is_opened))}
	    end
	    local Out = {New Open.file init(name:stdout flags:[write])} in
	       {@db printOut(@rootID '+' Out)}
	    end
	 end
      end
      meth 'print-reports'(V)
	 {System.showError 'ERROR REPORTS'}
	 if V then
	    for R in {Reverse @reports} do
	       {System.showError {Value.toVirtualString R 100 100}}
	    end
	 end
      end
      meth get_docdir($)
	 {RelativeTo @mogulDIR 'doc'}
      end
      meth get_pkgdir($)
	 {RelativeTo @mogulDIR 'pkg'}
      end
      meth get_catdir($)
	 {RelativeTo @mogulDIR 'info/category'}
      end
      meth get_infodir($)
	 {RelativeTo @mogulDIR 'info'}
      end
      meth 'wget'(V) wget<-V end
      meth get_wget($) @wget end
      meth is_verbose($) @verbose end
      meth 'update-pub'(V)
	 if V then
	    if @db==unit then
	       {Raise mogul('update-pub'(no_db_is_opened))}
	    end
	    {@db updatePub(@rootID)}
	 end
      end
      meth get_css($) @css end
      meth 'css'(V) css<-V end
      meth id_to_relurl(ID $)
	 {URL.toString {Adjoin {URL.make ID}
			url(scheme:unit absolute:false)}}
      end
      
      %% given the mogul id of a package return the SECTION-PACKAGE
      %% string that is used to form the 1st part of a package file
      %% according to the new naming scheme.  With the old naming
      %% scheme we only have files SECTION-PACKAGE.pkg.  With the
      %% new one, we have SECTION-PACKAGE__FORMAT__PLATFORM__VERSION.pkg
      
      meth id_to_package_name(ID $)
	 U = {Adjoin {URL.make ID} url(scheme:unit absolute:false)}
	 L = case {Reverse U.path}
	     of nil|L then {Reverse L}
	     [] L then {Reverse L}
	     end
      in
	 {FoldL L fun {$ Accu X}
		     if Accu==nil then X else Accu#'-'#X end
		  end nil}
      end
      
      meth id_to_href(ID $)
	 S = Admin,id_to_relurl(ID $)
      in
	 @mogulTOP#'/info/'#S#'.html'
      end
      meth id_to_docdir_href(ID $)
	 S = Admin,id_to_relurl(ID $)
      in
	 @mogulTOP#'/doc/'#S#'/'
      end
      meth id_to_pkgdir_href(ID $)
	 S = Admin,id_to_relurl(ID $)
      in
	 @mogulTOP#'/pkg/'#S#'/'
      end
      meth condGetId(ID D $)
	 if @db==unit then D else {@db condGet(ID D $)} end
      end
      meth 'update-html'(V)
	 if V then
	    if @db==unit then
	       {Raise mogul('update-html'(no_db_is_opened))}
	    end
	    {@db updateHtml(@rootID)}
	 end
      end
      meth id_to_html_filename(ID $)
	 S = Admin,id_to_relurl(ID $)
      in
	 {RelativeTo @mogulDIR 'info/'#S#'.html'}
      end
      meth 'mogul-dir'(V) mogulDIR<-V end
      meth 'mogul-url'(V) mogulURL<-V end
      meth 'update-provided'(V)
	 if V then
	    Admin,InitProvided
	    {@db updateProvided(@rootID @provided)}
	 end
      end
      meth InitProvided
	 if @provided==unit then
	    provided <-
	    {Record.toDictionary
	     if @db\=unit then {@db condGet('*provided*' o $)} else o end}
	 end
      end
      meth 'print-provided'(V)
	 if V then
	    Admin,InitProvided
	    {Manager trace('Printing map module->package')}
	    for E in {Sort {Dictionary.entries @provided} fun {$ X#_ Y#_} X<Y end} do
	       case E of Module#PkgIds then
		  {System.printError Module#'  --> '}
		  for P in PkgIds Sep in ' ';', ' do
		     {System.printError Sep#P}
		  end
		  {System.printError '\n'}
	       end
	    end
	 end
      end
      meth get_providers(File $)
	 Admin,InitProvided
	 {Dictionary.condGet @provided
	  {VirtualString.toAtom File} nil}
      end
      meth cat_to_href(C $)
	 Admin,InitCategories
	 case {CondSelect @categories C unit}
	 of unit then unit
	 [] R then @mogulTOP#'/info/category/'#R.normalized#'.html' end
      end
      meth InitCategories
	 if @categories==unit then
	    categories <-
	    if @db==unit then o else {@db condGet('*categories*' o $)} end
	 end
      end
      meth 'categories-url'(V)
	 categoriesURL<-V
      end
      meth 'update-categories'(V)
	 if V then
	    Admin,InitCategories
	    {@db updateCategories({RelativeTo @mogulURL @categoriesURL})}
	 end
      end
      meth set_categories(V) categories<-V end
      meth get_categories($)
	 Admin,InitCategories
	 @categories
      end
      meth 'update-categories-html'(V)=M
	 if V then
	    try
	       if @db==unit then
		  {Raise mogul('update-categories-html'(no_db_is_opened))}
	       end
	       {Category.updateCatPages}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      meth 'update-package-list'(V)=M
	 if V then
	    try
	       if @db==unit then
		  {Raise mogul('update-package-list'(no_db_is_opened))}
	       end
	       {@db updatePkgList(@rootID)}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      meth 'update-author-list'(V)=M
	 if V then
	    try
	       if @db==unit then
		  {Raise mogul('update-author-list'(no_db_is_opened))}
	       end
	       {@db updateAuthorList(@rootID)}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      %%
      meth get_packages($)
	 %% if you call get_packages then update then
	 %% call it again, you will get the old list prior
	 %% to updating. oh hum.
	 if @packages==unit then
	    packages<-
	    {Filter
	     {Map {@db condGet('*package list*' nil $)}
	      fun {$ Id}
		 {@db condGet(Id unit $)}
	      end}
	     fun {$ E} E\=unit end}
	 end
	 @packages
      end
      meth get_authors($)
	 if @Authors==unit then
	    Authors<-{@db condGet('*author list*' nil $)}
	 end
	 @Authors
      end
      %%
      meth 'update-package-list-html'(V)=M
	 if V then
	    try
	       if @db==unit then
		  {Raise mogul('update-package-list-html'(no_db_is_opened))}
	       end
	       {Category.updatePkgListPage}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      %%
      meth 'update-author-list-html'(V)=M
	 if V then
	    try
	       if @db==unit then
		  {Raise mogul('update-author-list-html'(no_db_is_opened))}
	       end
	       {HTML_ByAuthor.updatePage}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      %%
      meth 'update-ozpm-info'(V)=M
	 if V then
	    try
	       if @db==unit then
		  {Raise mogul('update-ozpm-info'(no_db_is_opened))}
	       end
	       {Pickle.saveCompressed
		{@db get_ozpm_info($)}
		{RelativeTo @mogulDIR 'ozpm.info'}
		9}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      %%
      meth 'mogul-top'(V) mogulTOP<-V end
      meth getTop($) @mogulTOP end
      meth getCssLink($)
	 link(rel : 'stylesheet'
	      type: 'text/css'
	      href: @mogulTOP#'/mogul.css')
      end
      %%
      meth 'ignore-id'(L)
	 for S in L do RE={Regex.make S} in
	    ignoreID <- RE|@ignoreID
	 end
      end
      meth ignoreID(ID $)
	 {Some @ignoreID fun {$ RE} {Regex.search RE ID}\=false end}
      end
      %%
      meth 'ignore-url'(L)
	 for S in L do RE={Regex.compile S [extended newline icase]} in
	    ignoreURL <- RE|@ignoreID
	 end
      end
      meth ignoreURL(URL $)
	 {Some @ignoreURL fun {$ RE} {Regex.search RE URL}\=false end}
      end
      %%
      meth 'update-ozmake'(B)=M
	 if B then
	    try
	       if @db==unit then
		  {Raise mogul('update-ozmake'(no_db_is_opened))}
	       end
	       DIR = {RelativeTo @mogulDIR 'ozmake'}
	    in
	       {Directory.mkDir DIR}
	       {Pickle.saveCompressed
		{@db get_ozmake_info($)}
		{RelativeTo DIR 'database.ozf'}
		9}
	    catch mogul(...)=E then
	       Admin,addReport(M E)
	    end
	 end
      end
      %%
      meth addToManifest(Filename)
	 @manifest.{VirtualString.toAtom Filename} := unit
      end
      meth 'save-manifest'(V)
	 if V then
	    File = {New Open.file init(name:@mogulTOP#'/pkg/MANIFEST'
				       flags:[write create truncate])}
	 in
	    try
	       for F in {Sort {Dictionary.keys @manifest} Value.'<'} do
		  {File write(vs:F#'\n')}
	       end
	    finally
	       try {File close} catch _ then skip end
	    end
	 end
      end
   end

   Manager = {New Admin init}

   proc {Trace Msg}
      {Manager trace(Msg)}
   end
   proc {Indent} {Manager indent} end
   proc {Dedent} {Manager dedent} end
end
