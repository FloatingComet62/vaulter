#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef __NR_memfd_create
  #if defined(__x86_64__)
    #define __NR_memfd_create 319
  #elif defined(__i386__)
    #define __NR_memfd_create 340
  #elif defined(__arm__)
    #define __NR_memfd_create 385
  #endif
#endif
#define NULL_STR (char*)0

typedef struct {
  unsigned char key[256];
  unsigned char iv[16];
} skey;
typedef enum {
  path_type_unknown,
  path_type_directory,
  path_type_file
} path_type;

void encrypt_file(int input_fd, int output_fd, skey s);
void decrypt_file(int input_fd, int output_fd, skey s);

char* shift(int* argc, char*** argv);

int check_path_type(const char* path);
void handle_input(const char* input_path, int input_type, int buffer_fd);
void handle_input_file(
  const char* input_path,
  int path_strip_index,
  int buffer_fd
);

void handle_output(const char* output_path, int buffer_fd);
