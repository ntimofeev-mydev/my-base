include(my_files)
include(my_compile_options)
include(GoogleTest)

set(TargetName TestBase)

my_files(Sources
  DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
  MASK "*.cpp" "*.h"
)
add_executable(${TargetName} ${Sources})
my_compile_options(
  TARGETS ${TargetName}
)

#target_compile_options(${TargetName} PRIVATE -Wno-character-conversion)



target_precompile_headers(${TargetName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

target_include_directories(${TargetName} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TargetName} PRIVATE
  My::Base
  gtest
  gmock
)

#my_add_compile_options(TARGETS ${TargetName})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_target_properties (${TargetName} PROPERTIES
    FOLDER "${MY_ENGINE_FOLDER}/Tests"
)

# gtest_discover_tests(${TargetName} DISCOVERY_TIMEOUT 10)