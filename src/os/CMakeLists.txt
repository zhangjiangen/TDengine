aux_source_directory(src OS_SRC)
add_library(os ${OS_SRC})
target_include_directories(
    os
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    PUBLIC "${CMAKE_SOURCE_DIR}/include/os"
)