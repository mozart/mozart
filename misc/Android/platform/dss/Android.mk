
MOZART_DSS_SRCBSE = base.cc

MOZART_DSS_SRCMSL = msl_timers.cc mslBase.cc	\
	msl_buffer.cc msl_prioQueues.cc	\
	msl_crypto.cc msl_dct.cc msgnLayer.cc	\
	msl_msgContainer.cc msl_dsite.cc msl_comObj.cc 		\
	msl_transObj.cc msl_tcpTransObj.cc			\
	msl_endRouter.cc msl_interRouter.cc 

MOZART_DSS_SRCDSS = dssBase.cc dss_interface.cc dss_msgLayerInterface.cc		\
	protocols.cc		                                        \
	coordinator.cc referenceConsistency.cc				\
	abstractEntityImplementations.cc dss_threads.cc			\
	protocol_once_only.cc protocol_migratory.cc			\
	protocol_simple_channel.cc protocol_invalid.cc			\
	protocol_pilgrim.cc protocol_transient_remote.cc		\
	protocol_immediate.cc dss_netId.cc protocol_immutable_lazy.cc	\
	protocol_immutable_eager.cc protocol_sited.cc			\
	coordinator_stationary.cc coordinator_fwdchain.cc		\
	dgc.cc dgc_rl2.cc dgc_fwrc.cc	                                \
	dgc_rl1.cc dgc_tl.cc dgc_rl_siteHandler.cc

MOZART_DSS_SRCALL = $(MOZART_DSS_SRCBSE) $(MOZART_DSS_SRCMSL) $(MOZART_DSS_SRCDSS)


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_CPPFLAGS := -g
LOCAL_SRC_FILES := $(MOZART_DSS_SRCALL:%=src/%)
$(warning $(LOCAL_SRC_FILES))
LOCAL_MODULE := libDSS
LOCAL_C_INCLUDES := . $(LOCAL_PATH)/include $(MOZART_GMP_INC)
LOCAL_PRELINK_MODULE := false
LOCAL_STATIC_LIBRARIES := libgmp
include $(BUILD_STATIC_LIBRARY)
