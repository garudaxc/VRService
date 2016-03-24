LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= fbtest.cpp
LOCAL_MODULE:= fbtest
LOCAL_CFLAGS := -Wall -Wno-unused-parameter

LOCAL_SHARED_LIBRARIES := libc libcutils libhardware libui libgui libutils

include $(BUILD_EXECUTABLE)
