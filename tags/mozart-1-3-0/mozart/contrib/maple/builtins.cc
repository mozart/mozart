/*
 *  Authors:
 *    Jürgen Zimmer (jzimmer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *
 *  Copyright:
 *    1999
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "maple.hh"

//-----------------------------------------------------------------------------

extern "C" {
  void maple2oz (FILE * in, FILE * out);
}

//-----------------------------------------------------------------------------
OZ_BI_define(maple_call, 2, 1)
{
  OZ_Return ret_val;

  int input_len, maple_len, parse_len;
  char * input     = strdup(OZ_virtualStringToC(OZ_in(0), &input_len));
  char * maple_str = strdup(OZ_virtualStringToC(OZ_in(1), &maple_len));

  char * maple_in_name  = mktemp(strdup(MAPLE_IN_TEMPLATE));
  char * maple_out_name = mktemp(strdup(MAPLE_OUT_TEMPLATE));
  char * parse_out_name = mktemp(strdup(PARSE_OUT_TEMPLATE));

  int total_maple_len = maple_len + strlen(maple_in_name) +
    strlen(maple_out_name) + strlen(MAPLE_CALL_TEMPLATE) - 6 + 1;
  char call_maple_str[total_maple_len];
  int actual_maple_len = sprintf(call_maple_str,
				 MAPLE_CALL_TEMPLATE,
				 maple_str,
				 maple_in_name,
				 maple_out_name);

  FOPEN(maple_in, maple_in_name, writing);

  fputs(input, maple_in);

  FCLOSE(maple_in, maple_in_name);

  OZ_Term rv = OZ_nil();

  maple_system(call_maple_str);       // call Maple(tm) ...
  
  FOPEN(maple_out, maple_out_name, reading);
  
  const int top_str_len = 6;
  char top_str[top_str_len];
  
  fgets(top_str, top_str_len, maple_out);
  
  // check if Maple produced an error
  if (strncmp("Error", top_str, 5) == 0) {
    int c;
    
    add_list(rv, '\'');
    
    for (int k = 1; k<6; k++)
      add_list(rv, top_str[k-1]);
    add_list(rv, '\'');
    add_list(rv, '(');
    add_list(rv, '\'');
    
    int first_blank = 1;
    do {              // read the Error message
      c = fgetc(maple_out);
      if (! ((c == ',') ||
	     (c == EOF) ||
	     (c == '\n') ||
	     (c == ' ' && first_blank))) {
	first_blank = 0;
	add_list(rv, c);
      }
    } while (!(c == EOF));
    add_list(rv, '\'');
    add_list(rv, ')');
    
    FCLOSE(maple_out, maple_out_name);
    
  } else {

    // everything went fine
    
    FCLOSE(maple_out, maple_out_name);
    
    // convert Maple output to an Oz value
    FOPEN(in, maple_out_name, reading);
    FOPEN(out, parse_out_name, writing);

    maple2oz (in, out);

    FCLOSE(in, maple_out_name);
    FCLOSE(out, parse_out_name);

    FOPEN(parse_out, parse_out_name, reading);

    int end = 0, c;

    // skip initial blanks
    do {
      c = fgetc(parse_out);
    } while (!((c == EOF) || (c == '"') ));


    if (!(c == EOF)) {
      do {
	c = fgetc(parse_out);
	if ((c == EOF) || (c == '"')) {
	  end = 1;
	} else {
	  add_list(rv, c);
	}
      } while (!end);

      FCLOSE(parse_out, parse_out_name);
    }
  }

  free(input);
  free(maple_str);

  int total_delete_len = strlen(DELETE_TEMPLATE) + strlen(maple_in_name) +
    strlen(maple_out_name) + strlen(parse_out_name) + 10;
  char delete_str[total_delete_len];
  int actual_delete_len = sprintf(delete_str,
				  DELETE_TEMPLATE,
				  maple_in_name,
				  maple_out_name,
				  parse_out_name);

  Assert(total_delete_len == actual_delete_len);

  maple_system(delete_str);          // delete temporary files...

  OZ_RETURN(close_list(rv));
}
OZ_BI_end
