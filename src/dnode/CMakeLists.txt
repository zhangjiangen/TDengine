aux_source_directory(src DNODE_SRC)
add_executable(taosd ${DNODE_SRC})
target_link_libraries(
    taosd
    PRIVATE os
)