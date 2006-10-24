#include "base.hh"
#include "value.hh"
#include "mozart.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

class Process : public OZ_Extension {
public:
  int pid;
  OZ_Term status;
  Process(pid_t i,OZ_Term s):OZ_Extension(),pid(i),status(s){}
  Process(Process&);
  // Extension
  static  int id;
  virtual int        getIdV() { return id; }
  virtual OZ_Term    typeV()  { return OZ_atom("process"); }
  virtual OZ_Term    printV(int depth = 10);
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void) { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void);
  virtual void sCloneRecurseV(void) { Assert(0); }
};

OZ_Term all_processes;

int Process::id;

inline Bool oz_isProcess(OZ_Term t)
{
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==Process::id;
}

Bool OZ_isProcess(OZ_Term t)
{ return oz_isProcess(oz_deref(t)); }

inline Process* tagged2Process(OZ_Term t)
{
  Assert(oz_isProcess(t));
  return (Process*) OZ_getExtension(t);
}

Process* OZ_toProcess(OZ_Term t)
{ return tagged2Process(oz_deref(t)); }

OZ_Term Process::printV(int)
{
  return OZ_pair2(OZ_atom("<process "),
		  OZ_pair2(OZ_int((pid<0)?-pid:pid),
			   OZ_isVariable(status) ?
			   OZ_atom(" running>") :
			   OZ_pair2(OZ_atom(" exit("),
				    OZ_pair2(status,OZ_atom(")>")))));
}

OZ_Extension* Process::gCollectV()
{
  return new Process(pid,status);
}

void Process::gCollectRecurseV()
{
  OZ_gCollect(&status);
}

typedef struct { int from; int to; } fdPair;

extern void addChildProc(pid_t);

#ifdef DEBUG_SUBPROCESSES
#define PDEBUG(X) X
#else
#define PDEBUG(X)
#endif

Process* makeProcess(char* const argv[],
		     fdPair map[],int n)
{
  pid_t pid;
  pid = fork();
  if (pid<0) return 0;
  if (pid==0) {
    // CHILD
    //
    // install IO redirections
    //
    // every `from' that appears as a `to' needs duping
    for(int i=0;i<n;i++) {
      int from = map[i].from;
      for (int j=0;j<n;j++)
	if (from==map[j].to) {
	  PDEBUG(cerr << "==> DUP " << from;)
	  map[i].from = dup(from);
	  PDEBUG(cerr << " -> " << map[i].from << endl;)
	  break;
	}
    }
    // dup2(from,to) for each entry
    for(int i=0;i<n;i++) {
      PDEBUG(cerr << "==> DUP2 " << map[i].from << " -> " << map[i].to;)
      int ret = dup2(map[i].from,map[i].to);
      PDEBUG(cerr << ((ret<0)?" FAILED":"") << endl;)
    }
    for(int i=0;i<n;i++) {
      PDEBUG(cerr << "==> CLOSE " << map[i].from << endl;)
      close(map[i].from);
    }
    //
    // exec program
    //
    execvp(argv[0],argv);
    fprintf(stderr,"execvp failed: %s\n",argv[0]);
    exit(-1);
  } else {
    // PARENT
    addChildProc(pid);
    return new Process(pid,OZ_newVariable());
  }
}

OZ_BI_define(process_make,3,1)
{
  //{Process.make CMD ARGS IOMAP}
  //
  // IOMAP is a list of pairs of integers FROM#TO
  // indicating that file descriptor FROM must be duped into
  // file descriptor TO
  //
  // we assume that the caller has verified that
  // - all arguments are fully instantiated
  // - CMD is a virtual string
  // - ARGS is a list of virtual strings
  // - IOMAP is a list of pairs of non-negative integers
  // it's easier to check this in Oz than in C++
  OZ_declareTerm(0,CMD);
  OZ_declareTerm(1,ARGS);
  OZ_declareTerm(2,IOMAP);
  //
  // create argv
  //
  int args_len = 0;
  OZ_Term list;
  list = ARGS;
  while (!OZ_isNil(list)) { args_len++; list=OZ_tail(list); }
  char**argv = (char**) malloc((2+args_len) * sizeof(char*));
  list = ARGS;
  argv[0] = strdup(OZ_virtualStringToC(CMD,0));
  for(int i=1;i<=args_len;i++,list=OZ_tail(list))
    argv[i] = strdup(OZ_virtualStringToC(OZ_head(list),0));
  argv[args_len+1] = 0;
  //
  // create the iomap
  //
  int iomap_len = 0;
  list = IOMAP;
  while (!OZ_isNil(list)) { iomap_len++; list=OZ_tail(list); }
  fdPair *iomap = new fdPair[iomap_len];
  list = IOMAP;
  for(int i=0;i<iomap_len;i++,list=OZ_tail(list)) {
    OZ_Term p = OZ_head(list);
    iomap[i].from = OZ_intToC(OZ_getArg(p,0));
    iomap[i].to   = OZ_intToC(OZ_getArg(p,1));
  }
  //
  // create the process value
  //
  Process* p = makeProcess(argv,iomap,iomap_len);
  //
  // free temp arrays
  //
  for(int i=0;i<=args_len;i++) free(argv[i]);
  free(argv);
  delete iomap;
  if (p==0)
    return OZ_raiseErrorC("process",1,OZ_atom("forkFailed"));
  //
  // register process
  //
  PDEBUG(cerr << "REGISTERING CHILD: " << p->pid << endl;)
  OZ_Term proc = OZ_extension(p);
  all_processes = OZ_cons(proc,all_processes);
  OZ_RETURN(proc);
} OZ_BI_end

OZ_BI_define(process_is,1,1)
{
  OZ_declareDetTerm(0,X);
  OZ_RETURN_BOOL(OZ_isProcess(X));
} OZ_BI_end

void process_child_handler()
{
  PDEBUG(cerr << "[BEGIN] Process Child Handler" << endl;)
  OZ_Term l = all_processes;
  while (!OZ_isNil(l)) {
    Process*p = OZ_toProcess(OZ_head(l));
    l = OZ_tail(l);
    int status;
    if (p->pid<0) continue;
    waitpid(p->pid,&status,WNOHANG);
    if      (WIFEXITED(  status)) { status = WEXITSTATUS(status); }
    else if (WIFSIGNALED(status)) { status = -(WTERMSIG(status)); }
    else continue;
    PDEBUG(cerr << "\tprocess done: " << p->pid << endl;)
    p->pid = -(p->pid);
    // Assert(oz_onTopLevel());
    OZ_unifyInThread(p->status,OZ_int(status));
  }
  PDEBUG(cerr << "[END] Process Child Handler" << endl;)
}

#ifdef DENYS_EVENTS
OZ_BI_define(process_handler,0,0)
{
  process_child_handler();
  return PROCEED;
} OZ_BI_end
#endif

OZ_BI_define(process_dropDead,0,0)
{
  OZ_Term l1 = all_processes;
  OZ_Term l2 = OZ_nil();
  PDEBUG(cerr << "[BEGIN] CHILD HANDLER" << endl;)
  while (!OZ_isNil(l1)) {
    OZ_Term head = OZ_head(l1);
    Process*p = OZ_toProcess(head);
    // pid==-1 indicates a dead process
    if (p->pid>=0) {
      l2=OZ_cons(head,l2);
      PDEBUG(cerr << "\tkeeping " << (p->pid) << endl;)
    } else {
      PDEBUG(cerr << "\tdropping " << -(p->pid) << endl;)
    }
    l1 = OZ_tail(l1);
  }
  PDEBUG(cerr << "[END] CHILD HANDLER" << endl;)
  all_processes=l2;
  return PROCEED;
} OZ_BI_end

#define OZ_declareProcess(ARG,VAR) \
OZ_declareType(ARG,VAR,Process*,"process", \
	       OZ_isProcess,OZ_toProcess)

OZ_BI_define(process_status,1,1)
{
  OZ_declareProcess(0,P);
  OZ_RETURN(P->status);
} OZ_BI_end

OZ_BI_define(process_kill,2,0)
{
  OZ_declareProcess(0,P);
  OZ_declareInt(    1,N);
  if (P->pid>=0) kill(P->pid,N);
  return PROCEED;
} OZ_BI_end

#ifndef DENYS_EVENTS
extern void (*oz_child_handle)();
#endif

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"make"		,3,1,process_make},
      {"is"		,1,1,process_is},
      {"dropDead"	,0,0,process_dropDead},
      {"status"		,1,1,process_status},
      {"kill"		,2,0,process_kill},
#ifdef DENYS_EVENTS
      {"handler"	,0,0,process_handler},
#endif
      {0,0,0,0}
    };
    Process::id = oz_newUniqueId();
    all_processes = OZ_nil();
    OZ_protect(&all_processes);
#ifndef DENYS_EVENTS
    oz_child_handle = process_child_handler;
#endif
    return i_table;
  }
} /* extern "C" */
