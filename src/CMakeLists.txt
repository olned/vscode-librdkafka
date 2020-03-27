cmake_minimum_required(VERSION 3.2)

add_executable(my_producer my_producer.cpp)
target_link_libraries(my_producer PUBLIC rdkafka++)
