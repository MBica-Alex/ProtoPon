cmake_minimum_required(VERSION 3.26)

# Required for compatibility with FreeType's older CMakeLists.txt
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# NOTE: update executable name in .github/workflows/cmake.yml:25 when changing executable name in this file
# for now, the project name is used as the executable name
set(MAIN_PROJECT_NAME "oop")
set(MAIN_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}")


project(${MAIN_PROJECT_NAME})
# set(CMAKE_PROJECT_VERSION_MAJOR 0)
# set(CMAKE_PROJECT_VERSION_MINOR 0)
# set(CMAKE_PROJECT_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(cmake/CopyHelper.cmake)
include(cmake/Options.cmake)

###############################################################################
# Set global sanitizer flags BEFORE FetchContent so SFML inherits them

if(USE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

if(USE_MSAN)
    if(NOT MSVC AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        # MSan requires consistent stdlib - force libstdc++ for SFML compatibility
        add_compile_options(-fsanitize=memory,undefined -fsanitize-recover=memory,undefined -fsanitize-memory-track-origins)
        add_link_options(-fsanitize=memory,undefined -fsanitize-recover=memory,undefined -fsanitize-memory-track-origins)
    endif()
endif()

###############################################################################

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0
)

FetchContent_MakeAvailable(sfml)

include(cmake/CompilerFlags.cmake)

###############################################################################

# external dependencies with find_package

# find_package(Threads REQUIRED)

###############################################################################

# SFML graphical version
add_executable(protopon
    main.cpp
    src/Unit.cpp
    src/GameStats.cpp
    src/Patapon.cpp
    src/Enemy.cpp
    src/Boss.cpp
    src/CommandSequence.cpp
    src/Army.cpp
    src/Game.cpp
    src/GameException.cpp
    src/GameConfig.cpp
)

target_include_directories(protopon PRIVATE include)

set_compiler_flags(RUN_SANITIZERS TRUE TARGET_NAMES protopon)

target_link_libraries(protopon
        SFML::Graphics
        SFML::Window
        SFML::System
)

# Copy SFML DLLs to executable directory on Windows
if(WIN32)
    add_custom_command(TARGET protopon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SFML::Graphics>
            $<TARGET_FILE:SFML::Window>
            $<TARGET_FILE:SFML::System>
            $<TARGET_FILE_DIR:protopon>
    )
endif()

###############################################################################

# copy binaries to "bin" folder; these are uploaded as artifacts on each release
# DESTINATION_DIR is set as "bin" in cmake/Options.cmake:6
install(TARGETS protopon DESTINATION ${DESTINATION_DIR})
if(APPLE)
    install(FILES launcher.command DESTINATION ${DESTINATION_DIR})
endif()

copy_files(DIRECTORY assets COPY_TO_DESTINATION TARGET_NAME protopon)
