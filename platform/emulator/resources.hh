/* $id$
 *
 * ### #     # ######   #####  ######  #######    #    #     # #######
 *  #  ##   ## #     # #     # #     #    #      # #   ##    #    #
 *  #  # # # # #     # #     # #     #    #     #   #  # #   #    #
 *  #  #  #  # ######  #     # ######     #    #     # #  #  #    #
 *  #  #     # #       #     # #   #      #    ####### #   # #    #
 *  #  #     # #       #     # #    #     #    #     # #    ##    #
 * ### #     # #        #####  #     #    #    #     # #     #    #
 *
 * Whenever you change this file, make sure that you also change the
 * appropriate handbook chapter: system/system.raw!!!!!!!!
 */

#define COMPILER_TIMEOUT	180 /* seconds */

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
#define CLOCK_TICK	        TIME_SLICE*(1000/5)	/* usec */


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
#define GC_VERBOSITY	0

#define RESIZESTACKMINSIZE 64 /* used for resizing a stack */

#define TASKFRAMESIZE   3
#define STACKMAXSIZE  8192 /* tasks */
#define STACKMINSIZE    4    /* tasks */

#define HEAPMAXSIZE	   64 * 1024 /* kByte   */
#define HEAPMINSIZE	        1024 /* kByte   */
#define HEAPFREE	          75 /* percent */
#define HEAPTOLERANCE	          20 /* percent */
#define INITIALHEAPTHRESHOLD	2048 /* kByte   */
#define HEAPBLOCKSIZE	     1048576 /* byte    */

#define QUEUEMINSIZE	        32

#define NUM_TOPLEVEL_VARS	10000

#define NumberOfXRegisters	10000

#define MAX_ID                  (1<<15)

#define OzCompiler	"oz.compiler"

#define MAX_TCP_CACHE     5
#define MAX_UDP_PACKET    100*1024  /* 100k */
#define TCP_PACKET_SIZE   1*1024    /* 1k */


/* if defined: variables are stored directly in structure */
#define OPT_VAR_IN_STRUCTURE

/*
 *
 * ###   #     # ######  ####### ######  #######    #    #     # #######
 *  #    ##   ## #     # #     # #     #    #      # #   ##    #    #
 *  #    # # # # #     # #     # #     #    #     #   #  # #   #    #
 *  #    #  #  # ######  #     # ######     #    #     # #  #  #    #
 *  #    #     # #       #     # #   #      #    ####### #   # #    #
 *  #    #     # #       #     # #    #     #    #     # #    ##    #
 * ###   #     # #       ####### #     #    #    #     # #     #    #
 *
 * Whenever you change this file, make sure that you also change the
 * appropriate handbook chapter: system/system.raw!!!!!!!!
 */

