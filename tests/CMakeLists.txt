find_package(Qt6 REQUIRED COMPONENTS Test)

# Helper function to add unit tests
function(add_sound2osc_test name source)
    add_executable(${name} ${source})
    
    target_link_libraries(${name} PRIVATE
        sound2osc::core
        Qt6::Test
    )
    
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Add Unit Tests
add_sound2osc_test(TestOSCMessage unit/TestOSCMessage.cpp)
