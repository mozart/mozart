/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __MAPLE_HH__
#define __MAPLE_HH__

#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#include "mozart_cpi.hh"

//-----------------------------------------------------------------------------

// templates for temporary maple files
#define MAPLE_IN_TEMPLATE  "/tmp/oz-maple-in-XXXXXX"
#define MAPLE_OUT_TEMPLATE "/tmp/oz-maple-out-XXXXXX"
#define PARSE_OUT_TEMPLATE "/tmp/oz-parse-out-XXXXXX"

// command templates
#define MAPLE_CALL_TEMPLATE "%s -q -w0 < %s > %s" 
#define DELETE_TEMPLATE "rm -f %s; rm -f %s; rm -f %s"

#define OPEN_FILE 

#define EXCEPTION "maple"

#define PARSER_OUTPUT_LENGTH OZ_atom("The parser ouput is too large.")

enum file_mode_t {writing, reading}; 

#define FOPEN(FILE_PTR, FILE_NAME, FILE_MODE)				 \
FILE * FILE_PTR = fopen(FILE_NAME, FILE_MODE == writing ? "w" : "r");	 \
if (FILE_PTR == NULL) {							 \
  ret_val = OZ_raiseErrorC(EXCEPTION, 0, 				 \
			   OZ_atom(FILE_MODE == writing 		 \
				   ? "Could not open file for writing"	 \
				   : "Could not open file for reading"), \
			   OZ_string(FILE_NAME));			 \
  goto exit;								 \
}

#define FCLOSE(FILE_PTR, FILE_NAME)				\
if (fclose(FILE_PTR)) {						\
  ret_val = OZ_raiseErrorC(EXCEPTION, 0, 			\
			   OZ_atom("Could not close file"),	\
			   OZ_string(FILE_NAME));		\
  goto exit;							\
}

//-----------------------------------------------------------------------------

#define INIT_FUNC(F_NAME) OZ_C_proc_interface * F_NAME(void)

extern "C" INIT_FUNC(oz_init_module);

OZ_BI_proto(maple_call);

//-----------------------------------------------------------------------------

inline
int maple_system(char * cmd) {
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

//-----------------------------------------------------------------------------
// auxiliary functions and other stuff


#endif /* __MAPLE_HH__ */
