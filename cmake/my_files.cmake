include_guard(GLOBAL)


### my_files is a copy-paste of nau_collect_files
### https://github.com/NauEngine/NauEnginePublic/blob/f3d7c27b22476566bbc8d9a22c4d76b7a0753036/cmake/NauCommon.cmake

###     my_files(<variable>
###         [DIRECTORIES <directories>]
###         [RELATIVE <relative-path>]
###         [MASK <globbing-expressions>]
###         [EXCLUDE <regex-to-exclude>]
###    )
###
###  Generate a list of files from <directories> (traverse all the subdirectories) that match the <globbing-expressions> and store it into the <variable>
###  If RELATIVE flag is specified, the results will be returned as relative paths to the given path.
###  If EXCLUDE is specified, all paths that matches any <regex-to-exclude> willbe removed from result
function (my_files VARIABLE)
  set(oneValueArgs RELATIVE PREPEND)
  set(multiValueArgs DIRECTORIES EXCLUDE INCLUDE MASK)
  cmake_parse_arguments(COLLECT "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (COLLECT_EXCLUDE AND COLLECT_INCLUDE)
    message(FATAL_ERROR "my_files must not specify both EXCLUDE and INCLUDE parameters")
  endif()

  set (allFiles)

  foreach (dir ${COLLECT_DIRECTORIES})
    foreach (msk ${COLLECT_MASK})
      if (NOT EXISTS "${dir}")
        message(FATAL_ERROR "Source lookup directory not exists (${dir})")
      endif()
      set(globExpr "${dir}/${msk}")

      if (COLLECT_RELATIVE)
        file (GLOB_RECURSE files RELATIVE ${COLLECT_RELATIVE} ${globExpr} )
      else()
        file (GLOB_RECURSE files ${globExpr})
      endif()

      list(APPEND allFiles ${files})
    endforeach()
  endforeach()

  if (COLLECT_EXCLUDE)
    foreach (re ${COLLECT_EXCLUDE})
      list (FILTER allFiles EXCLUDE REGEX ${re})
    endforeach()
  endif(COLLECT_EXCLUDE) # COLLECT_EXCLUDE

  if (COLLECT_INCLUDE)
    # for include mode
    # must independently filter allFiles by each regex
    # and union all filtration results (excluding duplicates) 
    set (filteredList)
    foreach (re ${COLLECT_INCLUDE})
        set (tempList ${allFiles})
        list (FILTER tempList INCLUDE REGEX ${re})
        list(APPEND filteredList ${tempList})
    endforeach()

    list(REMOVE_DUPLICATES filteredList)
    set (allFiles ${filteredList})

  endif(COLLECT_INCLUDE) # COLLECT_INCLUDE

  if (COLLECT_PREPEND)
    list(TRANSFORM allFiles PREPEND "${COLLECT_PREPEND}")
  endif()


  if (${VARIABLE})
    list(APPEND ${VARIABLE} ${allFiles})
    set(${VARIABLE} ${${VARIABLE}} PARENT_SCOPE)
  else()
    set(${VARIABLE} ${allFiles} PARENT_SCOPE)
  endif()

endfunction()
