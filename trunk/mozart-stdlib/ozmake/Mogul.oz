functor
export
   'class' : PubMogul
prepare
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
   Open
   Path at 'Path.ozf'
   Utils at 'Utils.ozf'
define
   %% here we keep track of the user's MOGUL offerings and
   %% automate the management of the user's MOGUL section
   class PubMogul
      attr
	 DB : unit

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
	 if {HasFeature R author   } then PKG.author    := R.author    end
	 if {HasFeature R blurb    } then PKG.blurb     := R.blurb     end
	 if {HasFeature R info_text} then PKG.info_text := R.info_text end
	 if {HasFeature R info_html} then PKG.info_html := R.info_html end
	 if {HasFeature R mogul    } then PKG.mogul     := R.mogul     end
	 if {HasFeature R released } then PKG.released  := R.released  end
	 if {HasFeature R version  } then PKG.version   := R.version   end
	 if {HasFeature R requires } then PKG.requires  := R.requires  end
	 DB.(PKG.mogul) := {Dictionary.toRecord package PKG}
      end

      meth ToMogulEntry(R VS)
	 Q={NewQueue}
      in
	 {Q.put 'type:         package\n'}
	 for A in {CondSelect R author nil} do
	    {Q.put 'author:       '#A#'\n'}
	 end
	 if {HasFeature R blurb} then {Q.put 'blurb:        '#R.blurb#'\n'} end
	 {Q.put 'url-pkg:      '#
	  {Path.resolve {self get_mogulpkgurl($)}
	   {Utils.mogulToPackagename {self get_mogul($)}}}#'\n'}
	 case {CondSelect R doc unit} of F|_ then
	    {Q.put 'url-doc:      '#
	     {Path.resolve {self get_moguldocurl($)}
	      {Utils.mogulToPackagename {self get_mogul($)}}#'/'#F}#'\n'}
	 end
	 for T in {self get_lib_targets($)} do
	    {Q.put 'provides:     '#
	     {Path.resolve {self get_uri($)} T}#'\n'}
	 end
	 for T in {self get_bin_targets($)} do
	    {Q.put 'provides:     '#T#'\n'}
	 end
	 if {HasFeature R info_html} then
	    {Q.put 'content-type: text/html\n\n'}
	    {Q.put R.info_html}
	 elseif {HasFeature R info_text} then
	    {Q.put 'content-type: text/plain'}
	    {Q.put R.info_text}
	 end
	 {List2VS {Q.toList} VS}
	 {self print(VS)}
      end

      meth mogul()
	 %% here we update the MOGUL entry for this package
	 {self makefile_read}
	 D  = {self get_moguldbdir($)}
	 VS = {self ToMogulEntry({self makefile_to_record($)} $)}
	 F  = {Utils.mogulToFilename {self get_mogul($)}}#'.mogul'
	 FF = {Path.resolve D F}
	 {self trace('writing '#FF)}
	 {Path.makeDirRec {Path.dirname FF}}
      in
	 if {self get_justprint($)} then skip else
	    O  = {New Open.file init(name:FF flags:[write create truncate])}
	 in
	    try {O write(vs:VS)}
	    finally try {O close} catch _ then skip end end
	 end
      end
   end
end
