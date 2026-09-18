cmake_minimum_required(VERSION 3.12)

project(05.containers)

add_executable(sequential_container
    sequential_container.cpp
)

add_executable(linked_list
    linked_list.cpp
)

set_target_properties(sequential_container linked_list PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
