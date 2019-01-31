# ---------------------------------------------------------------------------
# CMakeQtCreator.cmake - A function to generate a QtCreator project.
# ---------------------------------------------------------------------------


set(QT_CREATOR_SKELETON_DIR ${CMAKE_CURRENT_LIST_DIR})

function( ADD_QTCREATOR_PROJECT project_name)

    set(DST_DIR      ${CMAKE_BINARY_DIR})
    set(SKELETON_DIR ${QT_CREATOR_SKELETON_DIR})
    set(TOP_SRC_DIR  "${PROJECT_SOURCE_DIR}")

    # Set the configure_file variables, i.e. @VAR@ to be set when copied below.
    get_filename_component(QT_CREATOR_TOP_SRC_DIR ${TOP_SRC_DIR} ABSOLUTE)
    set(QT_CREATOR_BUILD_DIR     ${CMAKE_CURRENT_BINARY_DIR}) # in .creator.user file
    set(QT_CREATOR_TOP_CMAKE_DIR ${PROJECT_SOURCE_DIR})       # in .creator.user file

    FIND_PROCESSOR_COUNT()
    set(QT_CREATOR_MAKE_ARGS "-j${PROCESSOR_COUNT}") # in .creator.user file
    string(REPLACE ";" "\n" QT_CREATOR_INCLUDES "${QT_CREATOR_INCLUDES}") # in .includes file

    # These are the files to copy and adjust
    # qtcreator-template.config
    # qtcreator-template.creator
    # qtcreator-template.creator.user
    # qtcreator-template.files
    # qtcreator-template.includes

    # Copy these files only if they're not there, don't overwrite user modifications.

    if (NOT EXISTS "${DST_DIR}/qtcreator-${project_name}.config")
        configure_file("${SKELETON_DIR}/qtcreator-template.config"
                       "${DST_DIR}/qtcreator-${project_name}.config" @ONLY)
    endif()
    if (NOT EXISTS "${DST_DIR}/qtcreator-${project_name}.creator")
        configure_file("${SKELETON_DIR}/qtcreator-template.creator"
                       "${DST_DIR}/qtcreator-${project_name}.creator" @ONLY)
    endif()
    if (NOT EXISTS "${DST_DIR}/qtcreator-${project_name}.creator.user")
        configure_file("${SKELETON_DIR}/qtcreator-template.creator.user"
                       "${DST_DIR}/qtcreator-${project_name}.creator.user" @ONLY)
    endif()
    #if (NOT EXISTS "${DST_DIR}/qtcreator-${project_name}.includes")
        configure_file("${SKELETON_DIR}/qtcreator-template.includes"
                       "${DST_DIR}/qtcreator-${project_name}.includes" @ONLY)
    #endif()

    # Make the qtcreator-template.files

    file(GLOB_RECURSE QT_CREATOR_FILES
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.h
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.hh
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.hpp
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.hxx
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.c
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.cpp
         ${QT_CREATOR_TOP_SRC_DIR}/gpudb/*.cxx )
    file(GLOB_RECURSE QT_CREATOR_FILES_EXAMPLES
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.h
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.hh
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.hpp
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.hxx
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.c
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.cpp
         ${QT_CREATOR_TOP_SRC_DIR}/example/*.cxx )

    set(QT_CREATOR_FILES ${QT_CREATOR_FILES} ${QT_CREATOR_FILES_EXAMPLES})

    list(SORT QT_CREATOR_FILES)
    string(REPLACE ";" "\n" QT_CREATOR_FILES "${QT_CREATOR_FILES}")
    file(WRITE "${DST_DIR}/qtcreator-${project_name}.files" "${QT_CREATOR_FILES}")

    MESSAGE(STATUS "*")
    MESSAGE(STATUS "* Created  qt-creator config files.")
    MESSAGE(STATUS "*   Usage: qt-creator ${DST_DIR}/qtcreator-${project_name}.creator")

endfunction()

