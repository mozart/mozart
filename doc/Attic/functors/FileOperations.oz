functor
import
   Open(text file)
export
   CountLines
define
   class TextFile from Open.file Open.text
      prop final
   end

   fun {CountLinesSub File N}
      case {File getS($)} of false then N
      else {CountLinesSub File N + 1}
      end
   end

   fun {CountLines FileName}
      File = {New TextFile init(name: FileName flags: [read])}
      Res = {CountLinesSub File 0}
   in
      {File close()}
      Res
   end
end
