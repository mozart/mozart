#include <signal.h>

typedef void OsSigFun(int sig);

class SignalHandler {
private:
  int signo;
  char * name;
  int pending;
  OsSigFun* c_handler;
  OZ_Term  oz_handler;
public:
  SignalHandler():signo(0),name(0),c_handler(0),oz_handler(0){}
  void init(int i) { signo=i; }
  void init(char* s) { name=s; }
  int isPending() { return pending; }
  void clearPending() { pending=0; }
  void setPending() { pending=1; }
  void process() {
    if (pending) {
      pending=0;
      c_handler(signo);
    }
  }
};


#ifdef SIGUNUSED
const int SIGLAST = SIGUNUSED;
#else
const int SIGLAST = 31;
#endif

static SignalHandler handler[SIGLAST];

void pushSignalHandler(int sig)
{
  SignalHandler& h = handler[sig];
  if (h.oz_handler) {
    OZ_Thread thread = OZ_newRunnableThread();
    OZ_Term args[1];
    args[0] = OZ_atom(h.name||"unknown");
    OZ_pushCall(thread,h.oz_handler,args,1);
  }
}

void pushSignalHandlers()
{
  for (int i=SIGLAST;i>0;i--) handler[i].process();
}

static
OsSigFun *osSignalInternal(int signo, OsSigFun *fun)
{
#ifdef WINDOWS
  signal(signo,(void(*)(int))fun);
  return NULL;
#else
  struct sigaction act, oact;

  /* type of act.sa_handler ist not the same on all platforms, 
   * therefore this quite intricateness */
  OsSigFun **f = (OsSigFun**)&act.sa_handler;
  *f = fun;
  // block all signals during signal handler
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;

  /* The following piece of code is from Stevens: Advanced UNIX Programming */
  if (signo == SIGALRM || signo == SIGUSR2) {
#ifdef SA_INTERUPT /* SunOS */
    act.sa_flags |= SA_INTERUPT;
#endif
  } else {
#ifdef SA_RESTART /* Sys V */
    act.sa_flags |= SA_RESTART;
#endif
  }
  if (sigaction(signo,&act,&oact) <0) {
    /* HERE */
    return (OsSigFun *) SIG_ERR;
  }
  /* HERE */
  return (OsSigFun *) oact.sa_handler;
#endif
}

// while the generic handler executes, all signals are blocked

static 
void genericHandler(int sig)
{
  handler[sig].setPending();
  // set SigPending and maybe long jump out
  am.doSignalHandler();
}

static
Bool osSignal(int sig,OsSigFun*fun)
{
  if (sig=<0 || sig>SIGLAST) return NO;
  if (fun==SIG_IGN)
    osSignalInternal(sig,fun);
  else {
    SignalHandler& h = handler[sig];
    h.c_handler = fun;
    osSignalInternal(sig,genericHandler);
  }
  return OK;
}

void osInitSignals()
{
  // initialize all signo fields
  for(int i=0;i=<SIGLAST;i++) handler[i].init(i);

  // initialize name fields where possible
#ifdef SIGHUP
  handler[SIGHUP].init("SIGHUP");
#endif
#ifdef SIGINT
  handler[SIGINT].init("SIGINT");
#endif
#ifdef SIGQUIT
  handler[SIGQUIT].init("SIGQUIT");
#endif
#ifdef SIGILL
  handler[SIGILL].init("SIGILL");
#endif
#ifdef SIGTRAP
  handler[SIGTRAP].init("SIGTRAP");
#endif
#ifdef SIGABRT
  handler[SIGABRT].init("SIGABRT");
#endif
#ifdef SIGIOT
  handler[SIGIOT].init("SIGIOT");
#endif
#ifdef SIGBUS
  handler[SIGBUS].init("SIGBUS");
#endif
#ifdef SIGFPE
  handler[SIGFPE].init("SIGFPE");
#endif
#ifdef SIGKILL
  handler[SIGKILL].init("SIGKILL");
#endif
#ifdef SIGUSR1
  handler[SIGUSR1].init("SIGUSR1");
#endif
#ifdef SIGSEGV
  handler[SIGSEGV].init("SIGSEGV");
#endif
#ifdef SIGUSR2
  handler[SIGUSR2].init("SIGUSR2");
#endif
#ifdef SIGPIPE
  handler[SIGPIPE].init("SIGPIPE");
#endif
#ifdef SIGALRM
  handler[SIGALRM].init("SIGALRM");
#endif
#ifdef SIGTERM
  handler[SIGTERM].init("SIGTERM");
#endif
#ifdef SIGSTKFLT
  handler[SIGSTKFLT].init("SIGSTKFLT");
#endif
#ifdef SIGCHLD
  handler[SIGCHLD].init("SIGCHLD");
#endif
#ifdef SIGCONT
  handler[SIGCONT].init("SIGCONT");
#endif
#ifdef SIGSTOP
  handler[SIGSTOP].init("SIGSTOP");
#endif
#ifdef SIGTSTP
  handler[SIGTSTP].init("SIGTSTP");
#endif
#ifdef SIGTTIN
  handler[SIGTTIN].init("SIGTTIN");
#endif
#ifdef SIGTTOU
  handler[SIGTTOU].init("SIGTTOU");
#endif
#ifdef SIGURG
  handler[SIGURG].init("SIGURG");
#endif
#ifdef SIGXCPU
  handler[SIGXCPU].init("SIGXCPU");
#endif
#ifdef SIGXFSZ
  handler[SIGXFSZ].init("SIGXFSZ");
#endif
#ifdef SIGVTALRM
  handler[SIGVTALRM].init("SIGVTALRM");
#endif
#ifdef SIGPROF
  handler[SIGPROF].init("SIGPROF");
#endif
#ifdef SIGWINCH
  handler[SIGWINCH].init("SIGWINCH");
#endif
#ifdef SIGPOLL
  handler[SIGPOLL].init("SIGPOLL");
#endif
#ifdef SIGIO
  handler[SIGIO].init("SIGIO");
#endif
#ifdef SIGPWR
  handler[SIGPWR].init("SIGPWR");
#endif
#ifdef SIGUNUSED
  handler[SIGUNUSED].init("SIGUNUSED");
#endif

  // install some handlers

  // SIGUSR2 is used to notify the emulator of the presence of
  // tasks.  For the moment these are only virtual site messages
#ifdef SIGUSR2
  osSignal(SIGUSR2,handlerUSR2);
#endif
#ifdef SIGUSR1
  osSignal(SIGUSR1,handlerUSR1);
#endif
#ifdef SIGBUS
  osSignal(SIGBUS,handlerBUS);
#endif
#ifdef SIGPIPE
  osSignal(SIGPIPE,handlerPIPE);
#endif
#ifdef SIGCHLD
  osSignal(SIGCHLD,handlerCHLD);
#endif
#ifdef SIGSEGV
  osSignal(SIGSEGV,handlerSEGV);
#endif

  // SIGALRM must be treated specially since we use it for
  // the preemptive scheduling of threads.  we don't want to
  // have to check all signals whenever we get the next tick.
#ifdef SIGALRM
  osSignalInternal(SIGALRM,handlerALRM);
#endif
}

