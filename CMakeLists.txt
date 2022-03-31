cmake_minimum_required(VERSION 3.21)
project(task C)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS -fsanitize=address)

add_executable(server server.c DynamicBuffer.h DynamicBuffer.c PrimitiveResult.h AnarchyList.c AnarchyList.h)
add_executable(client client.c client.c)
