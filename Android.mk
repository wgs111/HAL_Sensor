LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	nusensors.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils libhardware

LOCAL_MODULE:= test-nusensors

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

#
## mythings
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        mytestsensor.cpp

LOCAL_SHARED_LIBRARIES := \
        libcutils libhardware

LOCAL_MODULE:= mytestsensor

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        mytestgps.cpp

LOCAL_SHARED_LIBRARIES := \
        libcutils libhardware libhardware_legacy

LOCAL_MODULE:= mytestgps

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        dataserver.cpp

LOCAL_SHARED_LIBRARIES := \
        libcutils libhardware

LOCAL_MODULE:= mydataserver

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        mytestacc.cpp

LOCAL_SHARED_LIBRARIES := \
        libcutils libhardware

LOCAL_MODULE:= mytestacc

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        mytestgyr.cpp

LOCAL_SHARED_LIBRARIES := \
        libcutils libhardware

LOCAL_MODULE:= mytestgyr

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        mytestbar.cpp

LOCAL_SHARED_LIBRARIES := \
        libcutils libhardware

LOCAL_MODULE:= mytestbar

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
