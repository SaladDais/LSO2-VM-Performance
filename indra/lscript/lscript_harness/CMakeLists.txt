# -*- cmake -*-

include(00-Common)
include(LLCommon)
include(LLMath)
include(LScript)

include_directories(
        ${LLCOMMON_INCLUDE_DIRS}
        ${LLMATH_INCLUDE_DIRS}
        ${LSCRIPT_INCLUDE_DIRS}
)
include_directories(SYSTEM
        ${LLCOMMON_SYSTEM_INCLUDE_DIRS}
        )

set(lscript_harness_SOURCE_FILES
        lscriptharness.cpp
        )

set(lscript_execute_HEADER_FILES
        CMakeLists.txt
        )

set_source_files_properties(${lscript_harness_HEADER_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND lscript_harness_SOURCE_FILES ${lscript_harness_HEADER_FILES})


add_executable(lscript_harness
        ${lscript_harness_SOURCE_FILES}
        )
set_target_properties(lscript_harness
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${EXE_STAGING_DIR}"
        )

if (WINDOWS)
    # The following come from LLAddBuildTest.cmake's INTEGRATION_TEST_xxxx target.
    set_target_properties(lscript_harness
            PROPERTIES
            LINK_FLAGS "/debug /NODEFAULTLIB:LIBCMT /SUBSYSTEM:CONSOLE"
            LINK_FLAGS_DEBUG "/NODEFAULTLIB:\"LIBCMT;LIBCMTD;MSVCRT\" /INCREMENTAL:NO"
            LINK_FLAGS_RELEASE ""
            )
endif (WINDOWS)

target_link_libraries(lscript_harness
    ${LSCRIPT_LIBRARIES}
    ${LLCOMMON_LIBRARIES}
    ${LLMATH_LIBRARIES}
    ${PTHREAD_LIBRARY}
    ${DL_LIBRARY}
    )
