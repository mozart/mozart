functor
export
   Help
require
   Utils at 'Utils.ozf'
prepare
   HELP = {Utils.slurpFile 'HELP.txt'}
import
   System(showError:Print)
define
   proc {Help} {Print HELP} end
end
