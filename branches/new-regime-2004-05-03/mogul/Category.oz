functor
import
   Open(file html)
   Admin(manager:Manager)
   Text(htmlQuote:HtmlQuote)
   Directory(mkDirForFile)
   HTML_Navigation(getNavigationBar)
export
   UpdateCatPages UpdatePkgListPage
define
   class HTML_File from Open.file Open.html end
   %%
   fun {MakePage Title Subtitle Rows Doc}
      html(
	 head(
	    title(Title)
	    {Manager getCssLink($)})
	 body({HTML_Navigation.getNavigationBar}
	    h1('class':'title' Subtitle)
	    'div'(
	       'class':'entryinfo'
	       'div'(
		  'class' : 'formatheaders'
		  {Adjoin
		   {List.toTuple table Rows}
		   table(
		      'class' : 'formatheaders'
		      'width' : '100%'
		      'cellspacing' : '0'
		      'border': '0')})
	       if Doc==unit then '' else
		  'div'(
		     'class':'formatbody'
		     Doc)
	       end)))
   end
   %%
   fun {MainPage Cats}
      {MakePage
       "Categories" "All Categories"
       {Map {Record.toListInd Cats}
	fun {$ C#R}
	   tr('class':'header'
	      'valign':'top'
	      td('class':'header'
		 a(href:R.normalized#'.html'
		   tt({HtmlQuote C})))
	      td('class':'header' ":")
	      td('class':'value' {HtmlQuote R.description}))
	end}
       unit}
   end
   %%
   fun {CatPage C R Pkgs}
      {MakePage C {HtmlQuote R.description}#" ("#C#")"
%       tr(th(colspan:3 'class':'catinfo' {HtmlQuote R.description}))|
       {Map {SortPkgs Pkgs}
	 fun {$ P}
	    Id = {P getSlot('id' $)}
	    Blurb = {P getSlot('blurb' $)}
	    Desc = if Blurb==unit then "&nbsp;" else {HtmlQuote Blurb} end
	 in
	    tr('class':'header'
	       'valign':'top'
	       td('class':'header'
		  a(href:{Manager id_to_href(Id $)} {HtmlQuote Id}))
	       td('class':'header' ":")
	       td('class':'value' Desc))
	 end}
       unit}
   end
   %%
   proc {WriteHtmlPage Page File}
      {Directory.mkDirForFile File}
      Out = {New HTML_File init(name:File flags:[write create truncate])}
   in
      {Out tag(Page)}
      {Out close}
   end
   %%
   proc {UpdateCatPages}
      {Manager trace('Updating category pages')}
      CatDir = {Manager get_catdir($)}
      Cats   = {Manager get_categories($)}
      Pkgs   = {Manager get_packages($)}
      {Manager trace('... page '#'/index.html')}
   in
      {WriteHtmlPage {MainPage Cats} CatDir#'/index.html'}
      for X in {Record.toListInd Cats} do
	 case X of C#R then
	    {Manager trace('... page '#CatDir#'/'#R.normalized#'.html')}
	    L = {Filter Pkgs fun {$ P} {P hasCategory(C $)} end}
	 in
	    {WriteHtmlPage {CatPage C R L}
	     CatDir#'/'#R.normalized#'.html'}
	 end
      end
   end
   %%
   proc {UpdatePkgListPage}
      {Manager trace('Updating package list page')}
      Dir = {Manager get_infodir($)}
      File = Dir#'/packages.html'
      {Manager trace('... page '#File)}
      Pkgs = {Manager get_packages($)}
   in
      {WriteHtmlPage
       {MakePage "Packages" "All Packages"
	{Map {SortPkgs Pkgs}
	 fun {$ P}
	    Id = {P getSlot('id' $)}
	    Blurb = {P getSlot('blurb' $)}
	    Desc = if Blurb==unit then "&nbsp;" else {HtmlQuote Blurb} end
	 in
	    tr('class':'header'
	       'valign':'top'
	       td('class':'header'
		  a(href:{Manager id_to_href(Id $)} tt(Id)))
	       td('class':'header' ":")
	       td('class':'value' Desc))
	 end}
	unit}
       File}
   end
   %%
   local
      fun {Cmp P1 P2}
	 {P1 getSlot('id' $)}<{P2 getSlot('id' $)}
      end
   in
      fun {SortPkgs L}
	 {Sort L Cmp}
      end
   end
end
