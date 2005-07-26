/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_SWITCH = 258,
     T_SWITCHNAME = 259,
     T_LOCALSWITCHES = 260,
     T_PUSHSWITCHES = 261,
     T_POPSWITCHES = 262,
     T_OZATOM = 263,
     T_ATOM_LABEL = 264,
     T_OZFLOAT = 265,
     T_OZINT = 266,
     T_AMPER = 267,
     T_DOTINT = 268,
     T_STRING = 269,
     T_VARIABLE = 270,
     T_VARIABLE_LABEL = 271,
     T_DEFAULT = 272,
     T_CHOICE = 273,
     T_LDOTS = 274,
     T_2DOTS = 275,
     T_attr = 276,
     T_at = 277,
     T_case = 278,
     T_catch = 279,
     T_choice = 280,
     T_class = 281,
     T_cond = 282,
     T_declare = 283,
     T_define = 284,
     T_dis = 285,
     T_else = 286,
     T_elsecase = 287,
     T_elseif = 288,
     T_elseof = 289,
     T_end = 290,
     T_export = 291,
     T_fail = 292,
     T_false = 293,
     T_FALSE_LABEL = 294,
     T_feat = 295,
     T_finally = 296,
     T_from = 297,
     T_fun = 298,
     T_functor = 299,
     T_if = 300,
     T_import = 301,
     T_in = 302,
     T_local = 303,
     T_lock = 304,
     T_meth = 305,
     T_not = 306,
     T_of = 307,
     T_or = 308,
     T_prepare = 309,
     T_proc = 310,
     T_prop = 311,
     T_raise = 312,
     T_require = 313,
     T_self = 314,
     T_skip = 315,
     T_then = 316,
     T_thread = 317,
     T_true = 318,
     T_TRUE_LABEL = 319,
     T_try = 320,
     T_unit = 321,
     T_UNIT_LABEL = 322,
     T_for = 323,
     T_FOR = 324,
     T_do = 325,
     T_ENDOFFILE = 326,
     T_REGEX = 327,
     T_lex = 328,
     T_mode = 329,
     T_parser = 330,
     T_prod = 331,
     T_scanner = 332,
     T_syn = 333,
     T_token = 334,
     T_REDUCE = 335,
     T_SEP = 336,
     T_ITER = 337,
     T_OOASSIGN = 338,
     T_DOTASSIGN = 339,
     T_COLONEQUALS = 340,
     T_orelse = 341,
     T_andthen = 342,
     T_RMACRO = 343,
     T_LMACRO = 344,
     T_FDCOMPARE = 345,
     T_COMPARE = 346,
     T_FDIN = 347,
     T_ADD = 348,
     T_OTHERMUL = 349,
     T_FDMUL = 350,
     T_DEREFF = 351
   };
#endif
#define T_SWITCH 258
#define T_SWITCHNAME 259
#define T_LOCALSWITCHES 260
#define T_PUSHSWITCHES 261
#define T_POPSWITCHES 262
#define T_OZATOM 263
#define T_ATOM_LABEL 264
#define T_OZFLOAT 265
#define T_OZINT 266
#define T_AMPER 267
#define T_DOTINT 268
#define T_STRING 269
#define T_VARIABLE 270
#define T_VARIABLE_LABEL 271
#define T_DEFAULT 272
#define T_CHOICE 273
#define T_LDOTS 274
#define T_2DOTS 275
#define T_attr 276
#define T_at 277
#define T_case 278
#define T_catch 279
#define T_choice 280
#define T_class 281
#define T_cond 282
#define T_declare 283
#define T_define 284
#define T_dis 285
#define T_else 286
#define T_elsecase 287
#define T_elseif 288
#define T_elseof 289
#define T_end 290
#define T_export 291
#define T_fail 292
#define T_false 293
#define T_FALSE_LABEL 294
#define T_feat 295
#define T_finally 296
#define T_from 297
#define T_fun 298
#define T_functor 299
#define T_if 300
#define T_import 301
#define T_in 302
#define T_local 303
#define T_lock 304
#define T_meth 305
#define T_not 306
#define T_of 307
#define T_or 308
#define T_prepare 309
#define T_proc 310
#define T_prop 311
#define T_raise 312
#define T_require 313
#define T_self 314
#define T_skip 315
#define T_then 316
#define T_thread 317
#define T_true 318
#define T_TRUE_LABEL 319
#define T_try 320
#define T_unit 321
#define T_UNIT_LABEL 322
#define T_for 323
#define T_FOR 324
#define T_do 325
#define T_ENDOFFILE 326
#define T_REGEX 327
#define T_lex 328
#define T_mode 329
#define T_parser 330
#define T_prod 331
#define T_scanner 332
#define T_syn 333
#define T_token 334
#define T_REDUCE 335
#define T_SEP 336
#define T_ITER 337
#define T_OOASSIGN 338
#define T_DOTASSIGN 339
#define T_COLONEQUALS 340
#define T_orelse 341
#define T_andthen 342
#define T_RMACRO 343
#define T_LMACRO 344
#define T_FDCOMPARE 345
#define T_COMPARE 346
#define T_FDIN 347
#define T_ADD 348
#define T_OTHERMUL 349
#define T_FDMUL 350
#define T_DEREFF 351




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
  OZ_Term t;
  int i;
} YYSTYPE;
/* Line 1318 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE xylval;
