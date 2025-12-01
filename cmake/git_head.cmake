function(get_git_head var)
    execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE REVISION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${var} ${REVISION} PARENT_SCOPE)
endfunction()
