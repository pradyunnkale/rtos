cmake_minimum_required(VERSION 3.20)

project(rtos LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(
    RTOS_PORT
    "host"
    CACHE STRING
    "RTOS target port"
)

set_property(
    CACHE RTOS_PORT
    PROPERTY STRINGS
    host
    mimxrt1064
)

if(
    NOT RTOS_PORT STREQUAL "host"
    AND NOT RTOS_PORT STREQUAL "mimxrt1064"
)
    message(FATAL_ERROR "Unsupported RTOS_PORT: ${RTOS_PORT}")
endif()

add_library(rtos STATIC)

target_sources(
    rtos
    PRIVATE
        kernel/dma.c
        kernel/isr.c
        kernel/log.c
        kernel/mutex.c
        kernel/queue.c
        kernel/rtos.c
        kernel/sched.c
        kernel/sem.c
        kernel/task.c
        kernel/time_internal.c

        port/${RTOS_PORT}/port_dma.c
        port/${RTOS_PORT}/port_isr.c
        port/${RTOS_PORT}/port_time.c
)

if(RTOS_PORT STREQUAL "host")
    target_sources(
        rtos
        PRIVATE
            port/host/port.c
    )
endif()

target_include_directories(
    rtos
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/port/${RTOS_PORT}/inc

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/inc
)

if(RTOS_PORT STREQUAL "host")
    target_compile_definitions(
        rtos
        PRIVATE
            _POSIX_C_SOURCE=200809L
    )
endif()

target_compile_options(
    rtos
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

if(RTOS_PORT STREQUAL "host")
    add_executable(
        host_context_test
        test/host_context_test.c
    )

    target_link_libraries(
        host_context_test
        PRIVATE
            rtos
    )

    add_executable(
        host_periodic_test
        test/host_periodic_test.c
    )

    target_link_libraries(
        host_periodic_test
        PRIVATE
            rtos
    )

    add_executable(
        host_preemption_test
        test/host_preemption_test.c
    )

    target_link_libraries(
        host_preemption_test
        PRIVATE
            rtos
    )

    enable_testing()

    add_test(
        NAME host_context_test
        COMMAND host_context_test
    )

    add_test(
        NAME host_periodic_test
        COMMAND host_periodic_test
    )

    add_test(
        NAME host_preemption_test
        COMMAND host_preemption_test
    )
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        compile_commands_link ALL
        COMMAND
            ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Linking compile_commands.json"
        VERBATIM
    )
endif()
