functor
import
   Admin(manager:Manager)
   Message(parseNodup:ParseNodup)
   URL(toAtom resolve)
   Externalizable('class':XT)
   GetSlot('class':GS)
   HTML_Section('class':HTMLClass)
   MogulID(normalizeID:NormalizeID)
export
   'class' : Section
define
   class Section from GS XT HTMLClass
      feat type : section
      attr id pid url email toc body:unit
      meth init(Msg Id Url Pid Prev)
	 {Manager incTrace('--> init Section '#Id)}
	 try
	    id    <- {NormalizeID Id Pid}
	    url   <- Url
	    pid   <- Pid
	    email <- {Msg condGet1('email' unit $)}
	    toc   <- toc()
	    Body = {Msg getBody($)}
	 in
	    if Body\=unit then
	       {Manager trace('Parsing message body')}
	       Toc = {ParseNodup {Sanitize {VirtualString.toString Body}}}
	    in
	       toc <- {List.toRecord toc
		       {Map {Toc entries($)}
			fun {$ K#[V]}
			   {NormalizeID K @id} #
			   {URL.toAtom {URL.resolve Url V}}
			end}}
	    end
	 finally
	    {Manager decTrace('<-- init Section '#Id)}
	 end
      end
      meth extern_label($) 'section' end
      meth extern_slots($)
	 [id pid url email toc]
      end
      meth printOut(Margin Out DB)
	 {Out write(vs:Margin#' '#@id#' (section)\n')}
	 for K in {Arity @toc} do
	    {DB printOut(K Margin#'+' Out)}
	 end
      end
      meth updatePub(DB)
	 {Manager incTrace('--> updatePub section '#@id)}
	 try
	    for K in {Arity @toc} do
	       {DB updatePub(K)}
	    end
	 finally
	    {Manager decTrace('<-- updatePub section '#@id)}
	 end
      end
      meth updateProvided(DB D)
	 {Manager incTrace('--> updateProvided section '#@id)}
	 try
	    for K in {Arity @toc} do
	       {DB updateProvided(K D)}
	    end
	 finally
	    {Manager decTrace('<-- updateProvided section '#@id)}
	 end
      end
      meth updatePkgList(DB L $)
	 if {Manager ignoreID(@id $)} then
	    {Manager trace('Ignoring ID='#@id)}
	    L % this is wrong, should use recorded stuff, but I cannot remember...
	 else
	    {Manager incTrace('--> updatePkgList section '#@id)}
	    try
	       {FoldL {Arity @toc}
		fun {$ L K}
		   {DB updatePkgListFor(K L $)}
		end L}
	    finally
	       {Manager decTrace('<-- updatePkgList section '#@id)}
	    end
	 end
      end
      %%
      meth updateAuthorList(DB L $)
	 if {Manager ignoreID(@id $)} then
	    {Manager trace('Ignoring ID='#@id)}
	    L % wrong for the same reason as above
	 else
	    {Manager incTrace('--> updateAuthorList section '#@id)}
	    try
	       {FoldL {Arity @toc}
		fun {$ L K}
		   {DB updateAuthorListFor(K L $)}
		end L}
	    finally
	       {Manager decTrace('<-- updateAuthorList section '#@id)}
	    end
	 end
      end
   end
   %%
   %% fix section body for users who just cannot get it right
   %%
   local
      fun {Loop S}
	 case S
	 of nil then nil
	 [] &\n|T then &\n|{Sanitize T}
	 []   C|T then   C|{Loop T}
	 end
      end
   in
      fun {Sanitize S}
	 case S
	 of &m|&o|&g|&u|&l|&:|L then
	    case L
	    of &/|&/|&/|T then &/|{Loop T}
	    []    &/|&/|T then &/|{Loop T}
	    else {Loop L} end
	 else {Loop S} end
      end
   end
end
