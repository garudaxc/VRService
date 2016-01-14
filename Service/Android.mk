LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_MODULE:= libvr
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	VRSurfaceManager.cpp


LOCAL_SHARED_LIBRARIES := \
	libutils \
	libEGL \
	libui \
	libgui
	

include $(BUILD_SHARED_LIBRARY)

ifeq (,$(ONE_SHOT_MAKEFILE))
include $(call first-makefiles-under,$(LOCAL_PATH))
endif
