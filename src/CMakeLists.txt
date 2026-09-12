include(my_files)
include(my_compile_options) 

set(TargetName MyBase)
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH ModuleRoot)


my_files(Sources
  DIRECTORIES ${ModuleRoot}/src
  RELATIVE ${ModuleRoot}/src
  MASK "*.cpp" "*.h" "*.hpp"
  EXCLUDE "platform/.*"
)

my_files(PublicHeaders
  DIRECTORIES ${ModuleRoot}/include
  RELATIVE ${ModuleRoot}
  MASK "*.cpp" "*.h" "*.hpp"
  EXCLUDE "platform/.*"
  PREPEND "../"
)

add_library(${TargetName} ${Sources} ${PublicHeaders})
add_library(My::Base ALIAS ${TargetName})
my_compile_options(
  TARGETS ${TargetName}
  STRICT
  RTTI ON
)

target_precompile_headers(${TargetName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

#target_link_libraries(${TargetName} PUBLIC ${ThirdPartyPublicLibs})
target_compile_definitions(${TargetName} PRIVATE MY_BASE_BUILD)

target_include_directories(${TargetName} PUBLIC
  $<BUILD_INTERFACE:${ModuleRoot}/include>
#  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/core/kernel/include>
)

target_include_directories(${TargetName} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TargetName} PUBLIC
  fmt
)

target_link_libraries(${TargetName} PRIVATE
  concurrentqueue
)

set_target_properties (${TargetName} PROPERTIES FOLDER ${MY_ENGINE_FOLDER})

source_group(TREE ${ModuleRoot}/src PREFIX Sources FILES ${Sources})
source_group(TREE ${ModuleRoot}/include PREFIX Headers FILES ${PublicHeaders})