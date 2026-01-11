if(SN_FILE_BUILD_SHARED)
    add_library(snfile SHARED)
    target_compile_definitions(snfile PRIVATE SN_EXPORT)
else()
    add_library(snfile STATIC)
    target_compile_definitions(snfile PRIVATE SN_FILE_STATIC)
endif()

target_include_directories(snfile PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(snfile PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(snfile PRIVATE sn_file_configs)

add_subdirectory(src)
