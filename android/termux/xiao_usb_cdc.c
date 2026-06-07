/*
 * Minimal USB CDC ACM client for a Seeed XIAO SAMD21 connected to Termux.
 *
 * Build in Termux:
 *   pkg install clang
 *   clang -O2 -Wall -Wextra -o xiao_usb_cdc xiao_usb_cdc.c
 *
 * Run through Termux:API:
 *   termux-usb -e ./xiao_usb_cdc /dev/bus/usb/001/002
 *
 * termux-usb appends the opened USB file descriptor as the final argument.
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CDC_CONTROL_INTERFACE 0
#define CDC_DATA_INTERFACE 1
#define CDC_ENDPOINT_OUT 0x02
#define CDC_ENDPOINT_IN 0x83

static int claim_interface(int fd, unsigned int interface_number) {
  if (ioctl(fd, USBDEVFS_CLAIMINTERFACE, &interface_number) < 0) {
    fprintf(stderr, "claim interface %u failed: %s\n", interface_number,
            strerror(errno));
    return -1;
  }
  return 0;
}

static int control_transfer(int fd, uint8_t request_type, uint8_t request,
                            uint16_t value, uint16_t index, void *data,
                            uint16_t length) {
  struct usbdevfs_ctrltransfer transfer = {
      .bRequestType = request_type,
      .bRequest = request,
      .wValue = value,
      .wIndex = index,
      .wLength = length,
      .timeout = 2000,
      .data = data,
  };

  int result = ioctl(fd, USBDEVFS_CONTROL, &transfer);
  if (result < 0) {
    fprintf(stderr, "control transfer 0x%02x failed: %s\n", request,
            strerror(errno));
  }
  return result;
}

static int configure_cdc(int fd) {
  /* 115200 baud, 1 stop bit, no parity, 8 data bits. */
  uint8_t line_coding[7] = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08};

  if (control_transfer(fd, 0x21, 0x20, 0, CDC_CONTROL_INTERFACE, line_coding,
                       sizeof(line_coding)) < 0) {
    return -1;
  }

  /* Set DTR and RTS. Opening DTR may reset the Arduino-style firmware. */
  if (control_transfer(fd, 0x21, 0x22, 3, CDC_CONTROL_INTERFACE, NULL, 0) < 0) {
    return -1;
  }

  sleep(2);
  return 0;
}

static int bulk_write_all(int fd, const char *text) {
  size_t remaining = strlen(text);
  const char *cursor = text;

  while (remaining > 0) {
    struct usbdevfs_bulktransfer transfer = {
        .ep = CDC_ENDPOINT_OUT,
        .len = (unsigned int)remaining,
        .timeout = 2000,
        .data = (void *)cursor,
    };

    int written = ioctl(fd, USBDEVFS_BULK, &transfer);
    if (written < 0) {
      fprintf(stderr, "USB write failed: %s\n", strerror(errno));
      return -1;
    }

    cursor += written;
    remaining -= (size_t)written;
  }

  return 0;
}

static void print_complete_lines(const char *data, size_t length) {
  static char line[512];
  static size_t used = 0;

  for (size_t i = 0; i < length; i++) {
    char c = data[i];
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      line[used] = '\0';
      printf("< %s\n", line);
      used = 0;
    } else if (used < sizeof(line) - 1) {
      line[used++] = c;
    }
  }
  fflush(stdout);
}

static void read_replies(int fd, int attempts) {
  char buffer[256];

  for (int i = 0; i < attempts; i++) {
    struct usbdevfs_bulktransfer transfer = {
        .ep = CDC_ENDPOINT_IN,
        .len = sizeof(buffer) - 1,
        .timeout = 300,
        .data = buffer,
    };

    int received = ioctl(fd, USBDEVFS_BULK, &transfer);
    if (received > 0) {
      print_complete_lines(buffer, (size_t)received);
    } else if (received < 0 && errno != ETIMEDOUT) {
      fprintf(stderr, "USB read failed: %s\n", strerror(errno));
      return;
    }
  }
}

static int send_command(int fd, const char *command, int read_attempts) {
  printf("> %s", command);
  fflush(stdout);
  if (bulk_write_all(fd, command) < 0) {
    return -1;
  }
  read_replies(fd, read_attempts);
  return 0;
}

static int send_command_file(int fd) {
  const char *home = getenv("HOME");
  if (home == NULL) {
    return 0;
  }

  char path[512];
  if (snprintf(path, sizeof(path), "%s/.xiao_mouse_commands", home) >=
      (int)sizeof(path)) {
    fprintf(stderr, "command file path is too long\n");
    return -1;
  }

  FILE *file = fopen(path, "r");
  if (file == NULL) {
    if (errno != ENOENT) {
      fprintf(stderr, "cannot open %s: %s\n", path, strerror(errno));
      return -1;
    }
    return 0;
  }

  printf("Using commands from %s\n", path);
  char line[128];
  while (fgets(line, sizeof(line), file) != NULL) {
    size_t length = strlen(line);
    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
      line[--length] = '\0';
    }

    if (length == 0 || line[0] == '#') {
      continue;
    }

    if (length + 1 >= sizeof(line)) {
      fprintf(stderr, "command is too long: %s\n", line);
      fclose(file);
      return -1;
    }

    line[length++] = '\n';
    line[length] = '\0';
    if (send_command(fd, line, 2) < 0) {
      fclose(file);
      return -1;
    }
  }

  fclose(file);
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s USB_FD\n", argv[0]);
    return 2;
  }

  int fd = atoi(argv[1]);
  if (fd <= 0 || fcntl(fd, F_GETFD) < 0) {
    fprintf(stderr, "invalid USB file descriptor: %s\n", argv[1]);
    return 2;
  }

  if (claim_interface(fd, CDC_CONTROL_INTERFACE) < 0 ||
      claim_interface(fd, CDC_DATA_INTERFACE) < 0 || configure_cdc(fd) < 0) {
    return 1;
  }

  read_replies(fd, 6);

  int command_file_result = send_command_file(fd);
  if (command_file_result < 0) {
    return 1;
  }
  if (command_file_result > 0) {
    return 0;
  }

  struct {
    const char *text;
    int read_attempts;
  } commands[] = {
      {"S\n", 2},
      {"M 0 0 1\n", 1},   /* Left button down for about 300 ms. */
      {"M 0 0 0\n", 2},   /* Left button up. */
      {"M 30 0 0\n", 2},  /* Visible horizontal movement test. */
      {"S\n", 2},
      {"Z\n", 2},
      {"S\n", 2},
  };

  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    if (send_command(fd, commands[i].text, commands[i].read_attempts) < 0) {
      return 1;
    }
  }

  return 0;
}
