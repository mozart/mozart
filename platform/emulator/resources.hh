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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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


#define PRINT_DEPTH             5
#define PRINT_WIDTH             20
#define ERROR_PRINT_DEPTH       PRINT_DEPTH
#define ERROR_PRINT_WIDTH       PRINT_WIDTH
#define ERROR_THREAD_DEPTH      50
#define ERROR_LOCATION          1
#define ERROR_DEBUG             1
#define ERROR_HINTS             1

#define TIME_SLICE              50  /* ms */
#define CLOCK_TICK              TIME_SLICE*(1000/5)     /* usec */

/* task manager */
/* 4 tasks are needed by virtual sites, see virtual.cc and one needed by ports se perdio.cc*/
#define MAXTASKS                5

#define NAMETRUE "true"
#define NAMEFALSE "false"
#define NAMEGROUPVOID "NameGroupVoid"
#define TRUEFALSEID 0


/* how to prefix errors/warnings, etc. for emacs */

#define MSG_ERROR 17
#define MSG_WARN 18
#define MSG_STATUS 19

/* how to prefix atoms/names */
#define ATOM_PREFIX 0
#define NAME_PREFIX 1
#define INTEGER_PREFIX 2

/* how to prefix arities for tuples and records  */
#define TUPLEWIDTH_PREFIX 0
#define RECORDARITY_PREFIX 1

/* how to mark EOF for scanner */
#define OZEOF 4

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
#define DEFAULT_CLOSE_TIME      0

#define DEFAULT_TRAIL_SIZE      200

#define SHOW_IDLE_MESSAGE       0
#define SHOW_SUSPENSION         0
#define STOP_ON_TOPLEVEL_FAILURE        0

#define GC_FLAG         1
#define GC_VERBOSITY    0
#define CODE_GC_CYLES   5

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

#define MAX_TCP_CACHE     5
#define MAX_UDP_PACKET    100*1024  /* 100k */
#define TCP_PACKET_SIZE   1*1024    /* 1k */

//
// Tasks manager;
// Minimal frequency in ms. Rounded up to the 'CLOCK_TICK' value;
// A minimal frequency of '0' means there is no minimal frequency;
#define DEFAULT_MIN_INTERVAL   0

//
// Distributed Oz - general;
#define PROBE_INTERVAL      1000 /* ms */

//
// Distributed Oz - virtual sites
#define PERDIO_ID       0xa3
// 128k mailbox (messages 8bytes; so 16k messages);
#define VS_MAILBOX_SIZE (128*1024)
// 128k segments - 256 chunks .5k each;
#define VS_CHUNK_SIZE   512
#define VS_CHUNKS_NUM   256
// 12,5% fill-up for 32 sites?
#define VS_REGISTER_HT_SIZE   256
//
// Wait time for "ping" probing. Note that this time should be
// considerably larger than 'PROBE_INTERVAL', since ping messages
// ('M_SITE_IS_ALIVE'/'M_SITE_ALIVE') are delivered with the same
// priority as all other messages. This time is effectively made
// at least as large as 'PROBE_INTERVAL';
#define PROBE_WAIT_TIME 30000 /* ms */
// GCing of chunk pool' segments - see vs_msgbuffer.*;
#define VS_MSGCHUNKS_USAGE         3
#define VS_SEGS_MAXIDLE_PHASES     5
// A new allocation phase starts at most after 'VS_SEGS_MAXPHASE_SECS'
// (or, in other words, GC takes places at most every XXX ms,
// regardless whether it's needed or not);
#define VS_SEGS_MAXPHASE_MS        60000

/*
 * Switches
 */

/* if defined the builtin print long is available to print detailed
 * informations about values */
#ifdef MISC_BUILTINS
# define DEBUG_PRINT
#endif

/* all debug switches for the emulator */
#ifdef DEBUG_EMULATOR

#define DEBUG_CHECK     // enable assertions

/* always define debug print for debugging */
#define DEBUG_PRINT

#define DEBUG_TRACE     // MM: enable low level debugging: step instructions

//#define SLOW_DEBUG_CHECK
//#define DEBUG_GC
//#define DEBUG_VERBOSE
#define RECINSTRFETCH 500

#define DEBUG_FD
#define DEBUG_FD_CONSTRREP
#define DEBUG_FSET
#define DEBUG_FSET_CONSTRREP

//#define DEBUG_FDCD

#define DEBUG_DET       // use counter instead of alarm timer for scheduling

//#define PROFILE_FD
#define DEBUG_MEM

#define OUTLINE         // do not inline functions

//#define DEBUG_META
//#define DEBUG_STACK
//#define DEBUG_STABLE
//#define DEBUG_THREADS
//#define DEBUG_NONMONOTONIC
//#define DEBUG_LTQ
//#define DEBUG_INDICES
//#define DEBUG_THREADCOUNT
//#define DEBUG_PROPAGATORS
//#define ALLDIFF

//#define MM_DEBUG

// don't create UVAR's
//#define DEBUG_NO_UVAR

// #define DEBUG_PERDIO

//#define DEBUG_PROP_STABILTY_TEST

// test if the liveness routine for X registers works
//#define DEBUG_LIVENESS

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
#define PROFILE_INSTR
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

// mm2: what is this
#define PICKLE2TEXTHACK 1
