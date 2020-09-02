cmake_minimum_required(VERSION 3.0.0)
project(basic_syscalls VERSION 0.1.0)

# poll server executable
add_executable(poll_server poll_server.c)
# select server
add_executable(select_server select_server.c)
# client
add_executable(client client.c)
