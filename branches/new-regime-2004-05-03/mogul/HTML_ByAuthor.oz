functor
import
   Admin(manager:Manager)
   Open(file html)
   Text(htmlQuote:HtmlQuote)
   Directory(mkDirForFile)
   Browser
   HTML_Navigation(getNavigationBar)
export
   UpdatePage
define
   class HTML_File from Open.file Open.html end
   proc {WriteHtmlPage Page File}
      {Directory.mkDirForFile File}
      Out = {New HTML_File init(name:File flags:[write create truncate])}
   in
      {Out tag(Page)}
      {Out close}
   end
   
   %% Makes record '#'(...) of list [...]
   fun{MMap Ls F}
      {List.toRecord '#' {List.mapInd Ls fun{$ I X} I#{F X} end}}
   end

   fun{GetAuthorList}
      As={Manager get_authors($)}
   in
      {Sort
       {Filter
	{Map {Record.toListInd As}
	 fun{$ I#X}
	    {Manager condGetId(I unit $)} #
	    {Map X fun{$ Y} {Manager condGetId(Y unit $)} end}
	 end}
	fun{$ X} X.1\=unit andthen X.1.type==contact end}
       fun {$ X Y}
	  %% this totally sucks
	  {VirtualString.toAtom {X.1 getSlot('name_for_index' $)}}<
	  {VirtualString.toAtom {Y.1 getSlot('name_for_index' $)}}
       end}
   end
   
   proc{UpdatePage}
      Tables={MMap {GetAuthorList}
	      fun{$ I#Ps}
		 Name={I getSlot(name $)}
		 NameId={I getSlot(id $)}
		 Rs=
		 {Map Ps fun{$ P}
			    Id={P getSlot('id' $)}
			    Blurb={P getSlot('blurb' $)}
			    Desc=if Blurb==unit then "&nbsp;"
				 else {HtmlQuote Blurb} end
			 in
			    tr('class':'header'
			       'valign':'top'
			       td('class':'header'
				  a(href:{Manager id_to_href(Id $)} tt(Id)))
			       td('class':'header' ":")
			       td('class':'value' Desc))
			 end}
	      in
		 '#'(
		    h1('class':"title" a(href:{Manager id_to_href(NameId $)} tt(Name)))
		    'div'('class':'entryinfo'
			  'div'('class':'formatheaders'
				{Adjoin
				 {MMap Rs fun{$ X} X end}
				 table(
				    'class' : 'formatheaders'
				    'width' : '100%'
				    'cellspacing' : '0'
				    'border': '0')}
			       ))
		    )
	      end}
      
      Body=html(
	      head(
		 title("By Author")
		 {Manager getCssLink($)})
	      body({HTML_Navigation.getNavigationBar}
		   Tables
		  )
	      )
      Dir={Manager get_infodir($)}
      File=Dir#"/byauthor.html"
   in
      try
	 {WriteHtmlPage Body File}
      catch E then
	 {Browser.browse nils(E)}
	 {Wait _}
      end
   end
end



