%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   class AboutDialog from TkTools.dialog
      meth init(master:Master)
         TkTools.dialog,tkInit(master:  Master
			       root:    pointer
                               title:   IconName#': About'
                               buttons: ['Ok'#tkClose]
                               focus:   1
                               pack:    false
                               default: 1)
	 Title = {New Tk.label tkInit(parent: self
                                      text:
					 'This is ' # IconName #
				         ' V' # Version)}

         Author = {New Tk.label tkInit(parent: self
                                       text:   NameOfBenni # '\n' #
				               '(' # EmailOfBenni # ')')}
      in
         {Tk.send pack(Title Author side:top expand:1)}
         AboutDialog,tkPack
      end

   end

in
   
   class Dialog

      meth init
	 skip
      end
   
      meth about
         {Wait {New AboutDialog init(master:self.toplevel)}.tkClosed}
      end
   end
   
end
