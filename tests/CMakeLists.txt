function(add_cstorage_kit_unit_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE CStorageKit::Core)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

set(CSTORAGE_KIT_UNIT_TEST_TARGETS
    cstorage_kit_cvalue_tests
    cstorage_kit_calist_ownership_tests
    cstorage_kit_calist_lifecycle_tests
    cstorage_kit_calist_bulk_tests
    cstorage_kit_calist_queries_tests
    cstorage_kit_calist_transformations_tests
)

add_cstorage_kit_unit_test(cstorage_kit_cvalue_tests test_cvalue.c)
add_cstorage_kit_unit_test(
    cstorage_kit_calist_ownership_tests
    test_calist_ownership.c
)
add_cstorage_kit_unit_test(
    cstorage_kit_calist_lifecycle_tests
    test_calist_lifecycle.c
)
add_cstorage_kit_unit_test(cstorage_kit_calist_bulk_tests test_calist_bulk.c)
add_cstorage_kit_unit_test(
    cstorage_kit_calist_queries_tests
    test_calist_queries.c
)
add_cstorage_kit_unit_test(
    cstorage_kit_calist_transformations_tests
    test_calist_transformations.c
)

add_executable(cstorage_kit_invalid_api test_invalid_api.c)
target_link_libraries(cstorage_kit_invalid_api PRIVATE CStorageKit::Core)

foreach(case IN ITEMS bounds empty-remove insert-bounds type-mismatch)
    add_test(
        NAME cstorage_kit.invalid_${case}
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:cstorage_kit_invalid_api>
            -DARG=${case}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/check_invalid_api.cmake
    )
endforeach()

if(CSTORAGE_KIT_STANDALONE)
    add_custom_target(tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
            ${CSTORAGE_KIT_UNIT_TEST_TARGETS}
            cstorage_kit_invalid_api
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
    )
endif()
