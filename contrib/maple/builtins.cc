#include "maple.hh"

extern "C" {
  void maple2oz (FILE * in, FILE * out);
}

int MySystem(char* cmd)
{
  pid_t pid;
  if ((pid = fork()) < 0) {
    return -1;
  }

  if (pid == 0) {
    execl("/bin/sh","sh","-c",cmd, (char*) NULL);
    _exit(127);     /* execl error */
  }

  int status;
  while(waitpid(pid,&status,0) < 0) {
    if (errno != EINTR) {
      return -1;
    }
  }

  return status;
}

//----------------------------------------------------
OZ_BI_define(maple_call, 3, 1)
{
  OZ_Return ret_val;

  OZ_Term dummy = OZ_atom("dummy");
  int input_len, maple_len, parse_len;
  char * input     = strdup(OZ_virtualStringToC(OZ_in(0), &input_len));
  char * maple_str = strdup(OZ_virtualStringToC(OZ_in(1), &maple_len));
  char * parse_str = strdup(OZ_virtualStringToC(OZ_in(2), &parse_len));

  char * maple_in_name = mktemp(strdup(MAPLE_IN_TEMPLATE));
  char * maple_out_name = mktemp(strdup(MAPLE_OUT_TEMPLATE));
  char * parse_out_name = mktemp(strdup(PARSE_OUT_TEMPLATE));

  char output[400];

  int total_maple_len = maple_len + strlen(maple_in_name) +
    strlen(maple_out_name) + strlen(MAPLE_CALL_TEMPLATE) + 10;
  int total_parse_len = parse_len + strlen(maple_out_name) +
    strlen(parse_out_name) + strlen(PARSE_CALL_TEMPLATE) + 10;
  int total_delete_len = strlen(DELETE_TEMPLATE) + strlen(maple_in_name) +
    strlen(maple_out_name) + strlen(parse_out_name) + 10;

  char call_maple_str[total_maple_len];
  char call_parse_str[total_parse_len];
  char delete_str[total_delete_len];

  int actual_maple_len = sprintf(call_maple_str,
				 MAPLE_CALL_TEMPLATE,
				 maple_str,
				 maple_in_name,
				 maple_out_name);

  int actual_parse_len = sprintf(call_parse_str,
				 PARSE_CALL_TEMPLATE,
				 parse_str,
				 maple_out_name,
				 parse_out_name);
  int actual_delete_len = sprintf(delete_str,
				  DELETE_TEMPLATE,
				  maple_in_name,
				  maple_out_name,
				  parse_out_name);

  FILE * maple_in = fopen(maple_in_name, "w");

  if (maple_in == NULL) {
    fprintf(stdout, "Could not open file %s for writing (%s:%d)\n",
	    maple_in_name, __FILE__, __LINE__);

    ret_val = OZ_raiseErrorC(EXCEPTION, 0, OPEN_FILE, 
			     OZ_string(maple_in_name));
    goto exit;
  }

  fputs(input, maple_in);

  fclose(maple_in);

  {
    MySystem(call_maple_str);       // call Maple(tm) ...

    FILE * maple_out = fopen(maple_out_name, "r"); // open maple output file

    if (maple_out == NULL) {
      fprintf(stdout, "Could not open file %s for reading (%s:%d)\n", 
	      maple_out_name,
	      __FILE__, __LINE__);

      ret_val = OZ_raiseErrorC(EXCEPTION, 0, OPEN_FILE, 
			       OZ_string(maple_out_name));
      goto exit;
    }

    char top_str[5];

    fgets(top_str, 6, maple_out);

    if (strncmp("Error", top_str, 4) == 0) {
      int c;

      output[0] = '\'';
      for (int k = 1; k<6; k++)
	output[k] = top_str[k-1];
      output[6] = '\'';
      output[7] = '(';
      output[8] = '\'';

      int i = 9;
      do {              // read the Error message
	c = fgetc(maple_out);
	if (! ((c == ',') || (c == EOF) || (c == '\n'))) output[i++] = c;
      } while (!(c == EOF));
      output[i++] = '\'';
      output[i++] = ')';
      output[i] = 0;

      fclose(maple_out);

      //fprintf(stdout, "Maple returned an error! %s\n", output);
      ret_val = OZ_unify(OZ_in(3), OZ_string(output));
      goto exit;
    }

    fclose(maple_out);

    //    MySystem(call_parse_str);       // ... and the parser


    FILE * in = fopen(maple_out_name, "r");
    FILE * out = fopen(parse_out_name, "w");

    maple2oz (in, out);

    fclose(in);
    fclose(out);

    MySystem("sync");               // do a system sync

    FILE * parse_out = fopen(parse_out_name, "r"); // open parser output file

    if (parse_out == NULL) {
      fprintf(stdout, "Could not open file %s for reading (%s:%d)\n",
	      parse_out_name,
	      __FILE__, __LINE__);

      ret_val = OZ_raiseErrorC(EXCEPTION, 0, OPEN_FILE,
			       OZ_string(parse_out_name));
      goto exit;
    }

    int i = 0;
    int end=0;
    int c;

    do {              // skip initial blanks
      c = fgetc(parse_out);
    } while (!((c == EOF) || (c == '"') ));

    //printf("char: %c \n", c);

    if (!(c == EOF)) {
      do {
	c = fgetc(parse_out);
	if ((c == EOF) || (c == '"'))
	  { end = 1;}
	else {
	  output[i++] = c;
	  //printf("char: %c \n", c);
	}
      } while (!end);

      output[i] = 0;
      fclose(parse_out);
    }
  }
  //printf("output: %s \n", output);               // the parser output...

  ret_val = OZ_unify(OZ_in(3), OZ_string(output));

 exit:
  free(input);
  //unlink(maple_in_name);
  //unlink(parse_out_name);        // unlink temporary files
  // MySystem(delete_str);          // delete temporary files...

  return ret_val;
}
OZ_BI_end



