cmake_minimum_required(VERSION 3.20)

project(rtos C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RTOS_PORT "host" CACHE STRING "RTOS target port")

add_library(rtos STATIC
	kernel/dma.c
	kernel/isr.c
	kernel/log.c
	kernel/mutex.c
	kernel/queue.c
	kernel/sched.c
	kernel/sem.c
	kernel/task.c
	kernel/time.c
)

target_sources(rtos PRIVATE
	port/${RTOS_PORT}/port_dma.c
	port/${RTOS_PORT}/port_isr.c
	port/${RTOS_PORT}/port_time.c
	port/${RTOS_PORT}/port.c
)

target_include_directories(rtos PUBLIC
	inc
	kernel/inc
	port/${RTOS_PORT}/inc
)

execute_process(
	COMMAND ${CMAKE_COMMAND} -E create_symlink
		${CMAKE_BINARY_DIR}/compile_commands.json
		${CMAKE_SOURCE_DIR}/compile_commands.json
)
