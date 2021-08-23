aux_source_directory(src SRC)

add_library(traft ${SRC})
target_link_libraries(traft INTERFACE itd)