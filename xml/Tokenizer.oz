functor
export
   NewFromString
   NewFromURL
   NewLazyFromURL
prepare

   %% ================================================================
   %% {IsNameChar C}
   %%     returns true iff C may legally occur in an XML name
   %% ================================================================
   
   CharIsAlNum = Char.isAlNum
   fun {IsNameChar C}
      {CharIsAlNum C} orelse
      C==&-           orelse
      C==&_           orelse
      C==&.           orelse
      C==&:
   end

   %% ================================================================
   %% {ScanSpaces +Sin ?Sout}
   %%     skips leading spaces in Sin and returns the tail which is
   %% either empty or begins with a non-space character
   %% ================================================================

   CharIsSpace = Char.isSpace
   fun {ScanSpaces S}
      case S
      of H|T andthen {CharIsSpace H} then {ScanSpaces T}
      else S end
   end

   %% ================================================================
   %% {ScanEntityRef +Sin ?C +Sout}
   %%     We have just encountered a `&´ and now we must recognize the
   %% entity reference.  We only support entity references that denote
   %% single characters: thus output C is the corresponding character.
   %% ================================================================

   fun {ScanEntityRef L H}
      case L
      of    &a|&m|&p|&;|T then H=&& T
      []       &l|&t|&;|T then H=&< T
      []       &g|&t|&;|T then H=&> T
      [] &a|&p|&o|&s|&;|T then H=&' T
      [] &q|&u|&o|&t|&;|T then H=&" T
      []          &#|&x|T then {ScanCharRefHex T 0 H}
      []             &#|T then {ScanCharRefDec T 0 H}
      else
	 {Exception.raiseError
	  xml(tokenize(scanEntityRef L))}
	 unit
      end
   end

   fun {ScanCharRefDec L I H}
      case L
      of &0|T then {ScanCharRefDec T I*10   H}
      [] &1|T then {ScanCharRefDec T I*10+1 H}
      [] &2|T then {ScanCharRefDec T I*10+2 H}
      [] &3|T then {ScanCharRefDec T I*10+3 H}
      [] &4|T then {ScanCharRefDec T I*10+4 H}
      [] &5|T then {ScanCharRefDec T I*10+5 H}
      [] &6|T then {ScanCharRefDec T I*10+6 H}
      [] &7|T then {ScanCharRefDec T I*10+7 H}
      [] &8|T then {ScanCharRefDec T I*10+8 H}
      [] &9|T then {ScanCharRefDec T I*10+9 H}
      [] &;|T then H=I T
      else
	 {Exception.raiseError
	  xml(tokenize(scanCharRefDec L))}
	 unit
      end
   end

   fun {ScanCharRefHex L I H}
      case L
      of &0|T then {ScanCharRefHex T I*16    H}
      [] &1|T then {ScanCharRefHex T I*16+1  H}
      [] &2|T then {ScanCharRefHex T I*16+2  H}
      [] &3|T then {ScanCharRefHex T I*16+3  H}
      [] &4|T then {ScanCharRefHex T I*16+4  H}
      [] &5|T then {ScanCharRefHex T I*16+5  H}
      [] &6|T then {ScanCharRefHex T I*16+6  H}
      [] &7|T then {ScanCharRefHex T I*16+7  H}
      [] &8|T then {ScanCharRefHex T I*16+8  H}
      [] &9|T then {ScanCharRefHex T I*16+9  H}
      [] &a|T then {ScanCharRefHex T I*16+10 H}
      [] &b|T then {ScanCharRefHex T I*16+11 H}
      [] &c|T then {ScanCharRefHex T I*16+12 H}
      [] &d|T then {ScanCharRefHex T I*16+13 H}
      [] &e|T then {ScanCharRefHex T I*16+14 H}
      [] &f|T then {ScanCharRefHex T I*16+15 H}
      [] &A|T then {ScanCharRefHex T I*16+10 H}
      [] &B|T then {ScanCharRefHex T I*16+11 H}
      [] &C|T then {ScanCharRefHex T I*16+12 H}
      [] &D|T then {ScanCharRefHex T I*16+13 H}
      [] &E|T then {ScanCharRefHex T I*16+14 H}
      [] &F|T then {ScanCharRefHex T I*16+15 H}
      [] &;|T then H=I T
      else
	 {Exception.raiseError
	  xml(tokenize(scanCharRefHex L))}
	 unit
      end
   end


   %% ================================================================
   %% {ScanValue +Sin ?Val ?Sout}
   %%     Scans a quoted attribute value
   %% ================================================================

   local
      fun {Loop L C Val}
	 case L
	 of &&|T then H Val2 in
	    Val=H|Val2
	    {Loop {ScanEntityRef T H} C Val2}
	 [] H|T then
	    if H==C then Val=nil T
	    else Val2 in Val=H|Val2 {Loop T C Val2} end
	 end
      end
   in
      fun {ScanValue Sin Val}
	 case Sin
	 of &"|T then {Loop T &" Val}
	 [] &'|T then {Loop T &' Val}
	 else
	    {Exception.raiseError
	     xml(tokenize(scanValue Sin))}
	    unit
	 end
      end
   end

   %% ================================================================
   %% {ScanName +Sin ?Name ?Sout}
   %%     reads an XML name (identifier).
   %% ================================================================

   fun {ScanName Sin Name}
      case Sin
      of H|T andthen {IsNameChar H} then Name2 in
	 Name=H|Name2 {ScanName T Name2}
      else Name=nil Sin end
   end

   %% ================================================================
   %% {ScanAttr +Sin ?Attr ?Sout}
   %%     scans an attribute declaration Name=Value and returns Attr
   %% of the form Name|Value.
   %% ================================================================

   fun {ScanAttr Sin Attr}
      Name Value
   in
      Attr=Name|Value
      case {ScanSpaces {ScanName Sin Name}}
      of &=|T then {ScanValue {ScanSpaces T} Value}
      []    T then {Exception.raiseError
		    xml(tokenize(scanAttr T))}
	 unit
      end
   end

   %% ================================================================
   %% {ScanEtag +Sin ?Tag ?Sout}
   %%     scans an end tag. returns Tag=etag(Name) where Name is the
   %% name of the tag.
   %% ================================================================

   fun {ScanEtag Sin Tag}
      Name
   in
      Tag=etag(Name)
      case {ScanSpaces {ScanName Sin Name}}
      of &>|T then T
      [] T then
	 {Exception.raiseError
	  xml(tokenize(scanEtag T))}
	 unit
      end
   end

   %% ================================================================
   %% {ScanStag +Sin ?Tag ?Sout}
   %%     scans a start tag and returns Tag=stag(Name Alist Empty)
   %% where Name is the name of the tag, Alist is the list of
   %% attributes, and Empty is a boolean indicating whether the
   %% element is empty (i.e. if the tag ends with `/>')
   %% ================================================================

   fun {ScanStag Sin Tag}
      Name Alist Empty
   in
      Tag=stag(Name Alist Empty)
      {ScanAlist {ScanName Sin Name} Alist Empty}
   end

   %% ================================================================
   %% {ScanAlist +Sin ?Alist ?Empty ?Sout}
   %%     scans the attribute declarations on a start tag.  returns
   %% Alist, the list of attribute declarations (where each is of the
   %% form Name|Value), and Empty (see above).
   %% ================================================================

   fun {ScanAlist Sin Alist Empty}
      case {ScanSpaces Sin}
      of &/|&>|T then Alist=nil Empty=true  T
      []    &>|T then Alist=nil Empty=false T
      []       T then Alist2 Attr in
	 Alist=Attr|Alist2
	 {ScanAlist {ScanAttr T Attr} Alist2 Empty}
      end
   end

   %% ================================================================
   %% {ScanText +Sin ?Text ?Sout}
   %%     scans character data and returns Text=text(Chars) where
   %% Chars is the data with entities and CDATA sections expanded.
   %% ================================================================

   local
      fun {Loop Sin Chars}
	 case Sin
	 of &&|T then H Chars2 in
	    Chars=H|Chars2
	    {Loop {ScanEntityRef T H} Chars2}
	 [] &<|&!|&[|T then {CDATA T Chars}
	 [] &<|_ then Chars=nil Sin
	 [] H|T  then Chars2 in
	    Chars=H|Chars2 {Loop T Chars2}
	 [] nil  then Chars=nil nil
	 end
      end
      fun {CDATA Sin Chars}
	 case {ScanSpaces Sin}
	 of &C|&D|&A|&T|&A|T then
	    case {ScanSpaces T}
	    of &[|T then {CDATALoop T Chars}
	    else {Exception.raiseError
		  xml(tokenize(cdata1 Sin))}
	       unit
	    end
	 else {Exception.raiseError
	       xml(tokenize(cdata2 Sin))}
	    unit
	 end
      end
      fun {CDATALoop Sin Chars}
	 case Sin
	 of &]|&]|&>|T then {Loop T Chars}
	 [] H|T then Chars2 in
	    Chars=H|Chars2 {CDATALoop T Chars2}
	 else {Exception.raiseError
	       xml(tokenize(cdata3))}
	    unit
	 end
      end
   in
      fun {ScanText Sin Text}
	 Chars
      in
	 Text=text(Chars)
	 {Loop Sin Chars}
      end
   end

   %% ================================================================
   %% {ScanComment +Sin ?Comment ?Sout}
   %%     we have just read `<!--' and we must now read up to the
   %% closing `-->'.  Returns Comment=comment(Chars) where Chars is
   %% the text of the comment.
   %% ================================================================

   local
      fun {Loop Sin Chars}
	 case Sin
	 of &-|&-|&>|T then Chars=nil T
	 [] H|T then Chars2 in
	    Chars=H|Chars2 {Loop T Chars2}
	 else
	    {Exception.raiseError
	     xml(tokenize(scanComment))}
	    unit
	 end
      end
   in
      fun {ScanComment Sin Comment}
	 Chars
      in
	 Comment=comment(Chars)
	 {Loop Sin Chars}
      end
   end

   %% ================================================================
   %% {ScanPI +Sin ?PI ?Sout}
   %%     we have just read `<?' indicating the start of a processing
   %% instruction, we must now read the name of the target and then
   %% its arguments up to the closing `?>'.  Returns PI=pi(Name Args)
   %% where Name is the name of the pi's target and Args is its
   %% argument text.
   %% ================================================================

   local
      fun {Loop Sin Args}
	 case Sin
	 of &?|&>|T then Args=nil T
	 [] H|T then Args2 in
	    Args=H|Args2 {Loop T Args2}
	 else
	    {Exception.raiseError
	     xml(tokenize(scanPI))}
	    unit
	 end
      end
   in
      fun {ScanPI Sin PI}
	 Name Args
      in
	 PI=pi(Name Args)
	 {Loop {ScanName Sin Name} Args}
      end
   end

   %% ================================================================
   %% {ScanNext +Sin ?Token ?Sout}
   %%     scans the next token.  Returns unit if Sin is empty.
   %% ================================================================

   fun {ScanNext Sin Token}
      case Sin
      of nil           then Token=unit nil
      [] &<|&/|T       then {ScanEtag    T Token}
      [] &<|&!|&-|&-|T then {ScanComment T Token}
      [] &<|&?|T       then {ScanPI      T Token}
      [] &<|&!|&[|_    then {ScanText  Sin Token}
      [] &<|T          then {ScanStag    T Token}
      else                  {ScanText  Sin Token} end
   end

   %% ================================================================
   %% {New +String ?Tokenizer}
   %%     A tokenizer is an ADT that exports a `get' function which
   %% returns the next token in the input string.  The tokenizer is
   %% initialized with input string String.  `get' returns unit when
   %% the input has been exhausted.
   %% ================================================================

   fun {NewFromString String}
      C={NewCell String}
      fun {Get}
	 OldString NewString
      in
	 {Exchange C OldString NewString}
	 {ScanNext OldString $ NewString}
      end
   in
      tokenizer(
	 get : Get)
   end

import
   Open(file:File)
define

   %% ================================================================
   %% {NewFromURL     +URL ?Tokenizer}
   %% {NewLazyFromURL +URL ?Tokenizer}
   %%     take input from the given URL.  There is a lazy version
   %% which reads input in a demand driven fashion; this saves memory.
   %% ================================================================

   fun {NewFromURL U}
      O={New File init(url:U)}
   in
      try {NewFromString
	   {O read(list:$ size:all)}}
      finally
	 try {O close} catch _ then skip end
      end
   end

   fun {NewLazyFromURL U}
      O={New File init(url:U)}
      fun lazy {More} L T N in
	 {O read(list:L tail:T len:N)}
	 T = if N==0 then {O close} nil
	     else {More} end
	 L
      end
   in
      {NewFromString {More}}
   end
end
