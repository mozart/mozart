#define KOSTJA_MAGIC 0x6b6f7374

#define COMPILER_TIMEOUT        100 /* ms */

#define IO_BUF_SIZE             10000

#define PRINT_DEPTH             2
#define PRINT_WIDTH             10
#define ERROR_PRINT_DEPTH       2
#define ERROR_PRINT_WIDTH       10
#define ERROR_THREAD            1
#define ERROR_LOCATION          1
#define ERROR_HINTS             1

#define TIME_SLICE              50  /* ms */

#define OZMIN_PRIORITY          0
#define DEFAULT_PRIORITY        50
#define PROPAGATOR_PRIORITY     50
#define SYSTEM_PRIORITY         100
#define OZMAX_PRIORITY          100


#define SHOW_FAST_LOAD          0
#define SHOW_FOREIGN_LOAD       0
#define SHOW_IDLE_MESSAGE       0
#define SHOW_SUSPENSION         0
#define STOP_ON_TOPLEVEL_FAILURE        0

#define OZ_PATH         ".:/usr/local/oz/lib"

#define GC_FLAG         1
#define GC_VERBOSITY    1

#define STACKMAXSIZE    10000

#define HEAPMAXSIZE          1048576 /* kByte   */
#define HEAPMINSIZE             1024 /* kByte   */
#define HEAPFREE                  75 /* percent */
#define HEAPTOLERANCE             20 /* percent */
#define INITIALHEAPTHRESHOLD    4096 /* kByte   */
#define HEAPBLOCKSIZE        1048576 /* byte    */

#define CLOCK_TICK      10000   /* usec */
#define TASK_STACK_SIZE         10
#define THREAD_QUEUE_SIZE       256
#define NUM_TOPLEVEL_VARS       10000

#define NumberOfXRegisters      10000

#define NameOfNil       "nil"
#define NameOfVoid      "_"
#define NameOfCons      "|"
#define NameOfPair      "#"
#define NameOfBool      "bool"
#define NameOfSup       "sup"
#define NameOfCompl     "compl"
#define NameOfUnknown   "unknown"

#define OzCompiler      "oz.compiler"
