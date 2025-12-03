//
// Created by yukunlin on 2025/12/2.
//

#include <basu/sd-bus.h>

#include <iostream>

int main(int argc, char **argv) {
  if (argc != 6) {
    std::cout << "expect 4 parameters" << std::endl;
    return 1;
  }

  std::cout << "create session bus" << std::endl;

  std::string service_name = argv[1];
  std::string object_path = argv[2];
  std::string interface_name = argv[3];
  std::string method_name = argv[4];
  int32_t input_number = atoi(argv[5]);
  std::cout << "input_number: " << input_number << std::endl;

  std::cout << "service_name: " << service_name << std::endl;
  std::cout << "object_path: " << object_path << std::endl;
  std::cout << "interface_name: " << interface_name << std::endl;
  std::cout << "method_name: " << method_name << std::endl;
  sd_bus *bus = nullptr;

  int r = 0;
  if (sd_bus_open_user(&bus) < 0) {
    fprintf(stderr, "failed to open session bus: %s\n", strerror(-r));
    return EXIT_FAILURE;
  }

  sd_bus_error error = SD_BUS_ERROR_NULL;
  sd_bus_message *reply = nullptr;
  r = sd_bus_call_method(bus,
                         service_name.c_str(),
                         object_path.c_str(),
                         interface_name.c_str(),
                         method_name.c_str(),
                         &error,
                         &reply,
                         "i", input_number);
  if (r < 0) {
    std::cerr << error.message << std::endl;
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
    return EXIT_FAILURE;
  }
  // read bool from reply
  bool result = false;
  r = sd_bus_message_read(reply, "b", &result);
  if (r < 0) {
    fprintf(stderr, "failed to parse message reply: %s\n", strerror(-r));
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return EXIT_FAILURE;
  }

  printf("return value: %s\n", result ? "true" : "false");
  printf("number %d is %s odd\n",
         input_number,
         result ? "not" : "");
  std::cout << input_number << " " << result << std::endl;

  std::cout << "done" << std::endl;

  return 0;
}