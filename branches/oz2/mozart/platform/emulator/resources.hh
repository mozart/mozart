#define COMPILER_TIMEOUT	100 /* ms */

#define IO_BUF_SIZE		10000

#define PRINT_DEPTH		2
#define PRINT_WIDTH		10
#define ERROR_PRINT_DEPTH	PRINT_DEPTH
#define ERROR_PRINT_WIDTH	PRINT_WIDTH
#define ERROR_THREAD_DEPTH      10
#define ERROR_LOCATION          1
#define ERROR_DEBUG             1
#define ERROR_HINTS             1

#define TIME_SLICE		50  /* ms */

#define HI_PRIORITY             2
#define MID_PRIORITY            1
#define LOW_PRIORITY            0

#define DEFAULT_PRIORITY	MID_PRIORITY
#define SYSTEM_PRIORITY		HI_PRIORITY
#define PROPAGATOR_PRIORITY	MID_PRIORITY

#define OZMAX_PRIORITY		HI_PRIORITY
#define OZMIN_PRIORITY		LOW_PRIORITY

#define DEFAULT_HI_MID_RATIO    10
#define DEFAULT_MID_LOW_RATIO   10

#define SHOW_FAST_LOAD		0
#define SHOW_FOREIGN_LOAD	0
#define SHOW_IDLE_MESSAGE	0
#define SHOW_SUSPENSION		0
#define STOP_ON_TOPLEVEL_FAILURE	0

#define OZ_PATH		".:/usr/local/oz/lib"

#define GC_FLAG		1
#define GC_VERBOSITY	1

#define STACKMAXSIZE	10000

#define HEAPMAXSIZE	     1048576 /* kByte   */
#define HEAPMINSIZE	        1024 /* kByte   */
#define HEAPFREE	          75 /* percent */
#define HEAPTOLERANCE	          20 /* percent */
#define INITIALHEAPTHRESHOLD	2048 /* kByte   */
#define HEAPBLOCKSIZE	     1048576 /* byte    */

#define CLOCK_TICK	        10000  /* usec */
#define TASK_STACK_SIZE		10
#define THREAD_QUEUE_SIZE	256
#define NUM_TOPLEVEL_VARS	10000

#define NumberOfXRegisters	10000

#define OzCompiler	"oz.compiler"

