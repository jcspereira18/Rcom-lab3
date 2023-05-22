#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define F 0x5C
#define A_set 0x01
#define A_ua  0x03
#define C_set 0x03
#define C_ua 0x07

#define ESC 0x5D

#define I(s) (0 | (s << 1))
#define RR(s) (1 | (s << 5))
#define REJ(s) (0b101 | (s << 5))
#define DISC 0b1011

#define buf_size 2000

int send_SET(int fd);

int read_SET(int fd);

int send_UA(int fd);

int read_UA(int fd);

int send_DISC(int fd);

int read_DISC(int fd);

int read_frame(char* packet, int fd);

int send_frame(const char* buf, int bufSize, int fd);

int write_frame(const char* buf, int bufSize, int fd, char control);

int expect_frame(char* packet, int fd, char control);