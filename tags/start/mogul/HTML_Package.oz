functor
import
   HTML_Entry('class':HE)
   Text(htmlQuote:HtmlQuote)
   Admin(manager:Manager)
   Author(toHref)
   URL
export
   'class' : HTML_Package
define
   class HTML_Package from HE
      meth formatHeaders($)
	 'div'(
	    'class' : 'formatheaders'
	    table(
	       'class' : 'formatheaders'
	       'width' : '100%'
	       'cellspacing' : '0'
	       'border': '0'
	       {self formatHeader('type' "section" $)}
	       {self formatHeader('id' tt({HtmlQuote @id}) $)}
	       if @pid==unit then '' else
		  {self formatHeader(
			   'section'
			   a(href:{Manager id_to_href(@pid $)}
			     {HtmlQuote @pid})
			   $)}
	       end
	       if @blurb==unit then '' else
		  {self formatHeader(
			   'blurb'
			   {HtmlQuote @blurb}
			   $)}
	       end
	       if @author==nil then '' else
		  {self formatHeaderEnum(
			   'author'
			    {Map @author Author.toHref}
			   $)}
	       end
	       if @contact==nil then '' else
		  {self formatHeaderEnum(
			   'contact'
			   {Map @contact Author.toHref}
			   $)}
	       end
	       if @categories==nil then '' else
		  {self formatHeader(
			   'category'
			   {self categoriesToHrefs($)}
			   $)}
	       end
	       /*
	       if @keywords==nil then '' else
		  {self formatHeader(
			   'keywords'
			   ...
			   $)}
	       end
	       */
	       if @provides==nil then '' else
		  {self formatHeaderEnum(
			   'provides'
			   {Map @provides
			    fun {$ S} tt({HtmlQuote S}) end}
			   $)}
	       end
	       if @requires==nil then '' else
		  {self formatHeaderEnum(
			   'requires'
			   {Map @requires
			    fun {$ S}
			       {self requiresToHrefs(S $)}
			    end}
			   $)}
	       end
	       local Docs = {self docHrefs($)} in
		  if Docs==nil then '' else
		     {self formatHeader(
			      'documentation'
			      {List.toTuple span Docs}
			      'class':'headerdoc'
			      $)}
		  end
	       end
	       local Pkgs = {self pkgHrefs($)} in
		  if Pkgs==nil then '' else
		     {self formatHeaderEnum(
			      'download' Pkgs
			      'class':'headerdoc'
			      $)}
		  end
	       end
	       )
	    )
      end
      %%
      meth docHrefs($)
	 Dir = '../doc/'#@id#'/'
      in
	 {FoldR
	  {Filter
	   {Map @url_doc
	    fun {$ U}
	       case {Reverse {URL.make U}.path}
	       of nil|D|_ then
		  a(href:Dir {HtmlQuote D#'/'})
	       [] D|_ then
		  a(href:Dir#D {HtmlQuote D})
	       else unit end
	    end}
	   fun {$ X} X\=unit end}
	  fun {$ A Accu}
	     if Accu==nil then [A] else A|" | "|Accu end
	  end nil}
      end
      %%
      meth pkgHrefs($)
	 Dir = '../pkg/'#@id#'/'
      in
	 {Filter
	  {Map @url_pkg
	   fun {$ U}
	      case {Reverse {URL.make U}.path}
	      of nil|D|_ then
		 a(href:Dir#D {HtmlQuote D})
	      [] D|_ then
		 a(href:Dir#D {HtmlQuote D})
	      else unit end
	   end}
	  fun {$ X} X\=unit end}
      end
      %%
      meth requiresToHrefs(File $)
	 case {Manager get_providers(File $)}
	 of nil then tt({HtmlQuote File})
	 [] [Id] then a(href:{Manager id_to_href(Id $)}
			{HtmlQuote File})
	 [] L then
	    {List.toTuple span
	     tt({HtmlQuote File})
	     |{Map L fun {$ Id}
			span("["
			     a(href:{Manager id_to_href(Id $)}
			       {HtmlQuote Id})
			     "]")
		     end}}
	 end
      end
      %%
      meth categoriesToHrefs($)
	 {List.toTuple span
	  {FoldR
	   {Map @categories
	    fun {$ C}
	       H = {Manager cat_to_href(C $)}
	       S = tt({HtmlQuote C})
	    in
	       if H==unit then S else a(href:'category/'#H S) end
	    end}
	   fun {$ X Accu}
	      if Accu==nil then [X] else X|", "|Accu end
	   end nil}}
      end
   end
end
