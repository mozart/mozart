functor
export
   'class' : Lister
prepare
   %% mogul uri author released installed blurb info_html info_text files lost zombies
   FEATURES =
   o(
      mogul     : '        mogul:  '
      uri       : '          uri:  '
      author    : '       author:  '
      version   : '      version:  '
      released  : '     released:  '
      installed : '    installed:  '
      )
import
   Utils at 'Utils.ozf'
define
   class Lister

      meth list
	 if {self get_package_given($)} then
	    Lister,list_package({self get_package($)})
	 else
	    Lister,list_all
	 end
      end

      meth list_all
	 {self database_read}
	 for E in {self database_get_packages($)} First in true;false do
	    if First then skip else {self print(nil)} end
	    Lister,LIST(E)
	 end
      end

      meth list_package(MOG)
	 {self database_read}
	 PKG = {self database_get_package({VirtualString.toAtom MOG} $)}
      in
	 if PKG==unit then
	    {self xtrace('package '#MOG#' not found')}
	 else
	    Lister,LIST(PKG)
	 end
      end

      meth LIST(PKG)
	 Lister,PrintTitle(' package ' &=)
	 %% mogul uri author released installed blurb info_html info_text files lost zombies
	 if {CondSelect PKG mogul unit}\=unit then
	    {self print(FEATURES.mogul#PKG.mogul)}
	 end
	 if {CondSelect PKG uri unit}\=unit then
	    {self print(FEATURES.uri#PKG.uri)}
	 end
	 if {CondSelect PKG author unit}\=unit then
	    for A in PKG.author do
	       {self print(FEATURES.author#A)}
	    end
	 end
	 if {CondSelect PKG version unit}\=unit then
	    {self print(FEATURES.version#PKG.version)}
	 end
	 if {CondSelect PKG released unit}\=unit then
	    {self print(FEATURES.released#{Utils.dateToUserVS PKG.released})}
	 end
	 if {CondSelect PKG installed unit}\=unit then
	    {self print(FEATURES.installed#{Utils.dateToUserVS PKG.installed})}
	 end
	 if {CondSelect PKG requires unit}\=unit then
	    Lister,PrintFiles(' requires ' PKG.requires)
	 end
	 if {CondSelect PKG blurb unit}\=unit then
	    Lister,PrintTitle(' blurb ' &-)
	    {self print(PKG.blurb)}
	 end
	 if {CondSelect PKG info_text unit}\=unit then
	    Lister,PrintTitle(' info_text ' &-)
	    {self print(PKG.info_text)}
	 elseif {CondSelect PKG info_html unit}\=unit then
	    Lister,PrintTitle(' info_html ' &-)
	    {self print(PKG.info_html)}
	 end
	 case {CondSelect PKG files unit}
	 of unit then skip
	 [] nil then skip
	 [] L then Lister,PrintFiles(' files ' L) end
	 case {CondSelect PKG lost unit}
	 of unit then skip
	 [] nil then skip
	 [] L then Lister,PrintFiles(' lost ' L) end
	 case {CondSelect PKG zombies unit}
	 of unit then skip
	 [] nil then skip
	 [] L then Lister,PrintFiles(' zombies ' L) end
	 Lister,PrintTitle('' &=)
      end

      meth PrintTitle(Text Char)
	 N = {VirtualString.length Text}
	 M = {self get_linewidth($)} - N
	 Line = {List.make M div 2}
	 for X in Line do X=Char end
	 More = {List.make M mod 2}
	 for X in More do X=Char end
      in
	 {self print(Line#Text#Line#More)}
      end

      meth PrintFiles(Title Files)
	 Lister,PrintTitle(Title &-)
	 for F in Files do {self print('    '#F)} end
      end
   end
end