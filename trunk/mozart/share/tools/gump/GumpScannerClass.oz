%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1996-1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% This defines the class `GumpScanner' which is the runtime part of
%% the Gump Scanner Generator.
%%

local
   LexBase = {Foreign.require 'tools/gump/GumpScanner.dl'
	      gump(createFromFile: 2 createFromVirtualString: 2
		   setInteractive: 2 getInteractive: 2
		   setBOL: 2 getBOL: 2
		   close: 1)}

   proc {FromFile FileName ?NewBufferState}
      NewBufferState = {LexBase.createFromFile FileName}
      case NewBufferState == 0 then
	 {Exception.raiseError gump(fileNotFound FileName)}
      else skip
      end
   end

   proc {FromVirtualString VS ?NewBufferState}
      NewBufferState = {LexBase.createFromVirtualString VS}
   end

   proc {FinalizeScanner S}
      {S close()}
   end
in
   class GumpScanner
      prop locking
      attr TokenStreamHd TokenStreamTl BufferList: nil LexerAddr: unit
      feat filenameprefix: ""

      meth init()
	 lock X in
	    {Finalize.register self FinalizeScanner}
	    TokenStreamHd <- X
	    TokenStreamTl <- X
	    GumpScanner, close()
	    LexerAddr <- {self.lexer.create}
	 end
      end
      meth putToken(Token Value) NewTl in
	 (TokenStreamTl <- NewTl) = Token#Value|NewTl
      end
      meth putToken1(Token) NewTl in
	 (TokenStreamTl <- NewTl) = Token#unit|NewTl
      end
      meth getToken(?Token ?Value)
	 case {IsFree @TokenStreamHd} then N in
	    case @BufferList == nil then
	       Token = 'EOF'
	       Value = unit
	    else
	       lock N = {self.lexer.getNextMatch @LexerAddr} end
	       {self lexExecuteAction(N)}
	       GumpScanner, getToken(?Token ?Value)
	    end
	 else NewTl in
	    (TokenStreamHd <- NewTl) = Token#Value|NewTl
	 end
      end
      meth scanFile(FileName) Buffer in
	 lock
	    Buffer = {FromFile FileName}
	    BufferList <- Buffer|@BufferList
	    {self.lexer.switchToBuffer @LexerAddr Buffer}
	 end
      end
      meth scanVirtualString(VS) Buffer in
	 lock
	    Buffer = {FromVirtualString VS}
	    BufferList <- Buffer|@BufferList
	    {self.lexer.switchToBuffer @LexerAddr Buffer}
	 end
      end
      meth closeBuffer()
	 lock
	    case @BufferList of Buffer|BufferRest then
	       BufferList <- BufferRest
	       case BufferRest of NewBuffer|_ then
		  {self.lexer.switchToBuffer @LexerAddr NewBuffer}
	       else skip
	       end
	       {LexBase.close Buffer}
	    else skip
	    end
	 end
      end
      meth getAtom($)
	 lock {self.lexer.getAtom @LexerAddr $} end
      end
      meth getString($)
	 lock {self.lexer.getString @LexerAddr $} end
      end
      meth getLength($)
	 lock {self.lexer.getLength @LexerAddr $} end
      end
      meth setMode(Mode)
	 lock {self.lexer.setMode @LexerAddr Mode} end
      end
      meth currentMode($)
	 lock {self.lexer.currentMode @LexerAddr $} end
      end
      meth input($)
	 lock {self.lexer.input @LexerAddr $} end
      end
      meth unput(C)
	 lock {self.lexer.unput @LexerAddr C} end
      end

      meth setInteractive(B)
	 lock
	    case @BufferList of Buffer|_ then
	       {LexBase.setInteractive Buffer case B then 1 else 0 end}
	    else skip
	    end
	 end
      end
      meth getInteractive(?B)
	 lock
	    case @BufferList of Buffer|_ then
	       B = {LexBase.getInteractive Buffer} \= 0
	    else
	       B = false
	    end
	 end
      end
      meth setBOL(B)
	 lock
	    case @BufferList of Buffer|_ then
	       {LexBase.setBOL Buffer case B then 1 else 0 end}
	    else skip
	    end
	 end
      end
      meth getBOL(?B)
	 lock
	    case @BufferList of Buffer|_ then
	       B = {LexBase.getBOL Buffer} \= 0
	    else
	       B = false
	    end
	 end
      end

      meth close()
	 lock
	    {ForAll @BufferList proc {$ B} {LexBase.close B} end}
	    BufferList <- nil
	    case @LexerAddr == unit then skip
	    else
	       {self.lexer.delete @LexerAddr}
	       LexerAddr <- unit
	    end
	 end
      end
   end
end
