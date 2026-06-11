include(CheckCXXCompilerFlag)

set(MY_COMPILER_MSVC OFF) # msvc cl (only)
set(MY_COMPILER_CLANG_CL OFF) # clang-cl (only)
set(MY_COMPILER_MSVC_COMPAT OFF) # any cl compatible compiler: cl, clang-cl

set(MY_COMPILER_CLANG OFF) # clang
set(MY_COMPILER_GCC OFF) # gcc

set(MY_PLATFORM_WINDOWS OFF)
set(MY_PLATFORM_LINUX OFF)
set(MY_PLATFORM_NAME "UNKNOWN")

# if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
#     set(Host_Arch "x86")
# else()
#     set(Host_Arch "x64")
# endif()

#message(FATAL_ERROR "THE_OS: (${OS})")


#TODO: check ${Platform} instead of cmake values (actual for cross-compilation)
if (WIN32)
  include(platform/microsoft)
  message(STATUS "Configure for Microsoft/(${CMAKE_SYSTEM_NAME}), msvc:(${MY_COMPILER_MSVC}), clang-cl:(${MY_COMPILER_CLANG_CL}), cl compatible:(${MY_COMPILER_MSVC_COMPAT})")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
  include(platform/Linux)
else()
  message(FATAL_ERROR "Unsupported platform (${CMAKE_SYSTEM_NAME})")
endif()
