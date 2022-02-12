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

set(lscript_execute_SOURCE_FILES
    llscriptresource.cpp
    llscriptresourceconsumer.cpp
    llscriptresourcepool.cpp
    lscript_execute.cpp
    lscript_heapruntime.cpp
    lscript_readlso.cpp
    )

set(lscript_execute_HEADER_FILES
    CMakeLists.txt

    ../llscriptresource.h
    ../llscriptresourceconsumer.h
    ../llscriptresourcepool.h
    ../lscript_execute.h
    ../lscript_rt_interface.h
    lscript_heapruntime.h
    lscript_readlso.h
    )

set_source_files_properties(${lscript_execute_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND lscript_execute_SOURCE_FILES ${lscript_execute_HEADER_FILES})

add_library (lscript_execute ${lscript_execute_SOURCE_FILES})

# Have a flag to run lscript tests without all the other tests
# There's a bunch of broken tests I don't feel like fixing, and I don't think anybody
# actually runs them anyway.
if (LL_TESTS OR LSCRIPT_TESTS)
    message(WARNING "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_command( TARGET lscript_execute PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_BINARY_DIR}/tests"
            COMMENT "Copying lscript_execute test files"
            )
    set(test_libs
            ${LSCRIPT_LIBRARIES}
            ${WINDOWS_LIBRARIES}
            ${LLMATH_LIBRARIES}
            ${LLCOMMON_LIBRARIES}
            )
    LL_ADD_INTEGRATION_TEST(lscript_execute_state "" "${test_libs}")
endif (LL_TESTS OR LSCRIPT_TESTS)