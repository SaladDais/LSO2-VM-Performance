# -*- cmake -*-

set(lscript_HEADER_FILES
    llscriptresource.h
    llscriptresourceconsumer.h
    llscriptresourcepool.h
    lscript_alloc.h
    lscript_byteconvert.h
    lscript_byteformat.h
    lscript_execute.h
    lscript_export.h
    lscript_http.h
    lscript_library.h
    lscript_rt_interface.h
    )

add_subdirectory(lscript_compile)
add_subdirectory(lscript_execute)

add_subdirectory(lscript_library)
add_subdirectory(lscript_harness)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/viewer_version.txt"
           "1.0\n")
