functor
import
   Admin(manager:Manager)
export
   GetNavigationBar
define
   NavBar
   fun{GetNavigationBar}
      Ls=if {IsDet NavBar} then NavBar
	 else NavBar={GetNavBar1} end
   in
      'div'('class':"margin" 
	table({MMap Ls fun{$ X}
			  case X of _#Anchor then
%				Cat :: cp, exe, ...
%				Anchor :: a(1:"..." alt:"..." href:"...")#" (X)"
			     '#'(tr(td(Anchor)))
			  [] separator then
			     '#'(tr(td(hr)))
			  end
		       end}
	     )
       )
   end
   fun{GetNavBar1}
      {Manager trace('Creating navigationbar')}
      TopUrl = {Manager getTop($)}
      InfoUrl = TopUrl#"/info"
      Cats   = {Manager get_categories($)}
      Pkgs   = {Manager get_packages($)}
   in
      %% Some default links followed by the categories
      separator|
      index#a(href:TopUrl#"/index.html" "Index")|
      packages#a(href:InfoUrl#"/packages.html" "All Packages")|
      category#a(href:InfoUrl#"/category/index.html" "All Categories")|
      authors#a(href:InfoUrl#"/byauthor.html" "By Author")|
      separator|
      {Append
       {Map {Record.toListInd Cats} 
	fun{$ C#R}
	   L={Length {Filter Pkgs fun{$ P} {P hasCategory(C $)} end}}
	in
	   C#(a(href:{Manager cat_to_href(C $)}
		{Atom.toString C}
		alt:R.description)#"&nbsp;("#L#")")
	end}
       [separator]}
   end
   %% Makes record '#'(...) of list [...]
   fun{MMap Ls F}
      {List.toRecord '#' {List.mapInd Ls fun{$ I X} I#{F X} end}}
   end
end
