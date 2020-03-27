# Learn Kafka on Ubuntu

```bash
set -e; mkdir build && cd build && cmake .. && make -j
```

```bash
./build/examples/rdkafka_example_cpp -P -t test -b ${KAFKA_SERVER_CONNECT} 
```

```bash
./build/examples/rdkafka_example_cpp -C -t test -b ${KAFKA_SERVER_CONNECT} -p 0
```
