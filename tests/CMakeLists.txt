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
add_sound2osc_test(TestLogger unit/TestLogger.cpp)
add_sound2osc_test(TestConfigStore unit/TestConfigStore.cpp)
add_sound2osc_test(TestAppInfo unit/TestAppInfo.cpp)
add_sound2osc_test(TestDSP unit/TestDSP.cpp)
add_sound2osc_test(TestTrigger unit/TestTrigger.cpp)
add_sound2osc_test(TestBPM unit/TestBPM.cpp)
