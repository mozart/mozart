functor
   %% ACTIONS
   %%     1. write a package entry either using the makefile or a given package
   %%     2. write contact entries
   %%     3. list mogul info using makefile or given package
   %%     4. list mogul database
   %%     5. write section entries for current database
   %%     6. delete entries in current database
   %% ozmake --mogul-update
   %% ozmake -M update
export
   'class' : PubMogul
prepare
   IsPrefix = List.isPrefix
   fun {Return X} X end
   fun {List2VSX Accu X} Accu#X end
   fun {List2VS L} {FoldL L List2VSX nil} end
   fun {NewQueue}
      C=local L in {NewCell L|L} end
      proc {Put X} L1 L2 in
	 {Exchange C L1|(X|L2) L1|L2}
      end
      proc {ToList L} {Exchange C L|nil unit} end
   in
      queue(put:Put toList:ToList)
   end
import
   Path at 'Path.ozf'
   Utils at 'Utils.ozf'
define
   %% here we keep track of the user's MOGUL offerings and
   %% automate the management of the user's MOGUL section
   class PubMogul
      attr
	 DB : unit
	 Dashes : unit
	 EntryFiles : unit

      meth pubmogul_read()
	 if @DB==unit then
	    F={self get_moguldatabase($)}
	    L={self databaselib_read(F Return $)}
	 in
	    if L\=unit then
	       PubMogul,Init(L)
	    elseif {self get_moguldatabase_given($)} then
	       raise ozmake(mogul:filenotfound(F#'.txt')) end
	    else
	       {self trace('no mogul database')}
	       PubMogul,Init(nil)
	    end
	 end
      end

      meth Init(L)
	 DB <- {NewDictionary}
	 for E in L do
	    DB.(E.mogul) := E
	 end
      end

      meth pubmogul_save()
	 F = {self get_moguldatabase($)}
      in
	 {self databaselib_save(F Return {Dictionary.items @DB})}
      end

      meth pubmogul_enter(R)
	 %% R is a makefile record describing a package
	 %% it is assumed that R has been processed by Database,Stringify
	 PKG = {NewDictionary}
      in
	 if {HasFeature R author    } then PKG.author      := R.author    end
	 if {HasFeature R blurb     } then PKG.blurb       := R.blurb     end
	 if {HasFeature R info_text } then PKG.info_text   := R.info_text end
	 if {HasFeature R info_html } then PKG.info_html   := R.info_html end
	 if {HasFeature R mogul     } then PKG.mogul       := R.mogul     end
	 if {HasFeature R released  } then PKG.released    := R.released  end
	 if {HasFeature R version   } then PKG.version     := R.version   end
	 if {HasFeature R requires  } then PKG.requires    := R.requires  end
	 if {HasFeature R categories} then PKG.categhories := R.categories end
	 DB.(PKG.mogul) := {Dictionary.toRecord package PKG}
      end

      meth ToMogulPackageEntry(R VS)
	 Q={NewQueue}
      in
	 {Q.put 'type:           package\n'}
	 {Q.put 'id:             '#R.mogul#'\n'}
	 for A in {CondSelect R author nil} do
	    {Q.put 'author:         '#A#'\n'}
	 end
	 if {HasFeature R blurb} then {Q.put 'blurb:          '#R.blurb#'\n'} end
	 {Q.put 'url-pkg:        '#
	  {Path.resolve {self get_mogulpkgurl($)}
	   {Utils.mogulToPackagename R.mogul}}#'\n'}
	 case {CondSelect R doc unit} of F|_ then
	    {Q.put 'url-doc:        '#
	     {Path.resolve {self get_moguldocurl($)}
	      {Utils.mogulToPackagename R.mogul}#'/'#F}#'\n'}
	 else skip end
	 for T in R.lib do %{self get_lib_targets($)} do
	    {Q.put 'provides:       '#
	     {Path.resolve R.uri T}#'\n'}
	 end
	 for T in R.bin do
	    {Q.put 'provides:       '#T#'\n'}
	 end
	 if {HasFeature R info_html} then
	    {Q.put 'content-type:   text/html\n\n'}
	    {Q.put R.info_html}
	 elseif {HasFeature R info_text} then
	    {Q.put 'content-type:   text/plain'}
	    {Q.put R.info_text}
	 end
	 {List2VS {Q.toList} VS}
      end

      meth ToMogulContactEntry(R VS)
	 Q={NewQueue}
      in
	 {Q.put 'type:           contact\n'}
	 {Q.put 'id:             '#R.mogul}
	 {Q.put '\nname:           '#R.name}
	 if {HasFeature R name_for_index} then
	    {Q.put '\nname-for-index: '#R.name_for_index}
	 end
	 if {HasFeature R email} then
	    {Q.put '\nemail:          '#R.email}
	 end
	 if {HasFeature R www} then
	    {Q.put '\nwww:            '#R.www}
	 end
	 {List2VS {Q.toList} VS}
      end

      meth HasPackageContribs(R $)
	 {CondSelect R lib nil}\=nil orelse
	 {CondSelect R bin nil}\=nil orelse
	 {CondSelect R doc nil}\=nil
      end

      meth HasContactContribs(R $)
	 {CondSelect R contact nil}\=nil
      end

      meth mogul_put()
	 %% here we update the MOGUL entry for this package
	 {self makefile_read_maybe_from_package}
	 R  = {self makefile_to_record($)}
	 PkgB = {self HasPackageContribs(R $)}
	 ConB = {self HasContactContribs(R $)}
      in
	 if PkgB then {self mogul_put_package(R)} end
	 if ConB then {self mogul_put_contact(R)} end
	 if {Not (PkgB orelse ConB)} then
	    {self trace('package contributes no MOGUL entries')}
	 end
      end

      meth mogul_put_package(R)
	 VS = {self ToMogulPackageEntry(R $)}
	 F  = {Utils.mogulToFilename R.mogul}#'.mogul'
	 D  = {self get_moguldbdir($)}
	 FF = {Path.resolve D F}
      in
	 {self exec_write_to_file(VS FF)}
	 {self trace({self format_title(' [ '#F#' ] ' $)})}
	 {self trace(VS)}
	 {self trace({self format_dashes($)})}
      end

      meth mogul_put_contact(R)
	 D  = {self get_moguldbdir($)}
      in
	 for C in R.contact do
	    VS = {self ToMogulContactEntry(C $)}
	    F  = {Utils.mogulToFilename C.mogul}#'.mogul'
	    FF = {Path.resolve D F}
	 in
	    {self exec_write_to_file(VS FF)}
	    {self trace({self format_title(' [ '#F#' ] ' $)})}
	    {self trace(VS)}
	    {self trace({self format_dashes($)})}
	 end
      end

      meth mogul()
	 case {self get_mogul_action($)}
	 of put then {self mogul_put}
	 [] db  then {self mogul_db_list}
	 end
      end

      meth mogul_validate_action(S $)
	 case for A in [put delete print db fix] collect:C do
		 if {IsPrefix S {AtomToString A}} then {C A} end
	      end
	 of nil then raise ozmake(mogul:unknownaction(S)) end
	 [] [A] then A
	 []  L  then raise ozmake(mogul:ambiguousaction(S L)) end
	 end
      end

      meth format_title(T $)
	 N  = {self get_linewidth($)} - {VirtualString.length T}
	 N1 = N div 2
	 N2 = N - N1
      in
	 for I in 1..N1 collect:COL do {COL &-} end#T#
	 for I in 1..N2 collect:COL do {COL &-} end
      end

      meth format_dashes($)
	 if @Dashes==unit then
	    Dashes <-
	    for I in 1..{self get_linewidth($)} collect:C do {C &-} end
	 end
	 @Dashes
      end

      meth mogul_db_readfiles()
	 if @EntryFiles==unit then
	    %% sections are deduced from the existing contact and package entries
	    %% which can be indentified by their .mogul extension
	    D = {self get_moguldbdir($)}
	    L =
	    for F in {Path.dir D} collect:Collect do
	       case {Reverse F}
	       of &l|&u|&g|&o|&m|&.|_ then {Collect {StringToAtom F}}
	       else skip end
	    end
	 in
	    EntryFiles <- {Sort L Value.'<'}
	 end
      end

      meth mogul_db_list()
	 {self mogul_db_readfiles}
	 D = {self get_moguldbdir($)}
      in
	 for E in @EntryFiles B in false;true do
	    if B then {self print(nil)} end
	    {self print({self format_title(' [ '#E#' ] ' $)})}
	    {self print({Utils.slurpFile {Path.resolve D E}})}
	 end
	 if @EntryFiles\=nil then
	    {self print({self format_dashes($)})}
	 end
      end

      meth CollectMogulIds()
	 {self mogul_db_readfiles}
	 EntryIds <-
	 for F in @EntryFiles collect:COL do
	    {COL for L in {String.tokens {Utils.slurpFile F} &\n} return:R do
		    case {String.token L &:}
		    of 
		 end}
	 end
      end
   end
end
