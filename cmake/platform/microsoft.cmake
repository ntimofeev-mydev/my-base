# Setup for Visual C/C++ compiler for Win64
# Compiler_LikeCl:      cl, clang-cl
# Compiler_MSVC:    cl only
# Compiler_ClangCl: clang-cl only
# Compiler_Clang:   currently not supported

set(MY_PLATFORM_WINDOWS ON)
set(MY_PLATFORM_NAME windows)


if(MSVC) # MSVC: cl.exe, clang-cl.exe
    set(MY_COMPILER_MSVC_COMPAT ON)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(MY_COMPILER_MSVC ON)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(MY_COMPILER_CLANG_CL ON)
    else()
        message(FATAL_ERROR "Unsupported compiler:(${CMAKE_CXX_COMPILER_ID}) on (${CMAKE_SYSTEM_NAME})")
    endif()
else()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(MY_COMPILER_CLANG ON)
    endif()
    message(FATAL_ERROR "Unsupported compiler:(${CMAKE_CXX_COMPILER_ID}). Currently supported only msvc like compilers")
endif()

#[[
STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
]]

function (my_target_platform_compile_options target)
    cmake_parse_arguments(ARG "" "STRICT;RTTI;EXCEPTIONS" "" ${ARGN})

    set(Msvc_IgnoreWarnings
        /wd4275 # non dll-interface struct 'XXX' used as base for dll-interface class 'YYY'
        /wd4250 # 'Class_XXX': inherits 'Method_YYY' via dominance
        /wd4251 #  Warning	C4251	'': class '' needs to have dll-interface to be used by clients of struct ''
        /wd4625 #  copy constructor was implicitly defined as deleted
        #/wd4435 # 'Class1': Object layout under /vd2 will change due to virtual base 'Interface'
        /wd4626 #  assignment operator was implicitly defined as deleted
        /wd4820 # 'bytes' bytes padding added after construct 'member_name'
        /wd4868 # warning C4866: compiler may not enforce left-to-right evaluation order for call to operator_name
        /wd5026 # move constructor was implicitly defined as deleted
        /wd5027 # move assignment operator was implicitly defined as deleted
        /wd5039 # pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
        /wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
    )

    set(Clang_IgnoreWarnings
        -Wno-deprecated-builtins
        -Wno-reorder-ctor
        -Wno-invalid-offsetof
        -Wno-deprecated-enum-enum-conversion
        -Wno-deprecated-volatile
    )

    if (MY_COMPILER_MSVC)

        target_compile_options(${target} PRIVATE
            $<IF:${ARG_RTTI},/GR,/GR->
            $<IF:${ARG_EXCEPTIONS},/EHsc,/EHsc->
            $<$<CONFIG:Debug>:/JMC>
            /Zc:preprocessor # preprocessor conformance mode (https://learn.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=msvc-170)
            /MP  #enable multi processor compilation (which is used only for cl)
            ${Msvc_IgnoreWarnings}
        )

    elseif(MY_COMPILER_CLANG_CL)

        target_compile_options(${target} PRIVATE
            $<IF:${ARG_RTTI},/clang:-frtti,/clang:-fno-rtti>
            $<IF:${ARG_EXCEPTIONS},/clang:-fexceptions /clang:-fcxx-exceptions, /clang:-fno-exceptions>
            /clang:-mavx2
            /clang:-mfma
            ${Clang_IgnoreWarnings}
        )

    endif()

    if(NOT BUILD_SHARED_LIBS)
        set(CrtRuntime_DEBUG /MTd)
        set(CrtRuntime_RELEASE /MT)
    else()
        set(CrtRuntime_DEBUG /MDd)
        set(CrtRuntime_RELEASE /MD)
    endif()

    target_compile_options(${target} PRIVATE
        /c
        /nologo
        /Zc:forScope
        /Zc:inline
        /Zc:wchar_t
        /J
        /bigobj
        $<$<CONFIG:Debug>:
            ${CrtRuntime_DEBUG} /GF /Gy /Gw /Oi /Oy /Od
        >
        $<$<CONFIG:Release>:
            ${CrtRuntime_RELEASE}
            /Ox /GF /Gy /Gw /Oi /Ot /Oy
        >
        $<$<CONFIG:RelWithDebInfo>:
            ${CrtRuntime_RELEASE}
            /Od /GS- /GF /Gy /Gw
        >
        $<${ARG_STRICT}:
            -W4 /permissive-
        >
         $<$<COMPILE_LANGUAGE:C>:/TC>
      )

    target_compile_definitions(${target} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DEBUG=1>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1>
    )

endfunction() 
