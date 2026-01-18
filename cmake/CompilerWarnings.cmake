# cmake/CompilerWarnings.cmake
# Modern compiler warning flags for sound2osc

function(sound2osc_set_warnings target)
    set(MSVC_WARNINGS
        /W4           # Baseline reasonable warnings
        /w14242       # 'identifier': conversion, possible loss of data
        /w14254       # 'operator': conversion, possible loss of data
        /w14263       # member function does not override any base class virtual member function
        /w14265       # class has virtual functions, but destructor is not virtual
        /w14287       # unsigned/negative constant mismatch
        /we4289       # nonstandard extension used: 'variable'
        /w14296       # expression is always 'boolean_value'
        /w14311       # pointer truncation
        /w14545       # expression before comma evaluates to a function
        /w14546       # function call before comma missing argument list
        /w14547       # operator before comma has no effect
        /w14549       # operator before comma has no effect
        /w14555       # expression has no effect
        /w14619       # pragma warning: warning number does not exist
        /w14640       # thread-unsafe static member initialization
        /w14826       # conversion is sign-extended
        /w14905       # wide string literal cast to 'LPSTR'
        /w14906       # string literal cast to 'LPWSTR'
        /w14928       # illegal copy-initialization
        /permissive-  # standards conformance mode
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    # Apply warnings as compile options (not as errors for now, during migration)
    target_compile_options(${target} PRIVATE ${PROJECT_WARNINGS})
endfunction()
