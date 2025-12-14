#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"

int encrypt(int argc, char** argv, skey s) {
  char* input_path = shift(&argc, &argv);
  if (input_path == NULL_STR) {
    printf("Missing input\n");
    return 1;
  }
  int file_type;
  if ((file_type = check_path_type(input_path)) == path_type_unknown) {
    printf("Input not a file or directory\n");
    return 1;
  }
  int output_file_type;
  char* output_path = shift(&argc, &argv);
  if (output_path == NULL_STR) {
    printf("Missing output\n");
    return 1;
  }
  if ((output_file_type = check_path_type(output_path)) != path_type_unknown) {
    printf("A file or directory already exists with the same name as output");
    return 1;
  }

  int buffer_fd = syscall(__NR_memfd_create, "buffer", 0x0001U);
  handle_input(input_path, file_type, buffer_fd);
  lseek(buffer_fd, 0, SEEK_SET);

  FILE* f = fopen("temp", "wb");
  int num_read;
  char buf[1024];
  while ((num_read = read(buffer_fd, buf, 1024)) > 0) {
    fwrite(buf, 1, 1024, f);
    // for (int i = 0; i < 100; i++)
      // printf("%c (%d)\n", buf[i], buf[i]);
  }
  // printf("\n");
  fclose(f);
  lseek(buffer_fd, 0, SEEK_SET);

  FILE* encrypted_fd = fopen(output_path, "wb");
  encrypt_file(buffer_fd, fileno(encrypted_fd), s);
  fclose(encrypted_fd);
  return 0;
}

int decrypt(int argc, char** argv, skey s) {
  char* input_path = shift(&argc, &argv);
  if (input_path == NULL_STR) {
    printf("Missing input\n");
    return 1;
  }
  int file_type;
  if ((file_type = check_path_type(input_path)) != path_type_file) {
    printf("Input not a file\n");
    return 1;
  }
  char* output_path = shift(&argc, &argv);
  if (output_path == NULL_STR) {
    printf("Missing output\n");
    return 1;
  }
  if ((file_type = check_path_type(output_path)) != path_type_unknown) {
    printf("Output with the name already exists\n");
    return 1;
  }

  int buffer_fd = syscall(__NR_memfd_create, "buffer", 0x0001U);

  FILE* encrypted_fd = fopen(input_path, "rb");
  decrypt_file(fileno(encrypted_fd), buffer_fd, s);
  lseek(buffer_fd, 0, SEEK_SET);
  fclose(encrypted_fd);

  // int num_read;
  // char buf[1024] = {0};
  // while ((num_read = read(buffer_fd, buf, 1024)) > 0) {
  //   for (int i = 0; i < 100; i++)
  //     printf("%c (%d)\n", buf[i], buf[i]);
  // }
  // printf("\n");
  // lseek(buffer_fd, 0, SEEK_SET);

  handle_output(output_path, buffer_fd);

  return 0;
}

void load_key(const char* key_path, skey* s) {
  int _;
  FILE* key_fd = fopen(key_path, "rb");
  if (key_fd == NULL) {
    printf("Inable to open file %s\n", key_path);
    return;
  }
  _ = fread(s, sizeof(skey), 1, key_fd);
  fclose(key_fd);
}

void save_key(const char* key_path, skey* s) {
  FILE* key_fd = fopen(key_path, "wb");
  if (key_fd == NULL) {
    printf("Inable to open file %s\n", key_path);
    return;
  }
  fwrite(s, sizeof(skey), 1, key_fd);
  fclose(key_fd);
}

void help() {
  printf("Vaulter - 1.0\n");
  printf("Encrypt files or directories with ease\n");
  printf("Usage:\n");
  printf("\tvaulter encrypt <key-path> <target> <output file> - Encrypt target, target can be either file or directory\n");
  printf("\tvaulter decrypt <key-path> <target file> <output directory> - Decrypt target file into output directory, the directory is created if it doesn't exist.\n");
  printf("\tvaulter gen-key <output-key-path> - Create a random key\n");
  printf("\tvaulter help - Display this message\n");
}

int source_main(int argc, char** argv) {
  shift(&argc, &argv);

  char* action = shift(&argc, &argv);
  skey s = {0};

  if (action == NULL_STR) {}
  else if (strcmp(action, "help") == 0) {
    help();
    return 0;
  }
  else if (strcmp(action, "gen-key") == 0) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 256; i++) {
      s.key[i] = (unsigned char)rand();
    }
    for (int i = 0; i < 16; i++) {
      s.iv[i] = (unsigned char)rand();
    }
    char* key_path = shift(&argc, &argv);
    if (key_path == NULL_STR) {
      printf("Key path not provided\n");
      return 1;
    }
    save_key(key_path, &s);
    return 0;
  }
  else if (strcmp(action, "encrypt") == 0) {
    char* key_path = shift(&argc, &argv);
    if (key_path == NULL_STR) {
      printf("Key path not provided\n");
      return 1;
    }
    load_key(key_path, &s);
    return encrypt(argc, argv, s);
  }
  else if (strcmp(action, "decrypt") == 0) {
    char* key_path = shift(&argc, &argv);
    if (key_path == NULL_STR) {
      printf("Key path not provided\n");
      return 1;
    }
    load_key(key_path, &s);
    return decrypt(argc, argv, s);
  }
  printf("Incorrect action\n");
  return 1;
}

int main(int argc, char** argv) {
  int result = source_main(argc, argv);
  if (result != 0) {
    printf("\n");
    help();
  }
  return result;
}

char* shift(int* argc, char*** argv) {
  if (argc == 0) return NULL_STR;
  --(*argc);
  char* output = **argv;
  ++(*argv);
  return output;
}
