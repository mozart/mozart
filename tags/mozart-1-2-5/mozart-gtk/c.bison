/*
 * Author:
 *   Thorsten Brunklaus <bruni@ps.uni-sb.de>
 *
 * Copyright:
 *   Thorsten Brunklaus, 2001
 *
 * Last Change:
 *   $Date$ by $Author$
 *   $Revision$
 *
 * This file is part of Mozart, an implementation of Oz 3:
 *   http://www.mozart-oz.org
 *
 * See the file "LICENSE" or
 *   http://www.mozart-oz.org/LICENSE.html
 * for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

%{
#include <setjmp.h>
#include <string.h>
#include <mozart.h>
#include "parse.h"

static OZ_Term type_list;
static OZ_Term anchor;
static jmp_buf error_buf;

static int atomEq(OZ_Term a, const char *b) {
  return (OZ_eq(a, OZ_atom(b)));
}

int lookupType(OZ_Term id) {
  OZ_Term cons = type_list;

  while (OZ_isCons(cons)) {
    if (OZ_eq(OZ_head(cons), id)) {
      return 1;
    }
    cons = OZ_tail(cons);
  }
  return 0;
}

#if defined(DEBUG)
static int isTypedef(OZ_Term id) {
  fprintf(stderr,"isTypedef: %s\n", OZ_toC(id, 10, 10));
  if (OZ_isTuple(id)) {
      fprintf(stderr,"isTypedef: checking for stor_spec/decl_spec\n");
    if (atomEq(OZ_label(id), "stor_spec/decl_spec")) {
      if (atomEq(OZ_getArg(id, 0), "typedef")) {
        fprintf(stderr,"isTypedef: found typedef\n");
        return 1;
      }
    }
  }
  fprintf(stderr,"isTypedef: Hmm, no typedef\n");
  return 0;
}
#else
static int isTypedef(OZ_Term id) {
  if (OZ_isTuple(id)) {
    if (atomEq(OZ_label(id), "stor_spec/decl_spec")) {
      return (atomEq(OZ_getArg(id, 0), "typedef"));
    }
  }
  return 0;
}
#endif

static OZ_Term getTypeName(OZ_Term id) {
#if defined(DEBUG)
  fprintf(stderr,"getTypeName: %s\n", OZ_toC(id, 10, 10));
#endif
  if (OZ_isAtom(id)) {
    return id;
  }
  else {
    OZ_Term l = OZ_label(id);

    if (atomEq(l, "pointer decl")) {
      return getTypeName(OZ_getArg(id, 1));
    }
    else if (atomEq(l, "decla(pars)")) {
      return getTypeName(OZ_getArg(id, 0));
    }
    else if (atomEq(l, "decla()")) {
      return getTypeName(OZ_getArg(id, 0));
    }
    else {
      return OZ_atom("unkown type name");
    }
  }
}

#if defined(DEBUG)
static void verifyTypeList() {
  OZ_Term cons = type_list;

  while (OZ_isCons(cons)) {
    if (atomEq(OZ_head(cons), "unknown type name")) {
      fprintf(stderr,"verifyTypeList: type_list has unknown types; check it\n");
    }
    cons = OZ_tail(cons);
  }
  fprintf (stderr,"verifyTypeList: type_list is clean\n");
}
#endif

int line_num = 1;

%}

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO INLINE RESTRICT REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token SEPARATOR OBRACE CBRACE COMMA COLON EQUAL_OP OPARENT CPARENT
%token OBRACKET CBRACKET DOT ADDR_OP DEREF_OP TILDE SUB_OP PLUS_OP MUL_OP DIV_OP
%token MOD_OP LESS_OP GREATER_OP ROOF PIPE QUESTIONMARK

%start start

%%

primary_expr
	: identifier {
		$$ = $1;
	}
	| CONSTANT {
		$$ = $1;
	}
	| STRING_LITERAL {
		$$ = $1;
	}
	| OPARENT expr CPARENT {
		$$ = $2;
	}
	;

postfix_expr
	: primary_expr {
		$$ = $1;
	}
	| postfix_expr OBRACKET expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("expr[expr]"), 2, $1, $3);
	}
	| postfix_expr OPARENT CPARENT {
		$$ = OZ_mkTuple(OZ_atom("expr()"), 1, $1); 
	}
	| postfix_expr OPARENT argument_expr_list CPARENT {
		$$ = OZ_mkTuple(OZ_atom("expr(expr_list)"), 2, $1, $3);
	}
	| postfix_expr DOT identifier {
		$$ = OZ_mkTuple(OZ_atom("expr.id"), 2, $1, $3);
	}
	| postfix_expr PTR_OP identifier {
		$$ = OZ_mkTuple(OZ_atom("expr->id"), 2, $1, $3);
	}
	| postfix_expr INC_OP {
		$$ = OZ_mkTuple(OZ_atom("expr++"), 1, $1);
	}
	| postfix_expr DEC_OP {
		$$ = OZ_mkTuple(OZ_atom("expr--"), 1, $1);
	}
	| OPARENT type_name CPARENT OBRACE initializer_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("(type_name){init_list}"), 2, $2, $5);
	}
	| OPARENT type_name CPARENT OBRACE initializer_list COMMA CBRACE {
		$$ = OZ_mkTuple(OZ_atom("(type_name){init_list},"), 2, $2, $5);
	}
	;

argument_expr_list
	: assignment_expr {
		$$ = $1;
	}
	| argument_expr_list COMMA assignment_expr {
		$$ = OZ_mkTuple(OZ_atom("arg_expr_list, ass_expr"), 2, $1, $3);
	}
	;

unary_expr
	: postfix_expr {
		$$ = $1;
	}
	| INC_OP unary_expr {
		$$ = OZ_mkTuple(OZ_atom("++expr"), 1, $2);
	}
	| DEC_OP unary_expr {
		$$ = OZ_mkTuple(OZ_atom("--expr"), 1, $2);
	}
	| unary_operator cast_expr {
		$$ = OZ_mkTuple(OZ_atom("un_op cast_exp"), 2, $1, $2);
	}
	| SIZEOF unary_expr {
		$$ = OZ_mkTuple(OZ_atom("sizeof(expr)"), 1, $2);
	}
	| SIZEOF OPARENT type_name CPARENT {
		$$ = OZ_mkTuple(OZ_atom("sizeof(type)"), 1, $3);
	}
	;

unary_operator
	: ADDR_OP {
		$$ = $1;
	}
	| MUL_OP {
		$$ = $1;
	}
	| PLUS_OP {
		$$ = $1;
	}
	| SUB_OP {
		$$ = $1;
	}
	| TILDE {
		$$ = $1;
	}
	| DEREF_OP {
		$$ = $1;
	}
	;

cast_expr
	: unary_expr {
		$$ = $1;
	}
	| OPARENT type_name CPARENT cast_expr {
		$$ = OZ_mkTuple(OZ_atom("(type)cast_expr"), 2, $2, $4);
	}
	;

multiplicative_expr
	: cast_expr {
		$$ = $1;
	}
	| multiplicative_expr MUL_OP cast_expr {
		$$ = OZ_mkTuple(OZ_atom("mul(expr, expr)"), 2, $1, $3);
	}
	| multiplicative_expr DIV_OP cast_expr {
		$$ = OZ_mkTuple(OZ_atom("div(expr, expr)"), 2, $1, $3);
	}
	| multiplicative_expr MOD_OP cast_expr {
		$$ = OZ_mkTuple(OZ_atom("mod(expr, expr)"), 2, $1, $3);
	}
	;

additive_expr
	: multiplicative_expr {
		$$ = $1;
	}
	| additive_expr PLUS_OP multiplicative_expr {
		$$ = OZ_mkTuple(OZ_atom("plus(expr, expr)"), 2, $1, $3);
	}
	| additive_expr SUB_OP multiplicative_expr {
		$$ = OZ_mkTuple(OZ_atom("minus(expr, expr)"), 2, $1, $3);
	}
	;

shift_expr
	: additive_expr {
		$$ = $1;
	}
	| shift_expr LEFT_OP additive_expr {
		$$ = OZ_mkTuple(OZ_atom("<<(expr, expr)"), 2, $1, $3);
	}
	| shift_expr RIGHT_OP additive_expr {
		$$ = OZ_mkTuple(OZ_atom(">>(expr, expr)"), 2, $1, $3);
	}
	;

relational_expr
	: shift_expr {
		$$ = $1;
	}
	| relational_expr LESS_OP shift_expr {
		$$ = OZ_mkTuple(OZ_atom("<(expr, expr)"), 2, $1, $3);
	}
	| relational_expr GREATER_OP shift_expr {
		$$ = OZ_mkTuple(OZ_atom(">(expr, expr)"), 2, $1, $3);
	}
	| relational_expr LE_OP shift_expr {
		$$ = OZ_mkTuple(OZ_atom("<=(expr, expr)"), 2, $1, $3);
	}
	| relational_expr GE_OP shift_expr {
		$$ = OZ_mkTuple(OZ_atom(">=(expr, expr)"), 2, $1, $3);
	}
	;

equality_expr
	: relational_expr {
		$$ = $1;
	}
	| equality_expr EQ_OP relational_expr {
		$$ = OZ_mkTuple(OZ_atom("==(expr, expr)"), 2, $1, $3);
	}
	| equality_expr NE_OP relational_expr {
		$$ = OZ_mkTuple(OZ_atom("!=(expr, expr)"), 2, $1, $3);
	}
	;

and_expr
	: equality_expr {
		$$ = $1;
	}
	| and_expr ADDR_OP equality_expr {
		$$ = OZ_mkTuple(OZ_atom("bitand(expr, expr)"), 2, $1, $3);
	}
	;

exclusive_or_expr
	: and_expr {
		$$ = $1;
	}
	| exclusive_or_expr ROOF and_expr {
		$$ = OZ_mkTuple(OZ_atom("exor(expr, expr)"), 2, $1, $3);
	}
	;

inclusive_or_expr
	: exclusive_or_expr {
		$$ = $1;
	}
	| inclusive_or_expr PIPE exclusive_or_expr {
		$$ = OZ_mkTuple(OZ_atom("inclor(expr, expr)"), 2, $1, $3);
	}
	;

logical_and_expr
	: inclusive_or_expr {
		$$ = $1;
	}
	| logical_and_expr AND_OP inclusive_or_expr {
		$$ = OZ_mkTuple(OZ_atom("logand(expr, expr)"), 2, $1, $3);
	}
	;

logical_or_expr
	: logical_and_expr {
		$$ = $1;
	}
	| logical_or_expr OR_OP logical_and_expr {
		$$ = OZ_mkTuple(OZ_atom("logor(expr, expr)"), 2, $1, $3);
	}
	;

conditional_expr
	: logical_or_expr {
		$$ = $1;
	}
	| logical_or_expr QUESTIONMARK logical_or_expr COLON conditional_expr {
		$$ = OZ_mkTuple(OZ_atom("quest_if(expr, expr, expr)"), 3, $1, $3, $5);
	}
	;

assignment_expr
	: conditional_expr {
		$$ = $1;
	}
	| unary_expr assignment_operator assignment_expr {
		$$ = OZ_mkTuple(OZ_atom("ass_expr(expr, op, expr)"), 3, $1, $2, $3);
	}
	;

assignment_operator
	: EQUAL_OP {
		$$ = $1;
	}
	| MUL_ASSIGN {
		$$ = $1;
	}
	| DIV_ASSIGN {
		$$ = $1;
	}
	| MOD_ASSIGN {
		$$ = $1;
	}
	| ADD_ASSIGN {
		$$ = $1;
	}
	| SUB_ASSIGN {
		$$ = $1;
	}
	| LEFT_ASSIGN {
		$$ = $1;
	}
	| RIGHT_ASSIGN {
		$$ = $1;
	}
	| AND_ASSIGN {
		$$ = $1;
	}
	| XOR_ASSIGN {
		$$ = $1;
	}
	| OR_ASSIGN {
		$$ = $1;
	}
	;

expr
	: assignment_expr {
		$$ = $1;
	}
	| expr COMMA assignment_expr {
		$$ = OZ_mkTuple(OZ_atom("expr, ass_expr"), 2, $1, $3);
	}
	;

constant_expr
	: conditional_expr {
		$$ = $1;
	}
	;

declaration
	: declaration_specifiers SEPARATOR {
		$$ = $1;
	}
	| declaration_specifiers init_declarator_list SEPARATOR {
		if (isTypedef($1)) {
			type_list = OZ_cons(getTypeName($2), type_list);
                }
		$$ = OZ_mkTuple(OZ_atom("decl_spec/init_decls"), 2, $1, $2);
	}
	;

declaration_specifiers
	: storage_class_specifier {
		$$ = $1;
	}
	| storage_class_specifier declaration_specifiers {
		$$ = OZ_mkTuple(OZ_atom("stor_spec/decl_spec"), 2, $1, $2);
	}
	| type_qualifier_specifier {
		$$ = $1;
        }
	| type_qualifier_specifier declaration_specifiers {
		$$ = OZ_mkTuple(OZ_atom("type_spec/decl_spec"), 2, $1, $2);
	}
	| function_specifier {
		$$ = $1;
	}
	| function_specifier declaration_specifiers {
		$$ = OZ_mkTuple(OZ_atom("func_spec/decl_spec"), 2, $1, $2);
	}
	;

type_qualifier_specifier
	: type_specifier {
		$$ = $1;
	}
	| type_qualifier {
		$$ = $1;
	}
	;

init_declarator_list
	: init_declarator {
		$$ = $1;
	}
	| init_declarator_list COMMA init_declarator {
		$$ = OZ_mkTuple(OZ_atom("init_decl_list init_decl"), 2, $1, $3);
	}
	;

init_declarator
	: declarator {
		$$ = $1;
	}
	| declarator EQUAL_OP initializer {
		$$ = OZ_mkTuple(OZ_atom("decl = expr"), 2, $1, $3);
	}
	;

storage_class_specifier
	: TYPEDEF {
		$$ = $1;
	}
	| EXTERN {
		$$ = $1;
	}
	| STATIC {
		$$ = $1;
	}
	| AUTO {
		$$ = $1;
	}
	| REGISTER {
		$$ = $1;
	}
	;

type_specifier
	: CHAR {
		$$ = $1;
	}
	| SHORT {
		$$ = $1;
	}
	| INT {
		$$ = $1;
	}
	| LONG {
		$$ = $1;
	}
	| SIGNED {
		$$ = $1;
	}
	| UNSIGNED {
		$$ = $1;
	}
	| FLOAT {
		$$ = $1;
	}
	| DOUBLE {
		$$ = $1;
	}
	| VOID {
		$$ = $1;
	}
	| struct_or_union_specifier {
		$$ = $1;
	}
	| enum_specifier {
		$$ = $1;
	}
	| TYPE_NAME {
		$$ = $1;
	}
	;

struct_or_union_specifier
	: struct_or_union identifier OBRACE struct_declaration_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("named struct{decls}"), 3, $1, $2, $4);
	}
	| struct_or_union OBRACE struct_declaration_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("struct{decls}"), 2, $1, $3);
	}

	| struct_or_union identifier {
		$$ = OZ_mkTuple(OZ_atom("named struct/union"), 2, $1, $2);
	}
	;

struct_or_union
	: STRUCT {
		$$ = $1;
	}
	| UNION {
		$$ = $1;
	}
	;

struct_declaration_list
	: struct_declaration {
		$$ = $1;
	}
	| struct_declaration_list struct_declaration {
		$$ = OZ_mkTuple(OZ_atom("struct_decls"), 2, $1, $2);
	}
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list SEPARATOR {
		$$ = OZ_mkTuple(OZ_atom("struct_item"), 2, $1, $2);
	}
	;

specifier_qualifier_list
	: type_specifier {
		$$ = $1;
	}
	| type_specifier specifier_qualifier_list {
		$$ = OZ_mkTuple(OZ_atom("specifier_list"), 2, $1, $2);
	}
	| type_qualifier {
		$$ = $1;
	}
	| type_qualifier specifier_qualifier_list {
		$$ = OZ_mkTuple(OZ_atom("specifier_list"), 2, $1, $2);
	}
	;

struct_declarator_list
	: struct_declarator {
		$$ = $1;
	}
	| struct_declarator_list COMMA struct_declarator {
		$$ = OZ_mkTuple(OZ_atom("struct_decl_list"), 2, $1, $3);
	}
	;

struct_declarator
	: declarator {
		$$ = $1;
	}
	| COLON constant_expr {
		$$ = $2;
	}
	| declarator COLON constant_expr {
		$$ = OZ_mkTuple(OZ_atom("struct_member(decl, expr)"), 2, $1, $3);
	}
	;

enum_specifier
	: ENUM OBRACE enumerator_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("enum{decls}"), 1, $3);
	}
	| ENUM identifier OBRACE enumerator_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("named enum{decls}"), 2, $2, $4);
	}
	| ENUM identifier {
		$$ = OZ_mkTuple(OZ_atom("named enum"), 1, $2);
	}
	;

enumerator_list
	: enumerator {
		$$ = $1;
	}
	| enumerator COMMA {
		$$ = $1;
	}
	| enumerator COMMA enumerator_list {
		$$ = OZ_mkTuple(OZ_atom("enumerator_list"), 2, $1, $3);
	}
	;

enumerator
	: identifier {
		$$ = $1;
	}
	| enumerator EQUAL_OP constant_expr {
		$$ = OZ_mkTuple(OZ_atom("enumerator(list, expr)"), 2, $1, $3);
	}
	;

type_qualifier
	: CONST {
		$$ = $1;
	}
	| RESTRICT {
		$$ = $1;
	}
	| VOLATILE {
		$$ = $1;
	}
	;

function_specifier
	: INLINE {
		$$ = $1;
	}
	;

declarator
	: direct_declarator {
		$$ = $1;
	}
	| pointer direct_declarator {
		$$ = OZ_mkTuple(OZ_atom("pointer decl"), 2, $1, $2);
	}
	;

direct_declarator
	: identifier {
		$$ = $1;
	}
	| OPARENT declarator CPARENT {
		$$ = $2;
	}
	| direct_declarator OBRACKET CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("pointer decl"), 2, OZ_atom("*"), $1);
	}
	| direct_declarator OBRACKET type_qualifier_list CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[qual_list]"), 2, $1, $3);
	}
	| direct_declarator OBRACKET assignment_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[ass]"), 2, $1, $3);
	}
	| direct_declarator OBRACKET type_qualifier_list assignment_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[qual ass]"), 3, $1, $3, $4);
	}
	| direct_declarator OBRACKET STATIC assignment_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[static ass]"), 2, $1, $4);
	}
	| direct_declarator OBRACKET STATIC type_qualifier_list assignment_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[static qual_list ass]"), 3, $1, $4, $5);
	}
	| direct_declarator OBRACKET type_qualifier_list STATIC assignment_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[qual_list static ass]"), 3, $1, $3, $5);
	}
	| direct_declarator OBRACKET MUL_OP CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[*]"), 1, $1);
	}
	| direct_declarator OBRACKET type_qualifier_list MUL_OP CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("decla[qual_list *]"), 2, $1, $3);
	}
	| direct_declarator OPARENT parameter_type_list CPARENT {
		$$ = OZ_mkTuple(OZ_atom("decla(pars)"), 2, $1, $3);
	}
	| direct_declarator OPARENT CPARENT {
		$$ = OZ_mkTuple(OZ_atom("decla()"), 1, $1);
	}
	| direct_declarator OPARENT identifier_list CPARENT {
		$$ = OZ_mkTuple(OZ_atom("decla(ids)"), 2, $1, $3);
	}
	;

pointer
	: MUL_OP {
		$$ = $1;
	}
	| MUL_OP type_qualifier_list {
		$$ = OZ_mkTuple(OZ_atom("*"), 1, $2);
	}
	| MUL_OP pointer {
		$$ = OZ_mkTuple(OZ_atom("*"), 1, $2);
	}
	| MUL_OP type_qualifier_list pointer {
		$$ = OZ_mkTuple(OZ_atom("* q p"), 2, $2, $3);
	}
	;

type_qualifier_list
	: type_qualifier {
		$$ = $1;
	}
	| type_qualifier_list type_qualifier {
		$$ = OZ_mkTuple(OZ_atom("type_qual_list"), 2, $1, $2);
	}
	;

parameter_type_list
	: parameter_list {
		$$ = $1;
	}
	| parameter_list COMMA ELIPSIS {
		$$ = OZ_mkTuple(OZ_atom("..."), 1, $1);
	}
	;

parameter_list
	: parameter_declaration {
		$$ = $1;
	}
	| parameter_list COMMA parameter_declaration {
		$$ = OZ_mkTuple(OZ_atom("pars decl"), 2, $1, $3);
	}
	;

parameter_declaration
	: declaration_specifiers declarator {
		$$ = OZ_mkTuple(OZ_atom("decl_spec decl"), 2, $1, $2);
	}
	| declaration_specifiers {
		$$ = $1;
	}
	| declaration_specifiers abstract_declarator {
		$$ = OZ_mkTuple(OZ_atom("decl_spec absdecl"), 2, $1, $2);
	}
	;

identifier_list
	: identifier {
		$$ = $1;
	}
	| identifier_list COMMA identifier {
		$$ = OZ_mkTuple(OZ_atom("id list"), 2, $1, $3);
	}
	;

type_name
	: specifier_qualifier_list {
		$$ = $1;
	}
	| specifier_qualifier_list abstract_declarator {
		$$ = OZ_mkTuple(OZ_atom("spec_list absdecl"), 2, $1, $2);
	}
	;

abstract_declarator
	: pointer {
		$$ = $1;
	}
	| direct_abstract_declarator {
		$$ = $1;
	}
	| pointer direct_abstract_declarator {
		$$ = OZ_mkTuple(OZ_atom("pointer decl"), 2, $1, $2);
	}
	;

direct_abstract_declarator
	: OPARENT abstract_declarator CPARENT {
		$$ = OZ_mkTuple(OZ_atom("(decl)"), 1, $2);
	}
	| OBRACKET CBRACKET {
		$$ = OZ_atom("[]");
	}
	| direct_abstract_declarator OBRACKET CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("abs_decl[]"), 1, $1);
	}
	| direct_abstract_declarator OBRACKET assignment_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("abs_decl[ass]"), 2, $1, $3);
	}
	| OBRACKET MUL_OP CBRACKET {
		$$ = OZ_atom("[*]");
	}
	| direct_abstract_declarator OBRACKET MUL_OP CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("abs_decl[*]"), 1, $1);
	}
	| OPARENT parameter_type_list CPARENT {
		$$ = OZ_mkTuple(OZ_atom("(pars)"), 1, $2);
	}
	| direct_abstract_declarator OPARENT parameter_type_list CPARENT {
		$$ = OZ_mkTuple(OZ_atom("abs_decl(pars)"), 2, $1, $3);
	}
	;

initializer
	: assignment_expr {
		$$ = $1;
	}
	| OBRACE initializer_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("{inits}"), 1, $2);
	}
	| OBRACE initializer_list COMMA CBRACE {
		$$ = OZ_mkTuple(OZ_atom("{inits}"), 1, $2);
	}
	;

initializer_list
	: initializer {
		$$ = $1;
	}
	| designation initializer {
		$$ = OZ_mkTuple(OZ_atom("desg init"), 2, $1, $2);
	}
	| initializer_list COMMA initializer {
		$$ = OZ_mkTuple(OZ_atom("init_list"), 2, $1, $3);
	}
	| initializer_list COMMA designation initializer {
		$$ = OZ_mkTuple(OZ_atom("inits, desg init"), 3, $1, $3, $4);
	}
	;

designation
	: designatator_list EQUAL_OP {
		$$ = OZ_mkTuple(OZ_atom("desgs eq"), 2, $1, $2);
	}
	;

designatator_list
	: designatator {
		$$ = $1;
	}
	| designatator_list designatator {
		$$ = OZ_mkTuple(OZ_atom("desg_list"), 2, $1, $2);
	}
	;

designatator
	: OBRACKET constant_expr CBRACKET {
		$$ = OZ_mkTuple(OZ_atom("{expr}"), 1, $2);
	}
	| DOT identifier {
		$$ = OZ_mkTuple(OZ_atom(".id"), 1, $2);
	}
	;

statement
	: labeled_statement {
		$$ = $1;
	}
	| compound_statement {
		$$ = $1;
	}
	| expression_statement {
		$$ = $1;
	}
	| selection_statement {
		$$ = $1;
	}
	| iteration_statement {
		$$ = $1;
	}
	| jump_statement {
		$$ = $1;
	}
	;

labeled_statement
	: identifier COLON statement {
		$$ = OZ_mkTuple(OZ_atom("id, statement"), 2, $1, $3); 
	}
	| CASE constant_expr COLON statement {
		$$ = OZ_mkTuple(OZ_atom("case expr: statement"), 2, $2, $4);
	}
	| DEFAULT COLON statement {
		$$ = OZ_mkTuple(OZ_atom("defalt: statement"), 1, $3);
	}
	;

compound_statement
	: OBRACE CBRACE {
		$$ = OZ_atom("{}");
	}
	| OBRACE block_item_list CBRACE {
		$$ = OZ_mkTuple(OZ_atom("{blockis}"), 1, $2);
	}
	;

block_item_list
	: block_item {
		$$ = $1;
	}
	| block_item_list block_item {
		$$ = OZ_mkTuple(OZ_atom("blockitem_list"), 2, $1, $2);
	}
	;

block_item
	: declaration {
		$$ = $1;
	}
	| statement {
		$$ = $1;
	}
	;

expression_statement
	: SEPARATOR {
		$$ = $1;
	}
	| expr SEPARATOR {
		$$ = $1;
	}
	;

selection_statement
	: IF OPARENT expr CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("if(e) e"), 2, $3, $5);
	}
	| IF OPARENT expr CPARENT statement ELSE statement {
		$$ = OZ_mkTuple(OZ_atom("if(e) e else e"), 3, $3, $5, $7);
	}
	| SWITCH OPARENT expr CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("switch(e) s"), 2, $3, $5);
	}
	;

iteration_statement
	: WHILE OPARENT expr CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("while(e) s"), 2, $3, $5);
	}
	| DO statement WHILE OPARENT expr CPARENT SEPARATOR {
		$$ = OZ_mkTuple(OZ_atom("do s while e"), 2, $2, $5);
	}
	| FOR OPARENT SEPARATOR SEPARATOR CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(;;) s"), 1, $6);
	}
	| FOR OPARENT expr SEPARATOR SEPARATOR CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(e;;) s"), 2, $3, $7);
	}
	| FOR OPARENT expr SEPARATOR expr SEPARATOR CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(e;e;) s"), 3, $3, $5, $8);
	}
	| FOR OPARENT expr SEPARATOR expr SEPARATOR expr CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(e;e;e) s"), 4, $3, $5, $7, $9);
	}
	| FOR OPARENT declaration SEPARATOR CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(d;) s"), 2, $3, $6);
	}
	| FOR OPARENT declaration expr SEPARATOR CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(d e;) s"), 3, $3, $4, $7);
	}
	| FOR OPARENT declaration SEPARATOR expr CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(d;e;) s"), 3, $3, $5, $7);
	}
	| FOR OPARENT declaration expr SEPARATOR expr CPARENT statement {
		$$ = OZ_mkTuple(OZ_atom("for(d e;e;) s"), 4, $3, $4, $6, $8);
	}
	;

jump_statement
	: GOTO identifier SEPARATOR {
		$$ = OZ_mkTuple(OZ_atom("goto"), 1, $2);
	}
	| CONTINUE SEPARATOR {
		$$ = $1;
	}
	| BREAK SEPARATOR {
		$$ = $1;
	}
	| RETURN SEPARATOR {
		$$ = $1;
	}
	| RETURN expr SEPARATOR {
		$$ = OZ_mkTuple(OZ_atom("return"), 1, $2);
	}
	;

start
	: translation_unit {
		$$ = anchor = $1;
	}
	;

translation_unit
	: external_declaration {
		$$ = $1;
	}
	| translation_unit external_declaration {
		$$ = OZ_mkTuple(OZ_atom("trans_list"), 2, $1, $2);
	}
	;

external_declaration
	: function_definition {
		$$ = $1;
	}
	| declaration {
		$$ = $1;
	}
	;

function_definition
	: declaration_specifiers declarator compound_statement {
		$$ = OZ_mkTuple(OZ_atom("function s"), 3, $1, $2, $3);
	}
	| declaration_specifiers declarator declaration_list compound_statement {
		$$ = OZ_mkTuple(OZ_atom("function d s"), 4, $1, $2, $3, $4);
	}
	;

declaration_list
	: declaration {
		$$ = $1;
	}
	| declaration_list declaration {
		$$ = OZ_mkTuple(OZ_atom("decl_list"), 2, $1, $2);
	}
	;

identifier
	: IDENTIFIER {
#if defined(DEBUG)
/*		fprintf(stderr,"consuming id: %s\n", OZ_virtualStringToC($1, NULL)); */
#endif 
		$$ = $1;
	}
	;
%%

#include <stdio.h>

extern char yytext[];

void yyerror(char *s)
{
  longjmp(error_buf, 0);
}

/*
 * Define Interface to Oz
 */

char oz_module_name[] = "CParser";

/* Export Parser Function */

OZ_BI_define (parse_tree, 1, 1) {
  static int enter = 0;
  OZ_declareTerm(0, file);

  if ((yyin = fopen(OZ_virtualStringToC(file, NULL), "r")) == NULL) {
    fclose(yyin);
    OZ_out(0) = OZ_atom("file error: unable to open file");
  }
  else {
    /* gcc 2.96 introduces __builtin_va_list internally */
    type_list = OZ_cons(OZ_atom("__builtin_va_list"), OZ_nil());
    setjmp(error_buf);
    if (enter == 0) {
      enter = 1;
      yyparse();
#if defined(DEBUG)
      fprintf(stderr,"parse_tree: line_num = %d\n", line_num);
      verifyTypeList();
#endif
      OZ_out(0) = anchor;
    }
    else {
      OZ_out(0) = OZ_mkTuple(OZ_atom("parse error"), 2, OZ_int(line_num), type_list);
    }
  }
  return OZ_ENTAILED;
} OZ_BI_end

static OZ_C_proc_interface oz_interface[] = {
  {"parse", 1, 1, parse_tree},
  {0, 0, 0, 0}
};

OZ_C_proc_interface *oz_init_module() {
  return oz_interface;
}
