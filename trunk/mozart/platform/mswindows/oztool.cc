/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *    Ralf Scheidhauer <scheidhr@dfki.de>
 *
 *  Contributors:
 *    Thorsten Brunklaus <brunklaus@ps.uni-sb.de>
 *
 *  Copyright:
 *    Thorsten Brunklaus, 2002-2003
 *    Leif Kornstaedt, 1999-2003
 *    Ralf Scheidhauer, 1999
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <process.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "startup.hh"

//
// Summary of how oztool behaves:
//
// oztool -gnu c++ -c <cfile> [ -o <ofile> ]
//    g++-2 -mno-cygwin -I"$OZHOME/include" -c <cfile> [ -o <ofile> ]
// oztool -gnu cc -c <cfile> [ -o <ofile> ]
//    gcc-2 -mno-cygwin -I"$OZHOME/include" -c <cfile> [ -o <ofile> ]
// oztool -gnu ld -o <target> <file1> ... <filen>
//    g++-2 -mno-cygwin -shared <args> \
//       $OZHOME/platform/win32-i486/emulator.dll -lmsvcrt
//
// oztool -msvc c++ -c <cfile> [ -o <ofile> ]
//    cl -nologo -TP -I"$OZHOME\include" -c <cfile> [ -Fo<ofile> ]
// oztool -msvc cc -c <cfile> [ -o <ofile> ]
//    cl -nologo -TC -I"$OZHOME\include" -c <cfile> [ -Fo<ofile> ]
// oztool -msvc ld -o <target> <file1> ... <filen>
//    lib /nologo /def:$OZHOME\include\emulator.def /machine:ix86 \
//       /out:<tmpfile>.lib
//    link /nologo /dll /out:<target> <file1> ...<filen> <tmpfile>.lib
//       /nodefaultlib:libc.lib /defaultlib:msvcrt.lib
//    rm <tmpfile>.lib <tmpfile>.exp
//
// oztool -watcom c++ -c <cfile> [ -o <ofile> ]
//    wpp386 -zq -bd -5s -i=$OZHOME\include <cfile> [ -fo=<ofile> ]
// oztool -watcom cc -c <cfile> [ -o <ofile> ]
//    wcc386 -zq -bd -5s -i=$OZHOME\include <cfile> [ -fo=<ofile> ]
// oztool -watcom ld -o <target> <file1> ... <filen>
//    wlib /q /n <tmpfile>.lib @$OZHOME\include\emulator.cmd
//    wlink system nt_dll name <target> \
//       file <file1>,...,<filen> library <tmpfile>.lib
//    rm <tmpfile>.lib
//

bool console = true;

void doexit(int n)
{
  if (n != 0)
    fprintf(stderr,"*** error: oztool aborted with code %d\n",n);
  exit(n);
}

void usage()
{
  fprintf
    (stderr,
     "Usage:\n"
     "\toztool [-verbose] [-gnu|-msvc|-watcom] c++ -c <cfile> [ -o <ofile> ]\n"
     "\toztool [-verbose] [-gnu|-msvc|-watcom] cc  -c <cfile> [ -o <ofile> ]\n"
     "\toztool [-verbose] [-gnu|-msvc|-watcom] ld  -o <target> <files>\n"
     "\toztool platform [ -o <ofile> ]\n");
  exit(2);
}

void usage(char *msg)
{
  fprintf(stderr,msg);
  usage();
}

char *concat(const char *st1, const char *st2)
{
  int i = strlen(st1) + strlen(st2) + 1;
  char *ret = new char[i];
  strcpy(ret, st1);
  strcat(ret, st2);
  return ret;
}

char *commaList(char **argv, int from, int to)
{
  int len = 0;
  for (int i = from; i < to; i++)
    len += strlen(argv[i]) + 1;
  char *ret = new char[len + 1];
  char *aux = ret;
  while (from < to) {
    strcpy(aux, argv[from++]);
    aux += strlen(aux);
    if (from < to)
      *aux++ = ',';
  }
  return ret;
}

char *ostmpnam()
{
  static char path[2048];
  int n = GetTempPath(sizeof(path),path);
  if (n == 0 || n == sizeof(path))
    doexit(18);
  static char file[MAX_PATH];
  n = GetTempFileName(path,"oztool",0,file);
  if (n == 0)
    doexit(19);
  return strdup(file);
}

char *toUnix(char *s)
{
  s = strdup(s);
  for (char *t = s; *t; t++)
    if (*t == '\\')
      *t = '/';
  return s;
}

char *toWindows(char *s)
{
  s = strdup(s);
  for (char *t = s; *t; t++)
    if (*t == '/')
      *t = '\\';
  return s;
}

char *toDos(char *path)
{
  static char buffer[2048];
  int n = GetShortPathName(path,buffer,sizeof(buffer));
  if (n == 0 || n == sizeof(buffer))
    doexit(17);
  return buffer;
}

int verbose = 0;

int execute(char **argv, bool dontQuote)
{
  char buffer[4096];
  char *aux = buffer;
  for (int i = 0; argv[i]; i++) {
    if (!dontQuote && i > 0)
      *aux++ = '\"';
    strcpy(aux,argv[i]);
    aux += strlen(argv[i]);
    if (!dontQuote && i > 0)
      *aux++ = '\"';
    *aux++ = ' ';
  }
  *aux = '\0';

  if (verbose) {
    printf("%s\n",buffer);
    fflush(stdout);
  }

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_FORCEOFFFEEDBACK|STARTF_USESTDHANDLES;
  SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE),
		       HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
  SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE),
		       HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
  SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE),
		       HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

  PROCESS_INFORMATION pi;
  if (!CreateProcess(NULL,buffer,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)) {
    panic(true,"Cannot run '%s'.\n",buffer);
  }
  CloseHandle(pi.hThread);

  if (WaitForSingleObject(pi.hProcess,INFINITE) == WAIT_FAILED) {
    panic(true,"Wait for subprocess failed.\n");
  }

  DWORD code;
  if (GetExitCodeProcess(pi.hProcess,&code) == FALSE)
    code = 1;
  CloseHandle(pi.hProcess);

  return code;
}

static char *getIncDir(int &argc, char **argv) {
  char *incdir = NULL;
  for (int i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-inc")) {
      if (i + 1 == argc)
	usage("Missing argument to `-inc'.\n");
      incdir = argv[i + 1];
      for (int j = i + 2; j < argc; j++)
	argv[j - 2] = argv[j];
      argc -= 2;
      break;
    }
  return incdir;
}

enum system_type {
  SYS_GNU, SYS_MSVC, SYS_WATCOM
};

int main(int argc, char **argv)
{
  if (argc < 2)
    usage();

  if (!strcmp(argv[1],"platform")) {
    if (argc==2) {
      printf("%s\n",ozplatform);
      exit(0);
    } else if (argc==4 && !strcmp(argv[2],"-o")) {
      FILE *output = fopen(argv[3],"w");
      if (output==NULL) {
	panic(true,"Cannot open file '%s' for writing.\n",argv[3]);
	exit(1);
      }
      fprintf(output,"%s\n",ozplatform);
      fclose(output);
      exit(0);
    } else {
      usage();
    }
  }

  if (!strcmp(argv[1],"-verbose")) {
    verbose = 1;
    argv++;
    argc--;
  }

  if (argc < 3)
    usage();

  system_type sys = SYS_GNU;
  if (!strcmp(argv[1],"-gnu")) {
    sys = SYS_GNU;
    argv++; argc--;
  } else if (!strcmp(argv[1],"-msvc")) {
    sys = SYS_MSVC;
    argv++; argc--;
  } else if (!strcmp(argv[1],"-watcom")) {
    sys = SYS_WATCOM;
    argv++; argc--;
  }

  char *gccProg = ozGetenv("OZTOOL_CC");
  gccProg = gccProg? strdup(gccProg): const_cast<char *>("gcc-2 -mno-cygwin");
  char *gxxProg = ozGetenv("OZTOOL_CXX");
  gxxProg = gxxProg? strdup(gxxProg): const_cast<char *>("g++-2 -mno-cygwin");
  char *gldProg = ozGetenv("OZTOOL_LD");
  gldProg = gldProg? strdup(gldProg): const_cast<char *>("g++-2 -mno-cygwin");
  char *incdir = getIncDir(argc,argv);
  bool hasIncdir = incdir != NULL;
  if (!hasIncdir) incdir = concat(getOzHome(true),"/include");

  if (!strcmp(argv[1],"cc") || !strcmp(argv[1],"c++")) {
    int cxx = !strcmp(argv[1],"c++");
    char **ccCmd;
    int r = 0;
    int k = 2;
    bool dontQuote = false;
    switch (sys) {
    case SYS_GNU:
      ccCmd = new char *[argc + 3];
      if (cxx)
	ccCmd[r++] = gxxProg;
      else
	ccCmd[r++] = gccProg;
      ccCmd[r++] = concat("-I",toUnix(incdir));
      while (k < argc) {
	if (!strcmp(argv[k],"-c") || !strcmp(argv[k],"-o")) {
	  if (argc == k + 1) {
	    usage("Missing file argument.\n");
	  }
	  ccCmd[r++] = argv[k++];
	  ccCmd[r++] = argv[k++];
	} else {
	  ccCmd[r++] = argv[k++];
	}
      }
      break;
    case SYS_MSVC:
      ccCmd = new char *[argc + 6];
      ccCmd[r++] = "cl";
      ccCmd[r++] = "-nologo";
      if (cxx)
	ccCmd[r++] = "-TP";
      else
	ccCmd[r++] = "-TC";
      ccCmd[r++] = concat("-I",toWindows(incdir));
      while (k < argc) {
	if (!strcmp(argv[k],"-c")) {
	  if (argc == k + 1) {
	    usage("Missing file argument.\n");
	  }
	  ccCmd[r++] = argv[k++];
	  ccCmd[r++] = argv[k++];
	} else if (!strcmp(argv[k],"-o")) {
	  if (argc == k + 1) {
	    usage("Missing file argument.\n");
	  }
	  ccCmd[r++] = concat("-Fo",argv[k + 1]);
	  k += 2;
	} else if (!strncmp(argv[k],"-I",2)) {
	  ccCmd[r++] = concat("-I",toWindows(argv[k++] + 2));
	} else {
	  ccCmd[r++] = argv[k++];
	}
      }
      break;
    case SYS_WATCOM:
      ccCmd = new char*[argc + 6];
      if (cxx)
	ccCmd[r++] = "wpp386";
      else
	ccCmd[r++] = "wcc386";
      ccCmd[r++] = "-zq";
      ccCmd[r++] = "-bd";
      ccCmd[r++] = "-5s";
      ccCmd[r++] = concat("-i=",toDos(incdir));
      while (k < argc) {
	if (!strcmp(argv[k],"-c")) {
	  if (argc == k + 1) {
	    usage("Missing file argument.\n");
	  }
	  ccCmd[r++] = argv[k + 1];
	  k += 2;
	} else if (!strcmp(argv[k], "-o")) {
	  if (argc == k + 1) {
	    usage("Missing file argument.\n");
	  }
	  ccCmd[r++] = concat("-fo=",argv[k + 1]);
	  k += 2;
	} else if (!strncmp(argv[k],"-I",2)) {
	  ccCmd[r++] = concat("-i=",toDos(argv[k++] + 2));
	} else {
	  ccCmd[r++] = argv[k++];
	}
      }
      dontQuote = true;
      break;
    }
    ccCmd[r] = NULL;
    r = execute(ccCmd,dontQuote);
    doexit(r);
  } else if (!strcmp(argv[1],"ld")) {
    switch (sys) {
    case SYS_GNU: {
      char *ozhome =
	hasIncdir? incdir: concat(getOzHome(true),"/platform/win32-i486");
      char **gccSharedCmd = new char *[4+argc+1];
      int index = 0;
      gccSharedCmd[index++] = gldProg;
      gccSharedCmd[index++] = "-shared";
      for (int k = 2; k < argc; k++)
	gccSharedCmd[index++] = argv[k];
      gccSharedCmd[index++] = concat(ozhome,"/emulator.dll");
      gccSharedCmd[index] = NULL;
      doexit(execute(gccSharedCmd,false));
    }
    case SYS_MSVC: {
      char *tmpfile = toWindows(ostmpnam());
      char *tmpfile_lib = concat(tmpfile,"lib");
      char *tmpfile_exp = concat(tmpfile,"exp");
      char **libCmd = new char *[6];
      libCmd[0] = "lib";
      libCmd[1] = "/nologo";
      libCmd[2] =
	concat("/def:",toWindows(concat(incdir,"\\emulator.def")));
      libCmd[3] = "/machine:ix86";
      libCmd[4] = concat("/out:",tmpfile_lib);
      libCmd[5] = NULL;
      int r = execute(libCmd,false);
      if (!r) {
	char **linkCmd = new char *[argc + 3];
	r = 0;
	linkCmd[r++] = "link";
	linkCmd[r++] = "/nologo";
	linkCmd[r++] = "/dll";
	int i = 2;
	while (i < argc) {
	  if (!strcmp(argv[i],"-o")) {
	    if (argc == i + 1) {
	      usage("Missing argument to `-o'.\n");
	    }
	    linkCmd[r++] = concat("/out:", argv[i + 1]);
	    i += 2;
	  } else if (!strncmp(argv[i],"-l",2)) {
	    linkCmd[r++] = concat(argv[i++] + 2, ".lib");
	  } else if (!strncmp(argv[i],"-L",2)) {
	    linkCmd[r++] = concat("/libpath:",toWindows(argv[i] + 2));
	  } else if (!strcmp(argv[i],"-s")) {
	    i++;
	  } else {
	    linkCmd[r++] = argv[i++];
	  }
	}
	linkCmd[r++] = tmpfile_lib;
	linkCmd[r++] = "/nodefaultlib:libc.lib";
	linkCmd[r++] = "/defaultlib:msvcrt.lib";
	linkCmd[r] = NULL;
	r = execute(linkCmd,false);
      }
      unlink(tmpfile_lib);
      unlink(tmpfile_exp);
      doexit(r);
    }
    case SYS_WATCOM:
      // options -l, -L, -s not implemented yet for WATCOM
      if (argc >= 4 && !strcmp(argv[2],"-o")) {
	if (argc == 4) {
	  usage("Missing object files.\n");
	}
	char *target = argv[3];
	char *tmpfile_lib = toDos(concat(ostmpnam(),".lib"));
	char **wlibCmd = new char *[6];
	wlibCmd[0] = "wlib";
	wlibCmd[1] = "/q";
	wlibCmd[2] = "/n";
	wlibCmd[3] = tmpfile_lib;
	wlibCmd[4] = concat("@",toDos(concat(incdir,"\\emulator.cmd")));
	wlibCmd[5] = NULL;
	int r = execute(wlibCmd,true);
	if (!r) {
	  char **wlinkCmd = new char *[10];
	  wlinkCmd[0] = "wlink";
	  wlinkCmd[1] = "system";
	  wlinkCmd[2] = "nt_dll";
	  wlinkCmd[3] = "name";
	  wlinkCmd[4] = target;
	  wlinkCmd[5] = "file";
	  wlinkCmd[6] = commaList(argv,4,argc);
	  wlinkCmd[7] = "library";
	  wlinkCmd[8] = tmpfile_lib;
	  wlinkCmd[9] = NULL;
	  r = execute(wlinkCmd,true);
	}
	unlink(tmpfile_lib);
	doexit(r);
      }
    }
  }
  usage();
  return 2;
}
