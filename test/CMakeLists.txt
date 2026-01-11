set(SRCS
    test.c
)

add_executable(sn_file_test)

target_sources(sn_file_test PRIVATE ${SRCS})

target_link_libraries(sn_file_test PRIVATE snfile)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_target(copy_dlls ALL
        COMMENT "Copy the dlls"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_RUNTIME_DLLS:sn_file_test>" "$<TARGET_FILE_DIR:sn_file_test>"
        COMMAND_EXPAND_LISTS
    )

    add_dependencies(copy_dlls sn_file_test)
endif()

