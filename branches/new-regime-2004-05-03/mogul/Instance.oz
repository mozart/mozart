functor
import
   Externalizable('class':XT)
   Admin(manager:Manager)
   GetSlot('class':GS)
   Wget(wgetPkg wgetDoc)
   %URL(toString resolve toBase make)
   Entry('class':EntryClass)
   HTML_Package('class':HTMLClass)
   %Regex(compile search group allMatches make) at 'x-oz://contrib/regex'
   %Text(strip:Strip split:Split)
   %MogulID(normalizeID:NormalizeID)
export
   'class' : Instance
define
   class Instance from GS XT EntryClass HTMLClass
      feat type : instance
      attr
	 id pid blurb url provides requires content_type
	 url_pkg body author contact keywords
	 format:nil categories title version:unit platform:unit

      meth init(Msg Id Url Pid Prev)
	 {Manager incTrace('--> init Instance '#ID)}
	 try
	    EntryClass,init(Msg)
	    id           <- {NormalizeID Id Pid}
	    pid          <- Pid
	    url          <- Url
	    blurb        <- {Msg condGet1('blurb' unit $)}
	    provides     <- {Msg condGet('provides' nil $)}
	    requires     <- {Msg condGet('requires' nil $)}
	    url_pkg      <- {Msg condGet('url-pkg' nil $)}
	    %url_doc      <- {Msg condGet('url-doc' nil $)}
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
	    %url_doc_extra<- {Msg condGet('url-doc-extra' nil $)}
	    title        <- {Msg condGet1('title' unit $)}
	    version      <- {Msg condGet1('version' unit $)}
	    %% !!! here we should copy the persistent info from Prev
	 finally
	    {Manager decTrace('<-- init Instance')}
	 end
      end

      %% newFake(Pkg) is used to create a fake instance for an
      %% old-style package
      
      meth newFake(Pkg)
	 {Manager incTrace('--> fake Instance')}
	 try
	    pid        <- {Pkg getSlot(id $)}
	    url        <- unit
	    blurb      <- {Pkg getSlot(blurb      $)}
	    provides   <- {Pkg getSlot(provides   $)}
	    requires   <- {Pkg getSlot(requires   $)}
	    url_pkg    <- {Pkg getSlot(url_pkg    $)}
	    author     <- {Pkg getSlot(author     $)}
	    contact    <- {Pkg getSlot(contact    $)}
	    keywords   <- {Pkg getSlot(keywords   $)}
	    format     <- {Pkg getSlot(format     $)}
	    categories <- {Pkg getSlot(categories $)}
	    title      <- {Pkg getSlot(title      $)}
	    version    <- {Pkg getSlot(version    $)}
	    platform   <- 'source' % by default
	 finally
	    {Manager decTrace('<-- fake Instance')}
	 end
      end
   end
end
