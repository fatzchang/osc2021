include_directories(${CMAKE_SOURCE_DIR}/kernel/include ${CMAKE_SOURCE_DIR}/lib/include)

# local variables
set(TARGET kernel8.elf)

# target
file(GLOB_RECURSE C_FILES src/*.c)
file(GLOB_RECURSE ASM_FILES src/*.S)

## deal with asm files
foreach(X IN LISTS ASM_FILES)
    set_property(SOURCE ${X} PROPERTY LANGUAGE C)
endforeach()

add_executable(${TARGET} ${C_FILES} ${ASM_FILES})

target_link_options(${TARGET} PRIVATE -T ${CMAKE_SOURCE_DIR}/kernel/src/linker.ld)


# link
target_link_libraries(
    ${TARGET}
    mini_uart
    base_ops
    string
    math
    io
    system_call
)
