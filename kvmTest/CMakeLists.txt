cmake_minimum_required(VERSION 3.5.1)
project (perform)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )

#---------[ what type of agent?
#SET(AGENT "native" CACHE STRING "AGENT type. Possible options: native(default), python, kvm")
#SET(PYAGENT TRUE)

#---------[ what type of tool?


#---------[ external Libs libmonitor
find_library(LIBMONITOR_LIBRARY
        NAMES monitor
        PATHS "${PROJECT_SOURCE_DIR}/dependencies/libmonitor/installDir"
        PATH_SUFFIXES lib lib64
        NO_DEFAULT_PATH
	REQUIRED
)

message("Libmonitor Library directory found: ${LIBMONITOR_LIBRARY}")

find_path(LibMonitor_INCLUDE_DIR
    NAMES monitor.h
    PATHS "${PROJECT_SOURCE_DIR}/dependencies/libmonitor/installDir"
    PATH_SUFFIXES include
    NO_DEFAULT_PATH
    REQUIRED
)

message("Libmonitor include directory found: ${LibMonitor_INCLUDE_DIR}")

#---------[ external Libs xed

set(XED_INSTALL_DIR "${PROJECT_SOURCE_DIR}/dependencies/xed/kits/xed-install-base")
message("Going to seach for XED installation in: ${XED_INSTALL_DIR}")
find_library(XED_LIBRARY
        NAMES xed
        PATHS "${XED_INSTALL_DIR}"
        PATH_SUFFIXES lib lib64
        NO_DEFAULT_PATH
	REQUIRED
)

message("Xed Library directory found: ${XED_LIBRARY}")

find_path(XED_INCLUDE_DIR
    NAMES xed-init.h
    PATHS "${XED_INSTALL_DIR}/include/xed"
#    PATH_SUFFIXES include
    NO_DEFAULT_PATH
    REQUIRED
)

message("Xed include directory found: ${XED_INCLUDE_DIR}")

#----------[ bfd
if(APPLE)
    include_directories("/opt/local/include")
    find_library(BFD_LIBRARY bfd /opt/local/lib)
else(APPLE)
    find_library(BFD_LIBRARY bfd /usr/lib)
endif(APPLE)


#---------[ pThreads
find_package (Threads REQUIRED)

#---------[ librt

find_library(LIBRT_LIBRARIES rt)

#---------[ lunwind
find_path(LIBUNWIND_INCLUDE_DIR libunwind.h
		HINTS 
		/usr/
)

find_library(LIBUNWIND_GENERIC_LIBRARY unwind)

#--------[ ldl
find_library(LIBDL_LIBRARY dl) 

#----------[ python libs
if (${PYAGENT})
	find_package(PythonLibs REQUIRED)
	include_directories(${PYTHON_INCLUDE_DIRS})
endif()

#----------[ Search for included files here
include_directories(${PROJECT_SOURCE_DIR}/includes/platform)
include_directories(${PROJECT_SOURCE_DIR}/includes/platform/device)
include_directories(${PROJECT_SOURCE_DIR}/includes/platform/misc)
include_directories(${PROJECT_SOURCE_DIR}/includes/tools)
include_directories(${PROJECT_SOURCE_DIR}/includes/agent)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${LibMonitor_INCLUDE_DIR})
include_directories(${XED_INCLUDE_DIR})
include_directories(${LIBUNWIND_INCLUDE_DIR})



set(SOURCES
	${PROJECT_SOURCE_DIR}/src/main.cpp
	${PROJECT_SOURCE_DIR}/src/platform/platform.cpp
	${PROJECT_SOURCE_DIR}/src/platform/device/device.cpp
	${PROJECT_SOURCE_DIR}/src/platform/device/ebsDevice.cpp
	${PROJECT_SOURCE_DIR}/src/platform/device/pebsDevice.cpp
	${PROJECT_SOURCE_DIR}/src/platform/device/bpDevice.cpp

	${PROJECT_SOURCE_DIR}/src/agent/agent.cpp
	${PROJECT_SOURCE_DIR}/src/platform/device/perf_parser.cpp
	${PROJECT_SOURCE_DIR}/src/platform/misc/logger.cpp

#	${PROJECT_SOURCE_DIR}/src/agent/perf_parser.cpp
#	${PROJECT_SOURCE_DIR}/src/agent/decoder.cpp
#	${PROJECT_SOURCE_DIR}/src/tools/tool.cpp
)

#---------------[ setting up an agent
if (${PYAGENT})
	set(AGENTSOURCE
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/myagent.cpp
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/session.cpp
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/libperform.cpp
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/pyagent.cpp
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/pyagent_on_signal.cpp
	        ${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/pyagent_stack_unwind.cpp
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/pyBytecodeIterator.cpp
		${PROJECT_SOURCE_DIR}/src/agent/pythonAgent/symbol.cpp
	)
	include_directories(${PROJECT_SOURCE_DIR}/includes/agent/pythonAgent)
elseif(${KVMAGENT})
	set(AGENTSOURCE
                ${PROJECT_SOURCE_DIR}/src/agent/kvmAgent/myagent.cpp
		${PROJECT_SOURCE_DIR}/src/agent/kvmAgent/kvminterface.cpp
		${PROJECT_SOURCE_DIR}/src/agent/kvmAgent/symbol.cpp
		${PROJECT_SOURCE_DIR}/src/agent/kvmAgent/decoder.cpp
        )
	include_directories(${PROJECT_SOURCE_DIR}/includes/agent/kvmAgent)
else()
	set(AGENTSOURCE
                ${PROJECT_SOURCE_DIR}/src/agent/nativeAgent/myagent.cpp
		${PROJECT_SOURCE_DIR}/src/agent/nativeAgent/session.cpp
		${PROJECT_SOURCE_DIR}/src/agent/nativeAgent/symbol.cpp
		${PROJECT_SOURCE_DIR}/src/agent/nativeAgent/decoder.cpp
        )
	include_directories(${PROJECT_SOURCE_DIR}/includes/agent/nativeAgent)
endif()

#--------------[ setting up a Tool
if (${SAMPLEEBS})
        set(TOOLSOURCE
                ${PROJECT_SOURCE_DIR}/src/tools/sampleEBSTool/mytool.cpp
        )
        include_directories(${PROJECT_SOURCE_DIR}/includes/tools/sampleEBSTool)
elseif (${SAMPLEPEBS})
	set(TOOLSOURCE
		${PROJECT_SOURCE_DIR}/src/tools/samplePEBSTool/mytool.cpp
	)
	include_directories(${PROJECT_SOURCE_DIR}/includes/tools/samplePEBSTool)
elseif (${SAMPLEPEBSBP})
        set(TOOLSOURCE
                ${PROJECT_SOURCE_DIR}/src/tools/samplePEBSBPTool/mytool.cpp
        )
        include_directories(${PROJECT_SOURCE_DIR}/includes/tools/samplePEBSBPTool)
elseif(${WITCHTOOL})
	set(TOOLSOURCE
                ${PROJECT_SOURCE_DIR}/src/tools/witchTool/mytool.cpp
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/watchpoint-lib/src/watchpoint.cpp
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/watchpoint-lib/src/watchpoint_mmap.cpp

		${PROJECT_SOURCE_DIR}/src/tools/witchTool/witchLogic/WitchLogic.cpp

		#DeadStore
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/witchLogic/deadStore.cpp
		#RedSpy
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/witchLogic/redSpy.cpp
		#LoadSpy
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/witchLogic/loadSpy.cpp
		#DupSpy
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/witchLogic/dupSpy.cpp
		#falsesharing
		${PROJECT_SOURCE_DIR}/src/tools/witchTool/witchLogic/falsesharing.cpp
        )
        include_directories(${PROJECT_SOURCE_DIR}/includes/tools/witchTool)
	include_directories(${PROJECT_SOURCE_DIR}/includes/tools/witchTool/watchpoint-lib)

	#WitchLogic
	include_directories(${PROJECT_SOURCE_DIR}/includes/tools/witchTool/witchLogic)
else()
	set(TOOLSOURCE
                ${PROJECT_SOURCE_DIR}/src/tools/sampleEBSTool/mytool.cpp
        )
	include_directories(${PROJECT_SOURCE_DIR}/includes/tools/sampleEBSTool)
endif()

#---------[ Tool Specific configuration
#---------------[ Client Tools

#-----------------[ Load redundancy
#SET(LOAD_REDUNDANCY TRUE)
#SET(DEAD_STORE TRUE)
#SET(RED_STORE TRUE)

if (${LOAD_REDUNDANCY})
	add_definitions(-DAVOID_CALLQ)
	add_definitions(-DAVOID_LOCK)
	add_definitions(-DAVOID_SEEN)
	#add_definitions(-DAVOID_PUSH)

	add_definitions(-DCONSIDER_KERNEL_HEAP)
	#add_definitions(-DCONSIDER_KERNEL_STACK)

	add_definitions(-DSHOW_SAMPLED_WATCHED_SAME_ADDRESS)
	#add_definitions(-DSHOW_USED_CONTEXT)

	add_definitions(-DWITCHTOOL_LOADSPY)
	add_definitions(-DWITCHTOOL_LOADSPY_INST)

	#add_definitions(-DWITCHTOOL_REDSPY)
	#add_definitions(-DWITCHTOOL_REDSPY_CONSIDER_APPROXRED)
	#add_definitions(-DWITCHTOOL_REDSPY_INST)

	#add_definitions(-DWITCHTOOL_DEADSPY)
	#add_definitions(-DWITCHTOOL_DEADSTORE_INST)

#-------------------------------------------
#add_definitions(-DWITCHTOOL_DUPSPY) #-
#add_definitions(-DWITCHTOOL_DUPSPY_LOAD) #-
#add_definitions(-DWITCHTOOL_DUPSPY_STORE)
#add_definitions(-DWITCHTOOL_DUPSPY_OVERLAP) #-
#--------------------------------------------

	#add_definitions(-DWITCHTOOL_FALSESHARE)

	add_definitions(-DPEBS_SAMPLING_PERIOD=10003)

	#add_definitions(-DPEBS_SAMPLING_LLC_LOAD_MISS)
	#add_definitions(-DPEBS_SAMPLING_L1_LOAD_MISS)
elseif(${RED_STORE})
	add_definitions(-DAVOID_CALLQ)
        add_definitions(-DAVOID_LOCK)
        add_definitions(-DAVOID_SEEN)
        #add_definitions(-DAVOID_PUSH)

        add_definitions(-DCONSIDER_KERNEL_HEAP)
        #add_definitions(-DCONSIDER_KERNEL_STACK)

        add_definitions(-DSHOW_SAMPLED_WATCHED_SAME_ADDRESS)
        #add_definitions(-DSHOW_USED_CONTEXT)

        #add_definitions(-DWITCHTOOL_LOADSPY)
        #add_definitions(-DWITCHTOOL_LOADSPY_INST)

        add_definitions(-DWITCHTOOL_REDSPY)
        #add_definitions(-DWITCHTOOL_REDSPY_CONSIDER_APPROXRED)
        add_definitions(-DWITCHTOOL_REDSPY_INST)

        #add_definitions(-DWITCHTOOL_DEADSPY)
        #add_definitions(-DWITCHTOOL_DEADSTORE_INST)

#-------------------------------------------
#add_definitions(-DWITCHTOOL_DUPSPY) #-
#add_definitions(-DWITCHTOOL_DUPSPY_LOAD) #-
#add_definitions(-DWITCHTOOL_DUPSPY_STORE)
#add_definitions(-DWITCHTOOL_DUPSPY_OVERLAP) #-
#--------------------------------------------

        #add_definitions(-DWITCHTOOL_FALSESHARE)

        add_definitions(-DPEBS_SAMPLING_PERIOD=10003)

        #add_definitions(-DPEBS_SAMPLING_LLC_LOAD_MISS)
        #add_definitions(-DPEBS_SAMPLING_L1_LOAD_MISS)
elseif(${DEAD_STORE})
        add_definitions(-DAVOID_CALLQ)
        add_definitions(-DAVOID_LOCK)
        add_definitions(-DAVOID_SEEN)
        #add_definitions(-DAVOID_PUSH)

        add_definitions(-DCONSIDER_KERNEL_HEAP)
        #add_definitions(-DCONSIDER_KERNEL_STACK)

        add_definitions(-DSHOW_SAMPLED_WATCHED_SAME_ADDRESS)
        #add_definitions(-DSHOW_USED_CONTEXT)

#        add_definitions(-DWITCHTOOL_LOADSPY)
#        add_definitions(-DWITCHTOOL_LOADSPY_INST)

        #add_definitions(-DWITCHTOOL_REDSPY)
        #add_definitions(-DWITCHTOOL_REDSPY_CONSIDER_APPROXRED)
        #add_definitions(-DWITCHTOOL_REDSPY_INST)

        add_definitions(-DWITCHTOOL_DEADSPY)
        add_definitions(-DWITCHTOOL_DEADSTORE_INST)

#-------------------------------------------
#add_definitions(-DWITCHTOOL_DUPSPY) #-
#add_definitions(-DWITCHTOOL_DUPSPY_LOAD) #-
#add_definitions(-DWITCHTOOL_DUPSPY_STORE)
#add_definitions(-DWITCHTOOL_DUPSPY_OVERLAP) #-
#--------------------------------------------

        #add_definitions(-DWITCHTOOL_FALSESHARE)

        add_definitions(-DPEBS_SAMPLING_PERIOD=10003)

        #add_definitions(-DPEBS_SAMPLING_LLC_LOAD_MISS)
        #add_definitions(-DPEBS_SAMPLING_L1_LOAD_MISS)
else()
        add_definitions(-DPEBS_SAMPLING_PERIOD=100003)
	add_definitions(-DPEBS_SAMPLING_L1_LOAD_MISS)
endif()

#---------[ executable and shared libs
#add_executable(perform ${SOURCES})
add_library(perform SHARED ${SOURCES} ${AGENTSOURCE} ${TOOLSOURCE})

TARGET_LINK_LIBRARIES(perform ${XED_LIBRARY} ${LIBMONITOR_LIBRARY} ${LIBRT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${BFD_LIBRARY} ${LIBUNWIND_GENERIC_LIBRARY} ${LIBDL_LIBRARY})

if (${PYAGENT})
	TARGET_LINK_LIBRARIES(perform ${PYTHON_LIBRARIES})
endif()

#TARGET_LINK_LIBRARIES(perform ${LIBMONITOR_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

