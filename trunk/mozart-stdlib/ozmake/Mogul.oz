functor
prepare
   fun {Return X} X end
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
      end
   end
end
