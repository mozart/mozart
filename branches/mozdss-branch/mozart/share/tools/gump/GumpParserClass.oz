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
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% This defines the class `GumpParser' which is the runtime part of
%% the Gump Parser Generator.
%%

local
   YYTERROR = 1
   YYEMPTY = {NewName}
   SynAccepted = {NewName}
   SynAborted = {NewName}
   SynUserError = {NewName}
in
   class GumpParser
      % Features required in derived classes:
      %    synFLAG (YYFLAG)
      %    synFINAL (YYFINAL)
      %    synLAST (YYLAST)
      %    synCheck (yycheck)
      %    synTable (yytable)
      %    synPAct (yypact)
      %    synDefAct (yydefact)
      %    synPGoto (yypgoto)
      %    synDefGoto (yydefgoto)
      %    synR1 (yyr1)
      %    synR2 (yyr2)
      %    synTranslate (yytranslate/YYTRANSLATE)
      %    synTokenNames (yytname)
      %    synNTOKENS (YYNTOKENS)
      %    synNNTS (YYNNTS)
      %    synStartSymbols

      feat noLookahead: YYEMPTY MyScanner
      attr
	 lookaheadSymbol % lookahead symbol
	 lookaheadValue  % semantic value of lookahead symbol

	 Yychar1     % lookahead symbol as an internal (translated) token number
	 Yyss        % the state stack
	 Yyvs        % the semantic value stack
	 Yystate     % the state the parser currently is in
	 Yyn         % denote the current parse action
	 Yyerrstatus % number of tokens to shift before resuming error reporting

      %-----------------------------------------------------------------
      % User Functionality

      meth init(Scanner)
	 self.MyScanner = Scanner
      end
      meth getScanner($)
	 self.MyScanner
      end

      meth parse(Res ?Status) Token Descr Parameters Is0 Ss0 Is Ss in
	 Token#Descr = self.synStartSymbols.{Label Res}
	 Parameters = {Arity Descr} = {Arity Res}
	 Is0#Ss0 = {FoldL Parameters
		    fun {$ Is#Ss F}
		       case Descr.F of inherited then (Res.F|Is)#Ss
		       [] synthesized then Is#(Res.F|Ss)
		       end
		    end nil#nil}
	 Is = case Is0 of [X] then X else {List.toTuple '#' Is0} end
	 Ss = case Ss0 of [X] then X else {List.toTuple '#' Ss0} end
	 {self.MyScanner putToken(Token Is)}
	 lookaheadSymbol <- YYEMPTY
	 Yyss <- nil
	 Yyvs <- ['#']
	 Yyerrstatus <- 0
	 Yystate <- 0
	 try
	    GumpParser, Yynewstate()   % this always ends with an exception
	 catch !SynAccepted then
	    @Yyvs = _|Ss|_   % the topmost element is the `$' symbol
	    Status = true
	 [] !SynAborted then
	    Status = false
	 end
      end
      meth error(VS)
	 {System.showError VS}
      end

      %-----------------------------------------------------------------
      % User Methods for Automaton Interaction

      meth accept()
	 raise SynAccepted end
      end
      meth abort()
	 raise SynAborted end
      end
      meth raiseError()
	 raise SynUserError end
      end
      meth errorOK()
	 Yyerrstatus <- 0
      end
      meth clearLookahead()
	 if @lookaheadSymbol \= 'EOF' then
	    lookaheadSymbol <- YYEMPTY
	 end
      end

      %-----------------------------------------------------------------
      % Private Methods

      meth Yynewstate()
	 % Push a new state, which is found in Yystate.
	 % In all cases, when getting here, the value stack has just been
	 % pushed, so pushing a state here evens the stacks.
	 Yyss <- @Yystate|@Yyss
	 % Do appropriate processing given the current state.
	 % First try to decide what to do without reference to lookahead token:
	 Yyn <- self.synPAct.@Yystate
	 if @Yyn == self.synFLAG then
	    GumpParser, Yydefault()
	 else
	    % Not known => get a lookahead token if don't already have one:
	    if @lookaheadSymbol == YYEMPTY then Token Value in
	       {self.MyScanner getToken(?Token ?Value)}
	       lookaheadSymbol <- Token
	       lookaheadValue <- Value
	       % Convert to internal form (in Yychar1) for indexing tables with:
	       Yychar1 <- {CondSelect self.synTranslate @lookaheadSymbol 2}
	    else skip
	    end

	    Yyn <- @Yyn + @Yychar1
	    if @Yyn < 0 orelse @Yyn > self.synLAST then
	       GumpParser, Yydefault()
	    elseif self.synCheck.@Yyn \= @Yychar1 then
	       GumpParser, Yydefault()
	    else
	       Yyn <- self.synTable.@Yyn
	       % Yyn is what to do for this token type in this state.
	       % Negative => reduce, -Yyn is rule number.
	       % Positive => shift, Yyn is new state.
	       % If new state is final state => no shift, just return success.
	       % 0, or most negative number => error.
	       if @Yyn == 0 orelse @Yyn == self.synFLAG then
		  GumpParser, Yyerrlab()
	       elseif @Yyn < 0 then
		  Yyn <- ~@Yyn
		  GumpParser, Yyreduce()
	       elseif @Yyn == self.synFINAL then
		  GumpParser, accept()
	       else   % shift the lookahead token and enter state Yyn
		  Yyvs <- @lookaheadValue|@Yyvs
		  if @lookaheadSymbol \= 'EOF' then
		     lookaheadSymbol <- YYEMPTY
		  end

		  % after 3 tokens shifted since error, turn off error status:
		  if @Yyerrstatus \= 0 then
		     Yyerrstatus <- @Yyerrstatus - 1
		  end

		  Yystate <- @Yyn
		  GumpParser, Yynewstate()
	       end
	    end
	 end
      end
      meth Yydefault()
	 % do the default action for the current state
	 Yyn <- self.synDefAct.@Yystate
	 if @Yyn == 0 then
	    GumpParser, Yyerrlab()
	 else
	    GumpParser, Yyreduce()
	 end
      end
      meth Yyreduce() Yyval NewYyvs UserError in
	 % do a reduction with the rule given by Yyn
	 try
	    Yyval = {self synExecuteAction(@Yyn @Yyvs ?NewYyvs $)}
	    UserError = false
	 catch !SynUserError then
	    UserError = true
	 end
	 if UserError then
	    GumpParser, Yyerrlab1()
	 else
	    Yyvs <- Yyval|NewYyvs
	    Yyss <- {List.drop @Yyss self.synR2.@Yyn}

	    % determine what state to go to, based on the state we popped
	    % back to and the symbol number reduced to:
	    Yyn <- self.synR1.@Yyn
	    Yystate <- self.synPGoto.@Yyn + @Yyss.1
	    if @Yystate < 0 orelse @Yystate > self.synLAST then
	       Yystate <- self.synDefGoto.@Yyn
	    elseif self.synCheck.@Yystate \= @Yyss.1 then
	       Yystate <- self.synDefGoto.@Yyn
	    else
	      Yystate <- self.synTable.@Yystate
	    end
	    GumpParser, Yynewstate()
	 end
      end
      meth Yyerrlab()
	 % here on detecting an error
	 if @Yyerrstatus == 0 then   % if not already recovering from an error
	    N = self.synPAct.@Yystate in
	    if N > self.synFLAG andthen N < self.synLAST then Expected Msg in
	       Expected =
	       {ForThread if N < 0 then ~N else 0 end
		self.synNTOKENS + self.synNNTS - 1 1
		fun {$ In X}
		   if {CondSelect self.synCheck X + N ~1} == X then
		      (self.synTokenNames.X)|In
		   else In
		   end
		end nil}
	       Msg =
	       case Expected of T|Tr then
		  {FoldL Tr fun {$ In T} In#' or `'#T#'\'' end
		   'parse error, expecting `'#T#'\''}
	       [] nil then 'parse error'
	       end
	       {self error(Msg)}
	    else
	       {self error('parse error')}
	    end
	 else skip
	 end
	 GumpParser, Yyerrlab1()
      end
      meth Yyerrlab1()
	 % here on error raised explicitly by an action
	 if @Yyerrstatus == 3 then
	    % just tried and failed to reuse lookahead token after an error,
	    % discard it:
	    if @lookaheadSymbol == 'EOF' then
	       GumpParser, abort()
	    else
	       lookaheadSymbol <- YYEMPTY
	       GumpParser, Yyerrhandle()
	    end
	 else   % try to reuse lookahead token after shifting the error token
	    Yyerrstatus <- 3   % each real token shifted decrements this
	    GumpParser, Yyerrhandle()
	 end
      end
      meth Yyerrhandle()
	 Yyn <- self.synPAct.@Yystate
	 if @Yyn == self.synFLAG then
	    GumpParser, Yyerrdefault()
	 else
	    Yyn <- @Yyn + YYTERROR
	    if @Yyn < 0 orelse @Yyn > self.synLAST then
	       GumpParser, Yyerrdefault()
	    elseif self.synCheck.@Yyn \= YYTERROR then
	       GumpParser, Yyerrdefault()
	    else
	       Yyn <- self.synTable.@Yyn
	       if @Yyn == 0 orelse @Yyn == self.synFLAG then
		  GumpParser, Yyerrdefault
	       elseif @Yyn < 0 then
		  Yyn <- ~@Yyn
		  GumpParser, Yyreduce()
	       elseif @Yyn == self.synFINAL then
		  GumpParser, accept()
	       else
		  Yyvs <- @lookaheadValue|@Yyvs
		  Yystate <- @Yyn
		  GumpParser, Yynewstate()
	       end
	    end
	 end
      end
      meth Yyerrdefault()
	 % current state does not do anything special for the error token:
	 % pop the current state because it cannot handle the error token
	 case @Yyss of nil then
	    GumpParser, abort()
	 [] State|Rest then
	    Yystate <- State
	    Yyss <- Rest
	    Yyvs <- @Yyvs.2
	    GumpParser, Yyerrhandle()
	 end
      end
   end
end
