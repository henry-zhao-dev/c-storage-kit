execute_process(
  COMMAND "${PROGRAM}" "${ARG}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error_output
)

if(result EQUAL 0)
  message(FATAL_ERROR "invalid API case '${ARG}' unexpectedly succeeded")
endif()

set(diagnostic "${output}${error_output}")
if(NOT diagnostic MATCHES "\\[Assertion Failed\\]")
  message(FATAL_ERROR
    "invalid API case '${ARG}' failed without the expected assertion diagnostic:\n${diagnostic}"
  )
endif()

message(STATUS "invalid API case '${ARG}' terminated as documented")
