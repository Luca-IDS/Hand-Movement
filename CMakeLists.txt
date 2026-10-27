cmake_minimum_required(VERSION 3.16...3.28 FATAL_ERROR)

project ("simple_live_qtwidgets_cpp")

message (STATUS "[${PROJECT_NAME}] Processing ${CMAKE_CURRENT_LIST_FILE}")

set (CMAKE_AUTOMOC TRUE)
set (CMAKE_AUTORCC TRUE)

if (MSVC)
    set (SAMPLE_TARGET_DEFINITION_ADDENDUM
            grid_overlay.h)
endif ()

# Setup target executable with the same name as our project
add_executable (${PROJECT_NAME} ${SAMPLE_TARGET_DEFINITION_ADDENDUM}
        main.cpp
        mainwindow.cpp
        display.cpp
        acquisitionworker.cpp
        mainwindow.h
        display.h
        acquisitionworker.h
        hand.cpp
        hand.h
        grid_overlay.h
        frame_overlay.cpp
        frame_overlay.h
)

# Find packages
if (NOT TARGET ids_peak)
    find_package (ids_peak REQUIRED
            HINTS
            ../../../../../../../lib/
    )
endif ()

if (NOT TARGET ids_peak_icv)
    find_package (ids_peak_icv REQUIRED
            HINTS
            ../../../../../../../lib/
    )
endif()

find_package (Threads REQUIRED)

find_package (Qt6 COMPONENTS Widgets Core Gui REQUIRED)

# Set include directories
target_include_directories (${PROJECT_NAME}
        PRIVATE ../../Downloads/simple_live_qtwidgets
)

target_compile_definitions(simple_live_qtwidgets_cpp PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# Link against libraries
target_link_libraries (${PROJECT_NAME}
        ids_peak::ids_peak
        ids_peak_icv::ids_peak_icv
        ${CMAKE_THREAD_LIBS_INIT}
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
)

# Call deploy functions
ids_peak_deploy(${PROJECT_NAME})
ids_peak_icv_deploy(${PROJECT_NAME})

if ((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
    # ignore errors in headers from qt widgets include directories
    foreach (INCLUDE_DIR ${Qt6Widgets_INCLUDE_DIRS})
        if (NOT "${INCLUDE_DIR}" STREQUAL "/usr/include")
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${INCLUDE_DIR}")
        endif ()
    endforeach ()
endif ()

# Qt6 requires at least C++17
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS NO
)

# Enable multiprocessing for MSVC
if (MSVC)
    target_compile_options (${PROJECT_NAME}
            PRIVATE "/MP"
    )
endif ()

if (WIN32)
    if(NOT WINDEPLOYQT_EXECUTABLE)
        get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    endif()

    # Copy Qt files to the output folder
    message (STATUS "[${PROJECT_NAME}] Add PostBuildStep for copy of the Qt libs.")
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --no-translations
            --no-opengl-sw
            --dir $<TARGET_FILE_DIR:${PROJECT_NAME}>
            $<TARGET_FILE:${PROJECT_NAME}>
    )
endif ()

if(UNIX)
    ids_peak_generate_starter_script(${PROJECT_NAME})
endif()