cmake_minimum_required(VERSION 3.2)

add_executable(producer_cpp producer.cpp)
target_link_libraries(producer_cpp PUBLIC rdkafka++)

add_executable(rdkafka_example_cpp rdkafka_example.cpp )
target_link_libraries(rdkafka_example_cpp PUBLIC rdkafka++)

add_executable(rdkafka_complex_consumer_example_cpp rdkafka_complex_consumer_example.cpp )
target_link_libraries(rdkafka_complex_consumer_example_cpp PUBLIC rdkafka++)

add_executable(kafkatest_verifiable_client kafkatest_verifiable_client.cpp)
target_link_libraries(kafkatest_verifiable_client PUBLIC rdkafka++)

