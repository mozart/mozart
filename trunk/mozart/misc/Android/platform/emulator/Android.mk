MOZART_PLATFORM := android-arm
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator
LOCAL_SRC_FILES := stack.cc ozostream.cc hashtbl.cc iso-ctype.cc \
	refsarray.cc tagged.cc value.cc atoms.cc \
	dictionary.cc extension.cc bitarray.cc word.cc \
	ozthread.cc heapchunk.cc bytedata.cc bits.cc \
	susplist.cc \
	var_base.cc var_ext.cc var_simple.cc \
	var_readonly.cc var_failed.cc var_opt.cc \
	namer.cc \
	weakdict.cc siteprop.cc \
	os.cc unix.cc urlc.cc ioHandler.cc \
	am.cc unify.cc indexing.cc codearea.cc \
	trail.cc emulate.cc scheduler.cc \
	copycode.cc opcodes.cc controlvar.cc \
	prop_int.cc prop_class.cc \
	thr_stack.cc thr_class.cc thr_pool.cc \
	susp_queue.cc thr_int.cc suspendable.cc \
	g-collect.cc cmem.cc mem.cc liveness.cc \
	board.cc distributor.cc \
	cpi.cc cpi_fd.cc cpi_prop.cc cpi_filter.cc \
	cpi_fs.cc cpi_expect.cc cpi_misc.cc \
	cpi_stream.cc cpi_profile.cc cpi_ct.cc \
	var_ct.cc \
	fdomn.cc \
	var_fd.cc var_bool.cc \
	fset.cc var_fs.cc \
	var_of.cc \
	gname.cc site.cc \
	gentraverser.cc marshalerBase.cc pickleBase.cc \
	marshalerPatch.cc marshalerDict.cc \
        pickle.cc components.cc componentBuffer.cc byteBuffer.cc \
	main.cc \
	version.cc \
	statisti.cc \
	print.cc base.cc ozconfig.cc \
	foreign.cc dpInterface.cc boot-manager.cc \
	builtins.cc vprops.cc debug.cc interFault.cc \
	system.cc alice.cc base64.cc
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -g
LOCAL_LDFLAGS := -Wl,--export-dynamic -Wl,--no-gc-sections
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SHARED_LIBRARIES := libz libdl
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Glue
LOCAL_MODULE_STEM := Glue
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic -g
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/ $(LOCAL_PATH)/../dss/include
LOCAL_STATIC_LIBRARIES := libDSS libgmp
####LOCAL_SHARED_LIBRARIES := libDSS
LOCAL_SRC_FILES := glue_interface.cc engine_interface.cc \
		glue_tables.cc glue_faults.cc \
		glue_builtins.cc \
		pstContainer.cc dpMarshaler.cc \
		glue_buffer.cc glue_site.cc \
		glue_mediators.cc glue_marshal.cc \
		glue_suspendedThreads.cc \
		glue_entityOpImpl.cc \
		glue_ioFactory.cc
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:%=libdp/%)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Schedule
LOCAL_MODULE_STEM := Schedule
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := libfd/scheduling.cc libfd/schedulingDist.cc \
	    libfd/schedulingDistAux.cc libfd/taskintervals.cc  \
            libfd/schedtable.cc libfd/schedinit.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_FDP
LOCAL_MODULE_STEM := FDP
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := libfd/fdaux.cc libfd/pel_fncts.cc libfd/pel_engine.cc \
	    libfd/fdinit.cc libfd/fdtable.cc \
	    libfd/arith.cc libfd/bool.cc \
	    libfd/card.cc libfd/count.cc \
	    libfd/rel.cc libfd/disjoint.cc libfd/std.cc \
	    libfd/sum.cc libfd/sumequal.cc \
	    libfd/distance.cc libfd/streamProps.cc \
	    libfd/sumd.cc libfd/sumabs.cc libfd/complalldist.cc libfd/diffn.cc \
	    libfd/distribute.cc libfd/taskoverlap.cc \
	    libfd/boundsalldist.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_FSP
LOCAL_MODULE_STEM := FSP
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := libfset/fsaux.cc libfset/fsinit.cc libfset/fsstd.cc \
libfset/intsets.cc libfset/monitor.cc libfset/reified.cc libfset/standard.cc \
libfset/std_n.cc libfset/table.cc libfset/telling.cc libfset/testing.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Parser
LOCAL_MODULE_STEM := Parser
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := parser.cc scanner.cc parsertable.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_CompilerSupport
LOCAL_MODULE_STEM := CompilerSupport
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := compiler.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Browser
LOCAL_MODULE_STEM := Browser
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := browser.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Debug
LOCAL_MODULE_STEM := Debug
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := debugBI.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Space
LOCAL_MODULE_STEM := Space
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := s-clone.cc board-bi.cc sit-check.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_FDB
LOCAL_MODULE_STEM := FDB
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := fdcore.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_FSB
LOCAL_MODULE_STEM := FSB
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := fsetcore.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_CTB
LOCAL_MODULE_STEM := CTB
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := ct-bi.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_RecordC
LOCAL_MODULE_STEM := RecordC
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := ofs-bi.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Compat
LOCAL_MODULE_STEM := Compat
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := compat.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Win32
LOCAL_MODULE_STEM := Win32
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := win32.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_Profile
LOCAL_MODULE_STEM := Profile
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := profile.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_OsTime
LOCAL_MODULE_STEM := OsTime
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := ostime.cc
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := ozEmulator_ZlibIO
LOCAL_MODULE_STEM := ZlibIO
LOCAL_MODULE_SUFFIX := .so-$(MOZART_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_CPPFLAGS := -fno-strict-aliasing -fno-implicit-templates -fpic
LOCAL_LDFLAGS := -Wl,--warn-unresolved-symbols
LOCAL_C_INCLUDES := $(MOZART_GMP_INC) $(TOPDIR)external/zlib/
LOCAL_STATIC_LIBRARIES := libgmp
LOCAL_SRC_FILES := zlibio.cc
include $(BUILD_SHARED_LIBRARY)

