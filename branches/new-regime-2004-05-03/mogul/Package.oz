functor
import
   Externalizable('class':XT)
   Admin(manager:Manager)
   GetSlot('class':GS)
   Wget(wgetPkg wgetDoc)
   URL(toString resolve toBase make)
   Entry('class':EntryClass)
   HTML_Package('class':HTMLClass)
   Regex(compile search group allMatches make) at 'x-oz://contrib/regex'
   Text(strip:Strip split:Split)
   MogulID(normalizeID:NormalizeID)
   Property OS
export
   'class' : Package
define
   %% !!! there should be a `validate' method to make sure that
   %% all slots have ok values
   class Package from GS XT EntryClass HTMLClass
      feat type : package
      attr id pid url blurb provides requires content_type
	 url_pkg url_doc body author contact keywords
	 format:nil
	 categories url_doc_extra title version:unit
      meth init(Msg Id Url Pid Prev)
	 {Manager incTrace('--> init Package '#Id)}
	 try
	    EntryClass,init(Msg)
	    id           <- {NormalizeID Id Pid}
	    pid          <- Pid
	    url          <- Url
	    blurb        <- {Msg condGet1('blurb' unit $)}
	    provides     <- {Msg condGet('provides' nil $)}
	    requires     <- {Msg condGet('requires' nil $)}
	    url_pkg      <- {Msg condGet('url-pkg' nil $)}
	    url_doc      <- {Msg condGet('url-doc' nil $)}
	    author       <- {Msg condGet('author' nil $)}
	    contact      <- {Msg condGet('contact' nil $)}
	    keywords     <- {Append {Msg getSplit('keyword' $)}
			     {Msg getSplit('keywords' $)}}
	    format       <- {Msg condGet1('format' nil $)}
	    local Table={NewDictionary} in
	       for C in {Msg condGet('category' nil $)} do
		  Table.{VirtualString.toAtom C} := unit
	       end
	       for A in {self getCategories($)} do
		  Table.A := unit
	       end
	       categories <- {Sort {Dictionary.keys Table} Value.'<'}
	    end
	    url_doc_extra<- {Msg condGet('url-doc-extra' nil $)}
	    title        <- {Msg condGet1('title' unit $)}
	    version      <- {Msg condGet1('version' unit $)}
	    %% !!! here we should copy the persistent info from Prev
	 finally
	    {Manager decTrace('<-- init Package '#Id)}
	 end
      end
      meth extern_label($) 'package' end
      meth extern_slots($)
	 [id pid url blurb provides requires content_type
	  url_pkg url_doc body author contact keywords
	  categories url_doc_extra title version]
      end
      meth printOut(Margin Out DB)
	 {Out write(vs:Margin#' '#@id#' (package)\n')}
      end
      meth updatePub(DB)
	 {Manager incTrace('--> updatePub package '#@id)}
	 try
	    for U in @url_pkg do {self UpdatePkg(U DB)} end
	    for U in @url_doc do {self UpdateDoc(U DB)} end
	    for U in @url_doc_extra do {self UpdateDocExtra(U DB)} end
	 finally
	    {Manager decTrace('<-- updatePub package '#@id)}
	 end
      end
      meth get_id_as_rel_path($)
	 {Manager id_to_relurl(@id $)}
      end
      meth get_this_pkgdir($)
	 {URL.toString
	  {URL.resolve
	   {URL.toBase {Manager get_pkgdir($)}}
	   {self get_id_as_rel_path($)}}}
      end
      %% here, we compute a new style pkg file name
      meth get_this_pkgname($ format:FMT<=unit)
	 Main = {Manager id_to_package_name(@id $)}
	 Format = if FMT\=unit then FMT
		  elseif @format==nil orelse @format==unit
		  then 'xxx' else @format end
	 Platform = 'source' % currently
	 Version = if @version==unit then '0' else @version end
      in
	 Main#'__'#Format#'__'#Platform#'__'#Version#'.pkg'
      end
      meth UpdatePkg(U DB)
	 M = {Regex.search RE_PROVIDES U}
	 U2 = {Regex.group 2 M U}
	 U3 = {URL.make U}
	 N = case {Reverse U3.path}
	     of nil|D|_ then D
	     [] D|_ then D
	     end
	 IsPKG = case {Reverse N}
		 of &g|&k|&p|&.|_ then true
		 else false end
	 FormatIsKnown = @format\=nil andthen @format\=unit
	 File = if IsPKG then
		   {self get_this_pkgname($)}
		else N end
	 Dir = {self get_this_pkgdir($)}
      in
	 {Manager trace('Downloading pkg '#U2)}
	 try
	    {Wget.wgetPkg U2 Dir File}
	    if IsPKG then
	       if FormatIsKnown then
		  {Manager addToManifest(Dir#'/'#File)}
	       else
		  FMT = {self determineFormat(Dir#'/'#File $)}
		  FileFinal = {self get_this_pkgname($ format:FMT)}
		  Cmd = 'mv '#Dir#'/'#File#' '#Dir#'/'#FileFinal
	       in
		  {Manager trace(Cmd)}
		  {OS.system Cmd _}
		  {Manager addToManifest(Dir#'/'#FileFinal)}
	       end
	    end
	 catch mogul(...)=E then
	    {Manager addReport(update_pub_pkg(@id) E)}
	 end
      end
      %% determine whether the package stored in File is in the current
      %% format (1.2.5 or 1.3.0)
      meth determineFormat(File $)
	 {Manager trace('--> determining format')}
	 Format = if {OS.system {Property.get 'oz.home'}#'/bin/'#
		      'ozmake -xnp '#File#' --dir=/tmp/MOGUL.TMP -q'}==0
		  then '1.3.0' else '1.2.5' end
      in
	 {Manager trace('<-- '#Format)}
	 Format
      end
      meth UpdateDoc(U DB)
	 {Manager trace('Downloading doc '#U)}
	 try {Wget.wgetDoc U
	      {URL.toString
	       {URL.resolve
		{URL.toBase {Manager get_docdir($)}}
		{self get_id_as_rel_path($)}}}}
	 catch mogul(...)=E then
	    {Manager addReport(update_pub_doc(@id) E)}
	 end
      end
      meth UpdateDocExtra(U DB)
	 {Manager trace('Downloading doc extra '#U)}
	 DocDir = {URL.toString
		   {URL.resolve
		    {URL.toBase {Manager get_docdir($)}}
		    {self get_id_as_rel_path($)}}}
	 Dir Url
      in
	 case {Regex.search RE_DOC_EXTRA U}
	 of false then Dir=DocDir Url=U
	 [] M then Dir=DocDir#'/'#{Strip {Regex.group 1 M U}}
	    Url = {Strip {Regex.group 2 M U}}
	 end
	 try {Wget.wgetDoc Url Dir}
	 catch mogul(...)=E then
	    {Manager addReport(update_pub_doc_extra(@id) E)}
	 end
      end
      meth updateProvided(DB D)
	 {Manager incTrace('--> updateProvided package '#@id)}
	 try
	    for X in @provides do
	       S = {VirtualString.toAtom
		    case {Regex.search RE_PROVIDES X}
		    of false then X
		    [] M then {Strip {Regex.group 2 M X}} end}
	       L = {Dictionary.condGet D S nil}
	    in
	       if {Not {Member @id L}} then
		  {Dictionary.put D S @id|L}
	       end
	    end
	 finally
	    {Manager decTrace('<-- updateProvided package '#@id)}
	 end
      end
      meth getCategories($)
	 D = {NewDictionary}
      in
	 for X in @provides do
	    for M in {Regex.allMatches RE_CATEGORY X} do
	       for S in {Split {Regex.group 1 M X} RE_WORD_SEP} do
		  A = {VirtualString.toAtom S}
	       in
		  if A\='' then D.A := true end
	       end
	    end
	 end
	 {Dictionary.keys D}
      end
      %%
      meth updatePkgList(DB L $) @id|L end
      %%
      %%
      meth updateAuthorList(DB L $) 
	 package(id:@id authors:@author)|L
      end
      %%
      meth hasCategory(C $)
	 {Member C @categories}
      end
   end
   %%
   RE_PROVIDES = {Regex.compile '^(\\[[^]]*\\][[:space:]]*)*(.*)$' [extended]}
   RE_CATEGORY = {Regex.make '^\\[([^]]*)\\]'}
   RE_WORD_SEP = {Regex.make '[[:space:],;]+'}
   RE_DOC_EXTRA= {Regex.make '^\\[([^]]+)\\][[:space:]]+(.+)$'}
end
