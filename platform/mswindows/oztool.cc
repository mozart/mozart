/*
 *  Authors:
 *    Ralf Scheidhauer (scheidhr@dfki.de)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

// #include "misc.cc"
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char *ozplatform = "win32-i486";

char *getParent(char *path, int levelsup)
{
  char *ret = strdup(path);
  int i;
  char *aux;
  for(i=0; i<levelsup; i++) {
    aux=strrchr(ret,'\\');
    if (aux==NULL) {
      return NULL;
    }
    *aux = '\0';
  }
  return ret;
}

void dossify(char *path)
{
  char *aux;
  for(aux=path; *aux; aux++) {
    if (*aux == '/') {
      *aux = '\\';
    }
  }
}

char *getOzHome()
{
  char *ret = getenv("OZHOME");
  if (ret!=NULL) {
    ret = strdup(ret);
  } else {
    char buffer[5000];
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    ret = getParent(buffer,2);
    if (ret == NULL) {
      fprintf(stderr,
	      "Cannot determine Oz installation directory.\n"
	      "Try setting OZHOME environment variable.\n");
      exit(2);
    }
  }
  dossify(ret);
  return ret;
}


char *pappzammn(char **argv, int von, int bis) 
{
  int l=0;
  for (int i=von; i<bis; i++) 
    l += strlen(argv[i])+strlen(",");
  char *ret= new char[l+1];
  if (ret==NULL) exit(2);
  ret[0]='\0';
  while (von<bis) {
    strcat(ret,argv[von++]);
    if (von<bis) strcat(ret, ",");
  }
  return ret;
}

void usage() 
{
  fprintf(stderr,"oztool for Win32 (November 1998)\n\
oztool:\n\
\tld  -gnu -o TargetLib FileList \n\
\tld  -watcom -o TargetLib FileList \n\
\tld  -msvc -o TargetLib FileList \n\
\tc++ -gnu -c Source1.cc [SourceFiles.c ...]\n\
\tc++ -watcom -c Source.cc\n\
\tc++ -msvc -c Source.cc\n\
\tc   -gnu -c Source1.c [SourceFiles.c ...]\n\
\tc   -watcom -c Source.c\n\
\tc   -msvc -c Source.cc\n\
\tozplatform\n");
  exit(2);
}
 
char * concat(const char * st1, const char * st2) 
{
  int i = (strlen(st1)+strlen(st2)+1)*sizeof(char);
  char *ret = new char[i];
  if (ret==NULL) {
    fprintf(stderr,"concat(%s,%s): malloc failed\n",st1,st2);
    exit(1);
  }

  strcpy(ret, st1);
  strcat(ret, st2);
  return ret;
}

char *oz_include() 
{
  return concat(concat("-I",getOzHome()),"\\include");
}

char * get_mozart_c() 
{
  return concat(getOzHome(),"\\include\\mozart.c");
}

char *ostmpnam()
{
  return tmpnam(NULL);
}


void doexit(int n)
{
  if (n!=0)
    fprintf(stderr,"*** error\n");
  exit(n);
}

int execute(char **argv)
{
  char **aux = argv;
  while(*aux) {
    printf("%s ",*aux);
    aux++;
  }
  printf("\n",argv);
  return spawnvp(P_WAIT,argv[0],argv);
}

int main(int argc, char** argv) 
{
  if (argc<2) 
    usage();

  if (!strcmp(argv[1],"ozplatform")) {
    if (argc!=2)
      usage();
    printf("%s\n",ozplatform);
    exit(0);
  }

  if (!strcmp(argv[1],"ld")) {
    if (argc>=3 && !strcmp(argv[2],"-watcom")) {
      if (argc>=5 && !strcmp(argv[3],"-o")) {
	if (argc==5) {
	  fprintf(stderr,"Missing Object Files.\n");
	  usage();
	}
	/* ld -watcom -o */
	char *tempfile=concat(ostmpnam(),".obj");
	char **wuergs = new char*[7];
	wuergs[0]="wcc386";
	wuergs[1]="-zq"; 
	wuergs[2]="-bd";
	wuergs[3]=oz_include();
	wuergs[4]=get_mozart_c();
	wuergs[5]=concat("-fo=",tempfile);
	wuergs[6]=NULL;
	int r=execute(wuergs);
	if (r)
	  goto cleanup;
	{
	  char **links = new char*[10];
	  char *libname = argv[4];
	  links[0]="wlink";
	  links[1]="system";
	  links[2]="nt_dll";
	  links[3]="initinstance";
	  links[4]="terminstance";
	  links[5]="name";
	  links[6]=libname;
	  links[7]="file";
	  links[8]=concat(tempfile,concat(",",pappzammn(argv,5,argc)));
	  links[9]=NULL;
	  r=execute(links);
	}
      cleanup:
	unlink(tempfile);
	doexit(r);
      }
    }
    if (argc>=3 && !strcmp(argv[2],"-msvc")) {
      if (argc>=5 && !strcmp(argv[3],"-o")) {
	if (argc==5) {
	  fprintf(stderr,"Missing Object Files.\n");
	  usage();
	}
	/* ld -msvc -o */
	char *tempfile=concat(ostmpnam(),".obj");
	char *junklib = concat(ostmpnam(),".lib");
	char *junkexp = concat(ostmpnam(),".exp");
	char **wuergs = new char*[7];
	wuergs[0]="cl";
	wuergs[1]="-nologo"; 
	wuergs[2]="-c"; 
	wuergs[3]=oz_include();
	wuergs[4]=get_mozart_c();
	wuergs[5]=concat("-Fo",tempfile);
	wuergs[6]=NULL;
	int r=execute(wuergs);
	if (r)
	  goto cleanup;
	{
	  char **links = new char*[argc+9];
	  char *libname = argv[4];
	  r = 0;
	  links[r++]="cl";
	  links[r++]="-LD";
	  links[r++]=tempfile;
	  for (int i=5; i<argc; i++) links[r++]=argv[i];
	  links[r++]=concat("-Fe",libname);
	  links[r++]="-link";
	  links[r++]=concat("-IMPLIB:",junklib);
	  links[r]=NULL;
	  r=execute(links);
	}
      cleanup3:
	unlink(tempfile);
	unlink(junklib);
	unlink(junkexp);
	doexit(r);
      }
    }
    if (argc>=3 && !strcmp(argv[2],"-gnu")) {
      if (argc>=5 && !strcmp(argv[3],"-o")) {
	if (argc==5) {
	  fprintf(stderr,"Missing Object Files.\n");
	  usage();
	}
	char **wuergs  = new char*[argc+4];
	char **links   = new char*[argc+9];
	char **moz     = new char*[7];
	char *libname  = argv[4];
	char *defname  = concat(libname,".def");
	char *aname    = concat("lib",concat(libname,".a"));
	char *tempfile = concat(ostmpnam(),".o");

	moz[0]="gcc";
	moz[1]=oz_include();
	moz[3]=get_mozart_c();
	moz[2]="-c";
	moz[4]="-o";
	moz[5]=tempfile;
	moz[6]=NULL;

	int r=0;
	wuergs[r++]="dlltool";
	wuergs[r++]="--output-def";
	wuergs[r++]=defname;
	wuergs[r++]="--dllname";
	wuergs[r++]=libname;
	wuergs[r++]="--output-lib";
	wuergs[r++]=aname;
	wuergs[r++]=tempfile;
	for (int i=5; i<argc; i++) wuergs[r++]=argv[i];
	wuergs[r]=NULL;
       
	r=0;
	links[r++]="dllwrap";
	links[r++]="-s";
	links[r++]="-o";
	links[r++]=libname;
	links[r++]="--def";
	links[r++]=defname;
	links[r++]="--dllname";
	links[r++]=libname;
	links[r++]=tempfile;
	for (int i=5; i<argc; i++) links[r++]=argv[i];
	links[r]=NULL;

	if ((r=execute(moz)) ||
	    (r=execute(wuergs)) ||
	    (r=execute(links))) {
	  goto cleanup2;
	}
      cleanup2:
	unlink(tempfile);
	unlink(aname);
	unlink(defname);
	doexit(r);
      }
    }
  }
  else if (strcmp(argv[1],"c")==0 || strcmp(argv[1],"c++")==0) {
    if (argc<4)
      usage();
    int cxx = (strcmp(argv[1],"c++")==0);
    int r=0;
    char ** wc=new char*[argc+1];
    if (!strcmp(argv[2],"-watcom")) {
      wc[r++]= cxx ? "wpp386" : "wcc386";
      wc[r++]="-zq";
      wc[r++]="-bd";
    } else if (!strcmp(argv[2],"-gnu")) {
      wc[r++]=cxx ? "g++" : "gcc";
    } else if (!strcmp(argv[2],"-msvc")) {
      wc[r++]="cl";
    } else {
      usage();
    }
    wc[r++]=oz_include();
    for (int i=3; i<argc; i++) wc[r++]=argv[i];
    wc[r]=NULL;
    r=execute(wc);
    exit (r);
  }
  usage();
}
