/*
 *  Authors:
 *    Ralf Scheidhauer <scheidhr@dfki.de>
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Ralf Scheidhauer, 1999
 *    Leif Kornstaedt, 1999
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

#include "startup.hh"

//
// Summary of how oztool behaves:
//
// oztool -gnu c++ -c <files>
//    g++ -I"$OZHOME/include" -c <files>
// oztool -gnu cc -c <files>
//    gcc -I"$OZHOME/include" -c <files>
// oztool -gnu ld -o <target> <file1> ... <filen>
//    dlltool --def emulator.def --output-lib <tmpfile1>.a
//    dlltool --dllname <target> --output-exp <tmpfile2>.a \
//       <file1> ... <filen>
//    dllwrap -s -o <target> --dllname <target> <file1> ... <filen> \
//       <tmpfile2>.a <tmpfile1>.a
//    rm tmpfile1.a tmpfile2.a
//
// oztool -msvc c++ -c <files> -o <outfile>
//    cl -nologo -TP -I"$OZHOME\include" -c <files>
//    // where `-o <outfile>' within <files> is converted to `/Fo<outfile>'
// oztool -msvc cc -c <files>
//    cl -nologo -TC -I"$OZHOME\include" -c <files>
//    // where `-o <outfile>' within <files> is converted to `/Fo<outfile>'
// oztool -msvc ld -o <target> <file1> ... <filen>
//    lib /nologo /def:$OZHOME\include\emulator.def /machine:ix86 \
//       /out:<tmpfile>.lib
//    link /nologo /dll /out:<target> <file1> ...<filen> <tmpfile>.lib
//    rm <tmpfile>.lib <tmpfile>.exp
//
// oztool -watcom c++ -c <files>
//    wpp386 -zq -bd -I"$OZHOME/include" -c <files>
// oztool -watcom cc -c <files>
//    wcc386 -zq -bd -I"$OZHOME/include" -c <files>
// oztool -watcom ld -o <target> <file1> ... <filen>
//    wlink system nt_dll initinstance terminstance name <target> \
//       file <file1>,...,<filen>
//

bool console = true;

void usage()
{
  fprintf(stderr,
	  "Usage:\n"
	  "\toztool [-verbose] [-gnu|-msvc|-watcom] c++ SourceFile\n"
	  "\toztool [-verbose] [-gnu|-msvc|-watcom] cc SourceFile\n"
	  "\toztool [-verbose] [-gnu|-msvc|-watcom] ld -o TargetLib FileList\n"
	  "\toztool platform\n");
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
  return tmpnam(NULL);
}

void doexit(int n)
{
  if (n != 0)
    fprintf(stderr,"*** error: oztool aborted with code %d\n",n);
  exit(n);
}

int verbose = 0;

int execute(char **argv)
{
  char buffer[4096];
  char *aux = buffer;
  while (*argv) {
    *aux++ = '\"';
    strcpy(aux,*argv);
    aux += strlen(*argv++);
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
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
  if (ret == FALSE) {
    panic(true,"Cannot run '%s'.\n",buffer);
  }
  WaitForSingleObject(pi.hProcess,INFINITE);

  DWORD code;
  if (GetExitCodeProcess(pi.hProcess,&code) != FALSE)
    return code;
  else
    return 0;

}

enum system_type {
  SYS_GNU, SYS_MSVC, SYS_WATCOM
};

int main(int argc, char** argv)
{
  if (argc < 2)
    usage();

  if (!strcmp(argv[1],"platform")) {
    if (argc != 2)
      usage();
    printf("%s\n",ozplatform);
    exit(0);
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

  char *ozhome = getOzHome(false);
  if ((!strcmp(argv[1],"cc") || !strcmp(argv[1],"c++")) && argc > 2) {
    int cxx = !strcmp(argv[1],"c++");
    int r = 0;
    char **wc = new char*[argc + 2];
    switch (sys) {
    case SYS_GNU:
      if (cxx)
	wc[r++] = "g++";
      else
	wc[r++] = "gcc";
      break;
    case SYS_MSVC:
      wc[r++] = "cl.exe";
      wc[r++] = "-nologo";
      if (cxx)
	wc[r++] = "-TP";
      else
	wc[r++] = "-TC";
      break;
    case SYS_WATCOM:
      if (cxx)
	wc[r++] = "wpp386";
      else
	wc[r++] = "wcc386";
      wc[r++] = "-zq";
      wc[r++] = "-bd";
      break;
    }
    wc[r++] = concat("-I",concat(ozhome,"\\include"));
    wc[r++] = "-DWINDOWS";
    for (int i = 2; i < argc; i++) {
      if (sys == SYS_MSVC && !strcmp(argv[i],"-o")) {
	wc[r++] = concat("/Fo",argv[++i]);
      }else {
	wc[r++] = argv[i];
      }
    }
    wc[r] = NULL;
    r = execute(wc);
    doexit(r);
  } else if (!strcmp(argv[1],"ld") && argc >= 4 && !strcmp(argv[2],"-o")) {
    if (argc == 4) {
      usage("Missing object files.\n");
    }
    char *target = argv[3];
    switch (sys) {
    case SYS_GNU:
      {
	char *libname  = argv[3];
	char *defname  = concat(libname,".def");
	char *aname    = concat("lib",concat(libname,".a"));
	char *tempfile = concat(ostmpnam(),".o");

	char **gcc     = new char*[7];
	gcc[0]="gcc";
	gcc[1]="-I.";//oz_include();
	gcc[3]="mozart.c";//get_mozart_c();
	gcc[2]="-c";
	gcc[4]="-o";
	gcc[5]=tempfile;
	gcc[6]=NULL;
	int r = execute(gcc);
	if (!r) {
	  char **dlltool = new char*[argc+4];
	  dlltool[r++]="dlltool";
	  dlltool[r++]="--output-def";
	  dlltool[r++]=defname;
	  dlltool[r++]="--dllname";
	  dlltool[r++]=libname;
	  dlltool[r++]="--output-lib";
	  dlltool[r++]=aname;
	  dlltool[r++]=tempfile;
	  for (int i=4; i<argc; i++) dlltool[r++]=argv[i];
	  dlltool[r]=NULL;
	  r = execute(dlltool);
	  if (!r) {
	    char **dllwrap = new char*[argc+9];
	    dllwrap[r++]="dllwrap";
	    dllwrap[r++]="-s";
	    dllwrap[r++]="-o";
	    dllwrap[r++]=libname;
	    dllwrap[r++]="--def";
	    dllwrap[r++]=defname;
	    dllwrap[r++]="--dllname";
	    dllwrap[r++]=libname;
	    dllwrap[r++]=tempfile;
	    for (int i=4; i<argc; i++) dllwrap[r++]=argv[i];
	    dllwrap[r]=NULL;
	    r = execute(dllwrap);
	  }
	}
	unlink(tempfile);
	unlink(aname);
	unlink(defname);
	doexit(r);
      }
    case SYS_MSVC:
      {
	char *tmpfile1 = "emulator";//--**ostmpnam();
	char *tmpfile1lib = concat(tmpfile1,".lib");
	char *tmpfile1exp = concat(tmpfile1,".exp");
	char **libCmd = new char *[6];
	libCmd[0] = "lib.exe";
	libCmd[1] = "/nologo";
	libCmd[2] =
	  concat("/def:",concat(ozhome,"\\include\\emulator.def"));
	libCmd[3] = "/machine:ix86";
	libCmd[4] = concat("/out:",tmpfile1lib);
	libCmd[5] = NULL;
	int r = execute(libCmd);
	if (!r) {
	  char **linkCmd = new char *[argc + 1];
	  r = 0;
	  linkCmd[r++] = "link.exe";
	  linkCmd[r++] = "/nologo";
	  linkCmd[r++] = "/dll";
	  linkCmd[r++] = concat("/out:",target);
	  for (int i = 4; i < argc; i++)
	    linkCmd[r++] = argv[i];
	  linkCmd[r++] = tmpfile1lib;
	  linkCmd[r] = NULL;
	  r = execute(linkCmd);
	}
	unlink(tmpfile1lib);
	unlink(tmpfile1exp);
	doexit(r);
      }
    case SYS_WATCOM:
      {
	char *tempfile=concat(ostmpnam(),".obj");
	char **wuergs = new char*[7];
	wuergs[0]="wcc386";
	wuergs[1]="-zq";
	wuergs[2]="-bd";
	wuergs[3]="-I.";//oz_include();
	wuergs[4]="mozart.c";//get_mozart_c();
	wuergs[5]=concat("-fo=",tempfile);
	wuergs[6]=NULL;
	int r = execute(wuergs);
	if (!r) {
	  char **links = new char*[10];
	  char *libname = argv[3];
	  links[0]="wlink";
	  links[1]="system";
	  links[2]="nt_dll";
	  links[3]="initinstance";
	  links[4]="terminstance";
	  links[5]="name";
	  links[6]=libname;
	  links[7]="file";
	  links[8]=concat(tempfile,concat(",",commaList(argv,4,argc)));
	  links[9]=NULL;
	  r=execute(links);
	}
	unlink(tempfile);
	doexit(r);
      }
    }
  }
  usage();
}
