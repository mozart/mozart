/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

/*
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
 * appropriate handbook chapter: system/system.raw.
 */

#define COMPILER_TIMEOUT        180 /* seconds */

#define IO_BUF_SIZE             10000

#define PRINT_DEPTH             2
#define PRINT_WIDTH             10
#define ERROR_PRINT_DEPTH       PRINT_DEPTH
#define ERROR_PRINT_WIDTH       PRINT_WIDTH
#define ERROR_THREAD_DEPTH      50
#define ERROR_LOCATION          1
#define ERROR_DEBUG             1
#define ERROR_HINTS             1

#define TIME_SLICE              50  /* ms */
#define CLOCK_TICK              TIME_SLICE*(1000/5)     /* usec */


#define HI_PRIORITY             2
#define MID_PRIORITY            1
#define LOW_PRIORITY            0

#define DEFAULT_PRIORITY        MID_PRIORITY
#define SYSTEM_PRIORITY         HI_PRIORITY
#define PROPAGATOR_PRIORITY     MID_PRIORITY

#define OZMAX_PRIORITY          HI_PRIORITY
#define OZMIN_PRIORITY          LOW_PRIORITY

#define DEFAULT_HI_MID_RATIO    10
#define DEFAULT_MID_LOW_RATIO   10

#define DEFAULT_TRAIL_SIZE      200

#define SHOW_FAST_LOAD          0
#define SHOW_FOREIGN_LOAD       0
#define SHOW_IDLE_MESSAGE       0
#define SHOW_SUSPENSION         0
#define STOP_ON_TOPLEVEL_FAILURE        0

#define OZ_PATH         ".:/usr/local/oz/lib"

#define GC_FLAG         1
#define GC_VERBOSITY    0

#define RESIZESTACKMINSIZE 64 /* used for resizing a stack */

#define TASKFRAMESIZE   3
#define STACKMAXSIZE  8192 /* tasks */
#define STACKMINSIZE    4    /* tasks */

#define HEAPMAXSIZE        64 * 1024 /* kByte   */
#define HEAPMINSIZE             1024 /* kByte   */
#define HEAPFREE                  75 /* percent */
#define HEAPTOLERANCE             20 /* percent */
#define INITIALHEAPTHRESHOLD    2048 /* kByte   */
#ifdef CS_PROFILE
#define HEAPBLOCKSIZE        8*1048576 /* byte    */
#else
#define HEAPBLOCKSIZE        1048576 /* byte    */
#endif

#define QUEUEMINSIZE            32

#define TIMEDETAILED            0

#define NUM_TOPLEVEL_VARS       10000

#define NumberOfXRegisters      10000

/* threads */
#define THREAD_ID_SIZE          16
#define THREAD_ID_MAX           ((1 << THREAD_ID_SIZE) - 1)
#define THREAD_ID_MASK          THREAD_ID_MAX

#define OzCompiler      "oz.compiler"

#define MAX_TCP_CACHE     5
#define MAX_UDP_PACKET    100*1024  /* 100k */
#define TCP_PACKET_SIZE   1*1024    /* 1k */


/*
 * Switches
 */

/* if defined: variables are stored directly in structure */
#define OPT_VAR_IN_STRUCTURE

/* if defined the builtin print long is available to print detailed
 * informations about values */
#ifndef DEBUG_PRINT
# define DEBUG_PRINT
#endif

/* all debug switches for the emulator */
#ifdef DEBUG_EMULATOR

#define DEBUG_CHECK     // enable assertions

#ifdef DEBUG_PRINT
# define DEBUG_TRACE    // MM: enable low level debugging: step instructions
#endif

//#define SLOW_DEBUG_CHECK
//#define DEBUG_GC
//#define DEBUG_VERBOSE
//#define RECINSTRFETCH 500

#define DEBUG_FD
//#define DEBUG_FDCD

#define DEBUG_DET       // use counter instead of alarm timer for scheduling

//#define PROFILE_FD
#define DEBUG_MEM

#define OUTLINE         // do not inline functions

//#define DEBUG_META
//#define DEBUG_STACK
//#define DEBUG_STABLE
//#define DEBUG_THREADS
#define DEBUG_FSET
//#define DEBUG_LTQ
//#define DEBUG_INDICES
//#define DEBUG_PROPAGATORS
//#define ALLDIFF

//#define MM_DEBUG

#define DEBUG_PERDIO

//#define DEBUG_PROP_STABILTY_TEST

// test if the liveness routine for X registers works
// #define DEBUG_LIVENESS

#endif

#ifdef PROFILE_EMULATOR

// #define DISABLE_INSTRPROFILE
#define DEBUG_DET
// #define HEAP_PROFILE
// #define PROFILE_INSTR
// #define PROFILE_BI
// #define CS_PROFILE
#define OUTLINE
#endif

#ifdef RS_PROFILE
#define HEAP_PROFILE
#define OUTLINE
#endif


// avoid inlining some functions
#if defined(OUTLINE)
#define INLINE
#else
#define INLINE inline
#endif

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
