include(FetchContent)

# Download GoogleTest at configure time — no manual installation needed
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent GoogleTest from overriding our compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test executable — add new test files here as you create them
add_executable(dnd_tests
    test_wallet.cpp
    test_character.cpp
    # Dice roller tests cover generic die ranges plus d20 mode rules.
    test_dice_roller.cpp
    test_features.cpp
    test_spellslots.cpp
    test_inventory.cpp
    test_items.cpp
    test_spell.cpp
    test_persistence.cpp
    test_colours.cpp
    test_consoleio.cpp
    test_manager_menus.cpp
    test_save_format.cpp
)

target_link_libraries(dnd_tests
    PRIVATE
    dnd_lib
    GTest::gtest_main
)

# Statically link MinGW runtime on Windows so the exe runs without DLLs on the PATH
if(WIN32)
    if(ENABLE_COVERAGE)
        # -static must come first; -lgcov explicitly provides the coverage runtime
        target_link_options(dnd_tests PRIVATE -static -fprofile-arcs -lgcov)
    else()
        target_link_options(dnd_tests PRIVATE -static)
    endif()
endif()

if(ENABLE_COVERAGE)
    target_compile_options(dnd_tests PRIVATE -fprofile-arcs -ftest-coverage)

    if(WIN32)
        set(GCOV_EXECUTABLE "C:/Program Files/CodeBlocks/MinGW/bin/gcov.exe")
    else()
        set(GCOV_EXECUTABLE "gcov")
    endif()

    find_program(GCOVR_EXECUTABLE gcovr)

    if(GCOVR_EXECUTABLE)
        add_custom_target(coverage
            DEPENDS dnd_tests
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
            COMMAND ${GCOVR_EXECUTABLE}
                --gcov-executable "${GCOV_EXECUTABLE}"
                --root ${CMAKE_SOURCE_DIR}
                --exclude "${CMAKE_SOURCE_DIR}/tests/"
                --exclude "${CMAKE_BINARY_DIR}/_deps/"
                --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
                --txt
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating coverage report -> ${CMAKE_BINARY_DIR}/coverage/index.html"
        )
    else()
        message(WARNING "gcovr not found — install it with: pip install gcovr")
    endif()
endif()

# PRE_TEST defers discovery until test-run time — more reliable on MinGW/Windows
# (avoids running the binary during cmake configure, which can fail due to missing DLLs)
include(GoogleTest)
gtest_discover_tests(dnd_tests DISCOVERY_MODE PRE_TEST)
