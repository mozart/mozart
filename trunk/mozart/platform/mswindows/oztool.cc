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

#define OZENGINE
#include "misc.cc"

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
  char buffer[5000];
  GetModuleFileName(NULL, buffer, sizeof(buffer));
  char *ret = getOzHome(buffer,2);
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

void usage(char *msg) 
{
  fprintf(stderr,
	  "%s"
	  "\nUsage:\n"
	  "\toztool [-gnu|-watcom|-msvc] ld -o TargetLib FileList \n"
	  "\toztool [-gnu|-watcom|-msvc] c++ SourceFile\n"
	  "\toztool [-gnu|-watcom|-msvc] c   SourceFile\n"
	  "\toztool platform\n",
	  msg);
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

#define SYS_GNU  1
#define SYS_WAT  2
#define SYS_MSVC 3

int main(int argc, char** argv) 
{
  if (argc<2) 
    usage("");

  if (!strcmp(argv[1],"platform")) {
    if (argc!=2)
      usage("");
    printf("%s\n",ozplatform);
    exit(0);
  }

  if (argc<3)
    usage("");

  int sys = SYS_GNU;
  if (!strcmp(argv[1],"-watcom")) {
    sys = SYS_WAT;
    argv++; argc--;
  } else if (!strcmp(argv[1],"-msvc")) {
    sys = SYS_MSVC;
    argv++; argc--;
  } else if (!strcmp(argv[1],"-gnu")) {
    sys = SYS_GNU;
    argv++; argc--;
  }

  if (!strcmp(argv[1],"ld")) {
    if (sys == SYS_WAT) {
      if (argc>=4 && !strcmp(argv[2],"-o")) {
	if (argc==4) {
	  usage("Missing Object Files.\n");
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
	  char *libname = argv[3];
	  links[0]="wlink";
	  links[1]="system";
	  links[2]="nt_dll";
	  links[3]="initinstance";
	  links[4]="terminstance";
	  links[5]="name";
	  links[6]=libname;
	  links[7]="file";
	  links[8]=concat(tempfile,concat(",",pappzammn(argv,4,argc)));
	  links[9]=NULL;
	  r=execute(links);
	}
      cleanup:
	unlink(tempfile);
	doexit(r);
      }
    }
    if (sys == SYS_MSVC) {
      if (argc>=4 && !strcmp(argv[2],"-o")) {
	if (argc==4) {
	  usage("Missing Object Files.\n");
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
	  char *libname = argv[3];
	  r = 0;
	  links[r++]="link";
	  links[r++]="-DLL";
	  links[r++]=tempfile;
	  for (int i=4; i<argc; i++) links[r++]=argv[i];
	  links[r++]=concat("-OUT:",libname);
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
    if (sys == SYS_GNU) {
      if (argc>=4 && !strcmp(argv[2],"-o")) {
	if (argc==4) {
	  usage("Missing Object Files.\n");
	}
	char **wuergs  = new char*[argc+4];
	char **links   = new char*[argc+9];
	char **moz     = new char*[7];
	char *libname  = argv[3];
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
    if (argc<3)
      usage("");
    int cxx = (strcmp(argv[1],"c++")==0);
    int r=0;
    char ** wc=new char*[argc+1];
    switch (sys) {
    case SYS_WAT:
      wc[r++]= cxx ? "wpp386" : "wcc386";
      wc[r++]="-zq";
      wc[r++]="-bd";
      break;
    case SYS_GNU:
      wc[r++]=cxx ? "g++" : "gcc";
      break;
    case SYS_MSVC:
      wc[r++]="cl";
      break;
    }
    wc[r++]=oz_include();
    for (int i=2; i<argc; i++) wc[r++]=argv[i];
    wc[r]=NULL;
    r=execute(wc);
    exit (r);
  }
  usage("");
}
