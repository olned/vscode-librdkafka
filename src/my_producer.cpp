/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2019, Magnus Edenhill
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Apache Kafka producer
 * using the Kafka driver from librdkafka
 * (https://github.com/edenhill/librdkafka)
 */

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#if _AIX
#include <unistd.h>
#endif

/*
 * Typical include path in a real application would be
 * #include <librdkafka/rdkafkacpp.h>
 */
#include <librdkafka/rdkafkacpp.h>

static volatile sig_atomic_t run = 1;

static void sigterm(int sig)
{
  run = 0;
}

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
  void dr_cb(RdKafka::Message &message)
  {
    /* If message.err() is non-zero the message delivery failed permanently
     * for the message. */
    if (message.err())
      std::cerr << "% Message delivery failed: " << message.errstr() << std::endl;
    else
      std::cerr << "% Message delivered to topic " << message.topic_name() << " [" << message.partition() << "] at offset " << message.offset() << std::endl;
  }
};

RdKafka::ErrorCode send_message(RdKafka::Producer *producer, const std::string &topic, const std::string &line)
{
  RdKafka::ErrorCode err = RdKafka::ERR__QUEUE_FULL;
  bool try_to_send = true;
  while (try_to_send) {

    
    // Send/Produce message.
    // This is an asynchronous call, on success it will only
    // enqueue the message on the internal producer queue.
    // The actual delivery attempts to the broker are handled
    // by background threads.
    // The previously registered delivery report callback
    // is used to signal back to the application when the message
    // has been delivered (or failed permanently after retries).
    
    err = producer->produce(
        topic,
        // partition
        (int32_t)RdKafka::Topic::PARTITION_UA,
        // msgflags - RK_MSG_COPY Make a copy of the value
        (int)RdKafka::Producer::RK_MSG_COPY,
        // payload
        const_cast<char *>(line.c_str()),
        // payload len
        line.size(),
        // key
        NULL,
        // key_len
        0,
        // Timestamp (defaults to current time)
        0,
        // Message headers, if any
        NULL,
        // Per-message opaque value passed to delivery report
        NULL);

    switch (err) {
    case RdKafka::ERR_NO_ERROR:
      try_to_send = false;
      std::cout << "% Enqueued message (" << line.size() << " bytes) "
                << "for topic " << topic << std::endl;
      break;
    case RdKafka::ERR__QUEUE_FULL:
      producer->poll(1000);
      break;
    default:
      try_to_send = false;
      std::cerr << "% Failed to produce to topic " << topic << ": " << RdKafka::err2str(err) << std::endl;
      break;
    }
  }

  return err;
}

RdKafka::Producer *create_producer(const std::string &brokers,
                                   ExampleDeliveryReportCb *ex_dr_cb)
{
  std::string errstr;

  //Create configuration object
  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  // Set bootstrap broker(s) as a comma-separated list of
  // host or host:port (default port 9092).
  if (conf->set("bootstrap.servers", brokers, errstr) !=
      RdKafka::Conf::CONF_OK) {
    std::cerr << errstr << std::endl;
    exit(1);
  }

  if (conf->set("dr_cb", ex_dr_cb, errstr) != RdKafka::Conf::CONF_OK) {
    std::cerr << errstr << std::endl;
    exit(1);
  }

  // Create producer instance.
  RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
  if (!producer) {
    std::cerr << "Failed to create producer: " << errstr << std::endl;
    exit(1);
  }

  delete conf;

  return producer;
}

int main(int argc, char **argv)
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <brokers> <topic>\n";
    exit(1);
  }

  // delivery report callback.
  ExampleDeliveryReportCb ex_dr_cb;

  RdKafka::Producer *producer = create_producer(argv[1], // brokers
                                                &ex_dr_cb);

  const std::string topic = argv[2];
  std::string errstr;

  signal(SIGINT, sigterm);
  signal(SIGTERM, sigterm);

  // Read messages from stdin and produce to broker.
  std::cout << "% Type message value and hit enter "
            << "to produce message." << std::endl;

  for (std::string line; run && std::getline(std::cin, line);) {
    if (!line.empty()) {
      send_message(producer, topic, line);
    }

    producer->poll(0);
  }

  // Wait for final messages to be delivered or fail in 10 seconds.
  std::cout << "% Flushing final messages..." << std::endl;
  producer->flush(10 * 1000 /* wait for max  */);

  if (producer->outq_len() > 0)
    std::cerr << "% " << producer->outq_len() << " message(s) were not delivered" << std::endl;

  delete producer;

  return 0;
}
