include_guard(GLOBAL)


function (my_compile_options)

    macro (MakeArgZeroOrOne var)
        if (DEFINED ${var})
            if (${${var}})
                set(${var} 1)
            else()
                set(${var} 0)
            endif()
        else()
            set(${var} 0)
        endif()
    endmacro()

    cmake_parse_arguments(OPTS "STRICT" "RTTI;EXCEPTIONS" "TARGETS" ${ARGN})

    if (NOT OPTS_TARGETS)
        message(FATAL_ERROR "TARGETS required")
    endif()

    MakeArgZeroOrOne(OPTS_STRICT)
    MakeArgZeroOrOne(OPTS_RTTI)
    MakeArgZeroOrOne(OPTS_EXCEPTIONS)

    set (forwardArgs 
        STRICT ${OPTS_STRICT}
        RTTI ${OPTS_RTTI}
        EXCEPTIONS ${OPTS_EXCEPTIONS}
    )

    foreach(target IN LISTS OPTS_TARGETS)
        if (NOT TARGET ${target})
            message(FATAL_ERROR "TARGET (${target}) not defined")
        endif()

        if (COMMAND my_target_platform_compile_options)
            my_target_platform_compile_options(${target} ${forwardArgs})
        endif()

        if (COMMAND my_target_custom_compile_options)
            my_target_custom_compile_options(${target} ${forwardArgs})
        endif()

    endforeach()


#[[
  set(optionalValueArgs STRICT)
  set(oneValueArgs RTTI EXCEPTIONS)
  set(multiValueArgs TARGETS)
  cmake_parse_arguments(OPTIONS "${optionalValueArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach (target ${OPTIONS_TARGETS})
    get_target_property(targetType ${target} TYPE)
    if(${targetType} STREQUAL "INTERFACE_LIBRARY")
      continue()
    endif()

    if (OPTIONS_STRICT)
      target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>:${MY_COMPILE_C_STRICT_OPTIONS}>$<$<COMPILE_LANGUAGE:CXX>:${MY_COMPILE_CPP_STRICT_OPTIONS}>)
    else()
      target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>:${MY_COMPILE_C_OPTIONS}>$<$<COMPILE_LANGUAGE:CXX>:${MY_COMPILE_CPP_OPTIONS}>)
    endif()

    if (OPTIONS_RTTI)
      target_compile_options(${target} PRIVATE ${MY_COMPILE_OPTIONS_RTTI_ON})
    else()
      target_compile_options(${target} PRIVATE ${MY_COMPILE_OPTIONS_RTTI_OFF})
    endif()

    if (OPTIONS_EXCEPTIONS)
      target_compile_options(${target} PRIVATE ${MY_COMPILE_OPTIONS_EXCEPTION_ON})
    else()
      target_compile_options(${target} PRIVATE ${MY_COMPILE_OPTIONS_EXCEPTION_OFF})
    endif()

    target_compile_definitions(${target} PRIVATE ${MY_COMPILE_DEFINITIONS} MY_TARGET_NAME="${target}")
  endforeach()
]]
endfunction()