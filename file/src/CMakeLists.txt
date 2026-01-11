set(HEADERFILES
    platform.h
    defines.h
    snfile.h
)

set(SRCS
)

set(SPECIFIC_SRCS
    file.c
)

set(SUBDIRS
    win32
    nix
)

set(INCLUDE_BASE "${PROJECT_SOURCE_DIR}/file/include/snfile")

list(TRANSFORM HEADERFILES PREPEND "${INCLUDE_BASE}/")

target_sources(snfile PRIVATE ${HEADERFILES} ${SRCS})

foreach(dir IN LISTS SUBDIRS)
    foreach(src IN LISTS SPECIFIC_SRCS)
        target_sources(snfile PRIVATE "${dir}/${src}")
    endforeach()
endforeach()

