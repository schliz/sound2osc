# CodeCoverage.cmake
# Defines a custom target 'coverage' that runs tests and generates a report

function(add_code_coverage_target)
    if(NOT SOUND2OSC_ENABLE_COVERAGE)
        return()
    endif()

    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(FATAL_ERROR "gcov not found! Aborting coverage setup.")
    endif()

    # Define the coverage report script
    set(COVERAGE_REPORT_SCRIPT "${CMAKE_BINARY_DIR}/generate_coverage_report.cmake")
    
    file(WRITE "${COVERAGE_REPORT_SCRIPT}" "
        # Find all gcda files
        file(GLOB_RECURSE GCDA_FILES \"\${CMAKE_BINARY_DIR}/*.gcda\")
        
        if(NOT GCDA_FILES)
            message(FATAL_ERROR \"No coverage data found. Did tests run?\")
        endif()

        message(\"=== sound2osc Code Coverage Report ===\")
        
        foreach(GCDA_FILE \${GCDA_FILES})
            get_filename_component(GCDA_DIR \${GCDA_FILE} DIRECTORY)
            
            # Run gcov
            execute_process(
                COMMAND \"${GCOV_PATH}\" -n -o \"\${GCDA_DIR}\" \"\${GCDA_FILE}\"
                OUTPUT_VARIABLE GCOV_OUTPUT
                ERROR_QUIET
            )
            
            # Parse gcov output
            string(REPLACE \"\\n\" \";\" LINES \"\${GCOV_OUTPUT}\")
            set(CURRENT_FILE \"\")
            
            foreach(LINE \${LINES})
                if(LINE MATCHES \"^File '(.*)'\")
                    set(CURRENT_FILE \${CMAKE_MATCH_1})
                elseif(LINE MATCHES \"^Lines executed:(.*)% of (.*)\")
                    set(PERCENT \${CMAKE_MATCH_1})
                    
                    # Filter for our source files
                    if(CURRENT_FILE MATCHES \"libs/sound2osc-core/src/\" AND NOT CURRENT_FILE MATCHES \"moc_\")
                        # Format output for alignment
                        string(LENGTH \"\${CURRENT_FILE}\" FILE_LEN)
                        set(PADDING \"\")
                        if(FILE_LEN LESS 80)
                            math(EXPR PAD_LEN \"80 - \${FILE_LEN}\")
                            foreach(I RANGE 1 \${PAD_LEN})
                                string(APPEND PADDING \" \")
                            endforeach()
                        endif()
                        message(\"\${CURRENT_FILE}\${PADDING} \${PERCENT}%\")
                    endif()
                endif()
            endforeach()
        endforeach()
    ")

    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${CMAKE_COMMAND} -P "${COVERAGE_REPORT_SCRIPT}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests and generating coverage report"
    )
endfunction()
