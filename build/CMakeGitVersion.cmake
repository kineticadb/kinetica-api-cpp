# ---------------------------------------------------------------------------
# CMakeGitVersion.cmake - Function to get the version of a git repo.
# ---------------------------------------------------------------------------

# Tries to get the git hash, last commit date, and if the repo is modified.
# Sets the variables :
#   ${GIT_VARIABLE_PREFIX}GIT_REPO_HASH_STRING = "b47ffe4c9ca84b988852a96ef238b895506baf16"
#   ${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_STRING = "Tue Sep 3 12:20:57 2013 -0400"
#   ${GIT_VARIABLE_PREFIX}GIT_REPO_MODIFIED    = TRUE or FALSE if any files are modified
# Usage : GetGitRepoVersionInfo( /path/to/git/repo "PROJECT1_")
#   To create variables "PROECT1_GIT_REPO_HASH_STRING" etc...
#
function( GetGitRepoVersionInfo GIT_REPO_DIRECTORY GIT_VARIABLE_PREFIX)
    if (NOT GIT_FOUND)
        find_package(Git)
        if (NOT GIT_FOUND)
            message(WARNING "git executable was not found, is it installed or in the PATH?")
            return()
        endif()
    endif()

    # Get the latest commit log to get the hash and the date.
    #   "b47ffe4c9ca84b988852a96ef238b895506baf16 2013-09-03 12:20:57 -0400"
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:"%H %ci"
                    WORKING_DIRECTORY ${GIT_REPO_DIRECTORY}
                    OUTPUT_VARIABLE GIT_LOG_STRING)

    string(STRIP "${GIT_LOG_STRING}" GIT_LOG_STRING)
    string(REGEX MATCH "([0-9a-fA-F]+) ([- a-zA-Z0-9:]+)" _dummy ${GIT_LOG_STRING})
    set(GIT_REPO_HASH_STRING ${CMAKE_MATCH_1})
    set(GIT_REPO_DATE_STRING ${CMAKE_MATCH_2})

    # Get the list of modified files to determine if the sources are modified.
    execute_process(COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD --
                    WORKING_DIRECTORY ${GIT_REPO_DIRECTORY}
                    OUTPUT_VARIABLE GIT_STATUS_STRING)
    string(STRIP "${GIT_STATUS_STRING}" GIT_MODIFIED_STRING)
    if ("${GIT_MODIFIED_STRING}" STREQUAL "")
        set(GIT_REPO_MODIFIED FALSE)
    else()
        set(GIT_REPO_MODIFIED TRUE)
    endif()

    # Create a condensed number from the date and time, discard the timezone. "20130903122057"
    string(REGEX REPLACE "[-:]" "" GIT_REPO_DATE_NUMBER ${GIT_REPO_DATE_STRING} )
    string(REGEX MATCH "([0-9]+) ([0-9]+)" _dummy ${GIT_REPO_DATE_NUMBER})
    set(GIT_REPO_DATE_NUMBER "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")

    set(${GIT_VARIABLE_PREFIX}GIT_LOG_STRING       ${GIT_LOG_STRING}       PARENT_SCOPE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_HASH_STRING ${GIT_REPO_HASH_STRING} PARENT_SCOPE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_STRING ${GIT_REPO_DATE_STRING} PARENT_SCOPE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_NUMBER ${GIT_REPO_DATE_NUMBER} PARENT_SCOPE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_MODIFIED    ${GIT_REPO_MODIFIED}    PARENT_SCOPE)

    set(${GIT_VARIABLE_PREFIX}GIT_LOG_STRING       ${GIT_LOG_STRING}       CACHE INTERNAL "" FORCE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_HASH_STRING ${GIT_REPO_HASH_STRING} CACHE INTERNAL "" FORCE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_STRING ${GIT_REPO_DATE_STRING} CACHE INTERNAL "" FORCE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_NUMBER ${GIT_REPO_DATE_NUMBER} CACHE INTERNAL "" FORCE)
    set(${GIT_VARIABLE_PREFIX}GIT_REPO_MODIFIED    ${GIT_REPO_MODIFIED}    CACHE INTERNAL "" FORCE)

    message(STATUS "* ${GIT_VARIABLE_PREFIX}GIT_LOG_STRING       = '${GIT_LOG_STRING}'")
    message(STATUS "* ${GIT_VARIABLE_PREFIX}GIT_REPO_HASH_STRING = '${GIT_REPO_HASH_STRING}'")
    message(STATUS "* ${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_STRING = '${GIT_REPO_DATE_STRING}'")
    message(STATUS "* ${GIT_VARIABLE_PREFIX}GIT_REPO_DATE_NUMBER = '${GIT_REPO_DATE_NUMBER}'")
    message(STATUS "* ${GIT_VARIABLE_PREFIX}GIT_REPO_MODIFIED    = ${GIT_REPO_MODIFIED}")

endfunction()
