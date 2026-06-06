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
      buffer[received] = '\0';
      printf("< %s", buffer);
      if (buffer[received - 1] != '\n') {
        putchar('\n');
      }
      fflush(stdout);
    } else if (received < 0 && errno != ETIMEDOUT) {
      fprintf(stderr, "USB read failed: %s\n", strerror(errno));
      return;
    }
  }
}

static int send_command(int fd, const char *command) {
  printf("> %s", command);
  fflush(stdout);
  if (bulk_write_all(fd, command) < 0) {
    return -1;
  }
  read_replies(fd, 4);
  return 0;
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

  const char *commands[] = {
      "S\n",
      "M 5 0 0\n",
      "S\n",
      "Z\n",
      "S\n",
  };

  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    if (send_command(fd, commands[i]) < 0) {
      return 1;
    }
  }

  return 0;
}
