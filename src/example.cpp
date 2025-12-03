#include <basu/sd-bus.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

static sd_bus *bus = NULL;

static void signal_handler(int sig) {
  if (bus) {
    sd_bus_unref(bus);
    bus = NULL;
  }
  exit(0);
}

static int method_handler(sd_bus_message *m, void *userdata,
                          sd_bus_error *ret_error) {
  int32_t input;
  int r;

  r = sd_bus_message_read(m, "i", &input);
  if (r < 0) {
    fprintf(stderr, "%s\n", strerror(-r));
    return r;
  }

  printf("method called: %d\n", input);

  sd_bus_message *reply = NULL;
  r = sd_bus_message_new_method_return(m, &reply);
  if (r < 0) {
    fprintf(stderr, "create reply failed: %s\n", strerror(-r));
    return r;
  }

  bool is_even = (input % 2 == 0);
  r = sd_bus_message_append(reply, "b", is_even);
  if (r < 0) {
    fprintf(stderr, "append reply failed: %s\n", strerror(-r));
    sd_bus_message_unref(reply);
    return r;
  }

  r = sd_bus_send(sd_bus_message_get_bus(m), reply, NULL);
  std::cout << "send reply" << std::endl;
  sd_bus_message_unref(reply);

  if (r < 0) {
    fprintf(stderr, "send failed: %s\n", strerror(-r));
  }

  return r;
}

static const sd_bus_vtable my_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("IsEven", "i", "b", method_handler,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END};

int main(int argc, char *argv[]) {
  int r;

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  r = sd_bus_open_user(&bus);
  if (r < 0) {
    fprintf(stderr, "faild to connect to session bus: %s\n", strerror(-r));
    return EXIT_FAILURE;
  }

  printf("connected\n");

  const char *service_name = "com.example.ServiceName";
  r = sd_bus_request_name(bus, service_name, 0);
  if (r < 0) {
    fprintf(stderr, "request name failed %s\n", strerror(-r));
    sd_bus_unref(bus);
    return EXIT_FAILURE;
  }

  if (r == 1) {
    printf("name requested: %s\n", service_name);
  } else if (r == 0) {
    printf("name %s exist\n", service_name);
  }

  const char *object_path = "/path/to/object";
  const char *interface_name = "com.example.InterfaceName";

  r = sd_bus_add_object_vtable(bus,
                               NULL,
                               object_path,
                               interface_name,
                               my_vtable,
                               NULL);

  if (r < 0) {
    fprintf(stderr, "add vtable failed: %s\n", strerror(-r));
    sd_bus_unref(bus);
    return EXIT_FAILURE;
  }

  printf("interface: %s\n", interface_name);
  printf("object path: %s\n", object_path);
  printf("method: IsEven(i:input_number) → b:is_even_result\n");

  while (1) {
    r = sd_bus_process(bus, NULL);
    if (r < 0) {
      fprintf(stderr, "handle message failed %s\n", strerror(-r));
      break;
    }

    if (r == 0) {
      r = sd_bus_wait(bus, (uint64_t)-1);
      if (r < 0) {
        fprintf(stderr, "wait message failed %s\n", strerror(-r));
        break;
      }
    }
  }

  if (bus) {
    sd_bus_unref(bus);
  }

  return EXIT_SUCCESS;
}