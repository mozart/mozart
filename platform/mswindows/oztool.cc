/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *    Ralf Scheidhauer <scheidhr@dfki.de>
 *
 *  Copyright:
 *    Leif Kornstaedt, 1999
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
#include <unistd.h>

#include "startup.hh"

//
// Summary of how oztool behaves:
//
// oztool -gnu c++ -c <cfile> [ -o <ofile> ]
//    g++ -I"$OZHOME/include" -c <cfile> [ -o <ofile> ]
// oztool -gnu cc -c <cfile> [ -o <ofile> ]
//    gcc -I"$OZHOME/include" -c <cfile> [ -o <ofile> ]
// oztool -gnu ld -o <target> <file1> ... <filen>
//    dlltool --def $OZHOME/include/emulator.def --output-lib <tmpfile>.a
//    dllwrap -s -o <target> --dllname <target> <file1> ... <filen> \
//       <tmpfile>.a -lmsvcrt
//    rm <tmpfile>.a
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
  //--** take oztmpnam from emulator/os.cc?
  return tmpnam(NULL);
}

char *toUnix(char *s)
{
  for (char *t = s; *t; t++)
    if (*t == '/')
      *t = '\\';
  return s;
}

void doexit(int n)
{
  if (n != 0)
    fprintf(stderr,"*** error: oztool aborted with code %d\n",n);
  exit(n);
}

char *shorten(char *path)
{
  static char buffer[2048];
  int n = GetShortPathName(path,buffer,sizeof(buffer));
  if (n == 0 || n >= sizeof(buffer))
    doexit(17);
  return buffer;
}

int verbose = 0;

int execute(char **argv, bool dontQuote)
{
  char buffer[4096];
  char *aux = buffer;
  while (*argv) {
    if (!dontQuote)
      *aux++ = '\"';
    strcpy(aux,*argv);
    aux += strlen(*argv++);
    if (!dontQuote)
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
  PROCESS_INFORMATION pi;
  DWORD ret = CreateProcess(NULL,buffer,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
  if (ret == FALSE) {
    panic(true,"Cannot run '%s'.\n",buffer);
  }
  CloseHandle(pi.hThread);
  WaitForSingleObject(pi.hProcess,INFINITE);

  DWORD code;
  ret = GetExitCodeProcess(pi.hProcess,&code);
  CloseHandle(pi.hProcess);
  if (ret != FALSE)
    return code;
  else
    return 0;

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

  if ((!strcmp(argv[1],"cc") || !strcmp(argv[1],"c++"))
      && (argc == 4 || argc == 6) && !strcmp(argv[2],"-c")
      && (argc == 4 || !strcmp(argv[4],"-o"))) {
    int cxx = !strcmp(argv[1],"c++");
    char **ccCmd;
    int r = 0;
    bool dontQuote = false;
    switch (sys) {
    case SYS_GNU:
      ccCmd = new char*[argc == 4? 5: 7];
      if (cxx)
        ccCmd[r++] = "g++";
      else
        ccCmd[r++] = "gcc";
      ccCmd[r++] = concat("-I",concat(getOzHome(true),"/include"));
      ccCmd[r++] = "-c";
      ccCmd[r++] = argv[3];
      if (argc == 6) {
        ccCmd[r++] = "-o";
        ccCmd[r++] = argv[5];
      }
      break;
    case SYS_MSVC:
      ccCmd = new char*[argc == 4? 7: 8];
      ccCmd[r++] = "cl";
      ccCmd[r++] = "-nologo";
      if (cxx)
        ccCmd[r++] = "-TP";
      else
        ccCmd[r++] = "-TC";
      ccCmd[r++] = concat("-I",concat(getOzHome(false),"\\include"));
      ccCmd[r++] = "-c";
      ccCmd[r++] = argv[3];
      if (argc == 6)
        ccCmd[r++] = concat("-Fo",argv[5]);
      break;
    case SYS_WATCOM:
      ccCmd = new char*[argc == 4? 7: 8];
      if (cxx)
        ccCmd[r++] = "wpp386";
      else
        ccCmd[r++] = "wcc386";
      ccCmd[r++] = "-zq";
      ccCmd[r++] = "-bd";
      ccCmd[r++] = "-5s";
      ccCmd[r++] = concat("-i=",concat(shorten(getOzHome(false)),"\\include"));
      ccCmd[r++] = argv[3];
      if (argc == 6)
        ccCmd[r++] = concat("-fo=",argv[5]);
      dontQuote = true;
      break;
    }
    ccCmd[r] = NULL;
    r = execute(ccCmd,dontQuote);
    doexit(r);
  } else if (!strcmp(argv[1],"ld") && argc >= 4 && !strcmp(argv[2],"-o")) {
    if (argc == 4) {
      usage("Missing object files.\n");
    }
    char *target = argv[3];
    switch (sys) {
    case SYS_GNU:
      {
        char *tmpfile_a = concat(ostmpnam(),".a");
        char **dlltoolCmd = new char*[6];
        dlltoolCmd[0] = "dlltool";
        dlltoolCmd[1] = "--def";
        dlltoolCmd[2] = concat(getOzHome(true),"/include/emulator.def");
        dlltoolCmd[3] = "--output-lib";
        dlltoolCmd[4] = tmpfile_a;
        dlltoolCmd[5] = NULL;
        int r = execute(dlltoolCmd,false);
        if (!r) {
          char **dllwrapCmd = new char*[argc+4];
          dllwrapCmd[r++] = "dllwrap";
          dllwrapCmd[r++] = "-s";
          dllwrapCmd[r++] = "-o";
          dllwrapCmd[r++] = target;
          for (int i = 4; i < argc; i++)
            dllwrapCmd[r++] = argv[i];
          dllwrapCmd[r++] = tmpfile_a;
          dllwrapCmd[r++] = "-lmsvcrt";
          dllwrapCmd[r] = NULL;
          r = execute(dllwrapCmd,false);
        }
        unlink(tmpfile_a);
        doexit(r);
      }
    case SYS_MSVC:
      {
        char *tmpfile = toUnix(ostmpnam());
        char *tmpfile_lib = concat(tmpfile,".lib");
        char *tmpfile_exp = concat(tmpfile,".exp");
        char **libCmd = new char *[6];
        libCmd[0] = "lib";
        libCmd[1] = "/nologo";
        libCmd[2] =
          concat("/def:",concat(getOzHome(false),"\\include\\emulator.def"));
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
          linkCmd[r++] = concat("/out:",target);
          for (int i = 4; i < argc; i++)
            linkCmd[r++] = argv[i];
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
      {
        char *tmpfile_lib = concat(ostmpnam(),".lib");
        char **wlibCmd = new char*[6];
        wlibCmd[0] = "wlib";
        wlibCmd[1] = "/q";
        wlibCmd[2] = "/n";
        wlibCmd[3] = tmpfile_lib;
        wlibCmd[4] = concat("@",concat(shorten(getOzHome(false)),
                                       "\\include\\emulator.cmd"));
        wlibCmd[5] = NULL;
        int r = execute(wlibCmd,true);
        if (!r) {
          char **wlinkCmd = new char*[10];
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
}
