functor
import
   Admin(manager:Manager)
   Message(parseNodup:ParseNodup)
   URL(toAtom resolve)
   Externalizable('class':XT)
   GetSlot('class':GS)
   HTML_Section('class':HTMLClass)
export
   'class' : Section
define
   class Section from GS XT HTMLClass
      attr id pid url email toc body:unit
      meth init(Msg Id Url Pid Prev)
	 {Manager incTrace('--> init Section '#Id)}
	 try
	    id    <- Id
	    url   <- Url
	    pid   <- Pid
	    email <- {Msg condGet1('email' unit $)}
	    toc   <- toc()
	    Body = {Msg getBody($)}
	 in
	    if Body\=unit then
	       {Manager trace('Parsing message body')}
	       Toc = {ParseNodup Body}
	    in
	       for E in {Toc entries($)} do
		  case E of K#[V] then
		     {Msg check_id_prefix(K Id)}
		     toc <- {AdjoinAt @toc K {URL.toAtom {URL.resolve Url V}}}
		  end
	       end
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
end
