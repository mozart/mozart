functor
import
   System(showInfo)
   Syslet(spec args exit)
   FileOperations(countLines: WC)
define
   Syslet.spec = plain
   case Syslet.args of nil then
      {System.showInfo 'Usage: wc <file> ...'}
      {Syslet.exit 2}
   else
      {ForAll Syslet.args
       proc {$ FileName}
          try
             {System.showInfo FileName#'\t'#{WC FileName}}
          catch _ then
             {System.showInfo 'wc: could not open file "'#FileName#'"'}
             {Syslet.exit 1}
          end
       end}
      {Syslet.exit 0}
   end
end
