#include <dirent.h>
#include <stdio.h>
#include "lib.h"

int check_path_type(const char* path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return path_type_unknown;
    }
    if (S_ISDIR(path_stat.st_mode)) {
      return path_type_directory;
    }
    return path_type_file;
}

char _EOT = (char)0x04; // End of Transmission
void handle_input_file(
  const char* input_path,
  int path_strip_index,
  int buffer_fd
) {
  int _;
  FILE* input_fd = fopen(input_path, "rb");
  _ = write(
    buffer_fd,
    (input_path + path_strip_index),
    strlen(input_path + path_strip_index)
  );
  _ = write(buffer_fd, &_EOT, 1);

  float current_pointer = ftell(input_fd);
  fseek(input_fd, 0, SEEK_END);
  long int file_size = ftell(input_fd);
  fseek(input_fd, current_pointer, SEEK_SET);

  printf("%ld bytes", file_size);

  unsigned char size_int[64]; // it goes up to 64 IF some insane system has sizeof(long int) = 64
  int size_range = sizeof(long int);
  int k = 0;
  for (int i = size_range - 1; i >= 0; i--) {
    size_int[k++] = (char)((file_size >> (i * 8)) & 0b11111111);
  }
  _ = write(buffer_fd, size_int, 64);

  unsigned char in_buf[1024];
  int num_read;

  while ((num_read = fread(in_buf, 1, 1024, input_fd)) > 0) {
    _ = write(buffer_fd, in_buf, num_read);
  }

  fclose(input_fd);
}

void push(char directories[PATH_MAX][PATH_MAX], int cursor, const char* to_add) {
  int i = 0;
  for (; to_add[i] != 0; i++) {
    directories[cursor][i] = to_add[i];
  }
  while (i < PATH_MAX) {
    directories[cursor][i++] = 0;
  }
}

void handle_input_directory(const char* input_path, int buffer_fd) {
  int cursor = 0;
  char directories[PATH_MAX][PATH_MAX] = {0};
  push(directories, cursor, input_path);
  int input_shift = strlen(input_path) + 1;
  while (cursor != -1) {
    char* path = directories[cursor--];
    DIR* dir_ptr = opendir(path);
    if (dir_ptr == NULL) {
      printf("Couldn't open the directory: %s\n", path);
      continue;
    }

    struct dirent* entry_ptr;
    char full_path[2048];
    int i = 0;
    for (; path[i] != 0; i++) {
      full_path[i] = path[i];
    }
    full_path[i++] = '/';
    while ((entry_ptr = readdir(dir_ptr)) != NULL) {
      if (
        strcmp(entry_ptr->d_name, ".") == 0 ||
        strcmp(entry_ptr->d_name, "..") == 0
      ) continue;
      int j = 0;
      for (; entry_ptr->d_name[j] != 0; j++) {
        full_path[i + j] = entry_ptr->d_name[j];
      }
      full_path[i + j] = 0;

      if (entry_ptr->d_type == DT_REG) {
        printf("%s ", full_path);
        handle_input_file(full_path, input_shift, buffer_fd);
        printf("\n");
        continue;
      }
      push(directories, ++cursor, full_path);
    }
    closedir(dir_ptr);
  }
}

void handle_input(const char* input_path, int input_type, int buffer_fd) {
  if (input_type == path_type_file) {
    handle_input_file(input_path, 0, buffer_fd);
  } else {
    handle_input_directory(input_path, buffer_fd);
    return;
  }
}

void mkdir_if_not_exist(char* path) {
  if (mkdir(path, S_IRWXU) < 0) return;
  printf("(Created directory %s)", path);
}

void verify_parent_dirs_exist(char* file_path) {
  for (int i = 0; file_path[i] != 0; i++) {
    if (file_path[i] == '/') file_path[i] = 0;
    if (file_path[i] != 0) continue;
    mkdir_if_not_exist(file_path);
    file_path[i] = '/';
  }
}

long int read_long_int(unsigned char* c) {
  long int output;
  for (int i = 0; i < sizeof(long int); i++) {
    output = (output << 8) | c[i];
  }
  return output;
}

void handle_output(const char* output_path, int buffer_fd) {
  int _;
  char file_path[2048] = {0};
  int i = 0;
  for (; output_path[i] != 0; i++) {
    file_path[i] = output_path[i];
  }
  file_path[i++] = '/';
  long int file_size = 0;

  FILE* f;
  int num_read;
  unsigned char buf[1024];
  int file_name_mode = 0;
  unsigned char read_size_int[64]; // it goes up to 64 IF some insane system has sizeof(long int) = 64
  int read_int_mode = -1;

  // a part of crude fuck fix
  char last_char_of_chunk = 0;
  
  while ((num_read = read(buffer_fd, buf, 1024)) > 0) {
    for (int j = 0; j < num_read; j++) {
      if (read_int_mode >= 0) {
        read_size_int[read_int_mode++] = buf[j];

        if (read_int_mode == 64) {
          file_size = read_long_int(read_size_int);
          printf("(%ld bytes)", file_size);
          read_int_mode = -1;
        }
        continue;
      }
      if (buf[j] == _EOT) {
        file_path[i + file_name_mode] = 0;
        file_name_mode = -1;
        printf("Writing file %s", file_path);
        verify_parent_dirs_exist(file_path);
        f = fopen(file_path, "wb");
        if (f == NULL) {
          printf("Unable to open file: %s", file_path);
          return;
        }
        read_int_mode = 0;
        continue;
      }
      if (file_name_mode >= 0) {
        // this is a very crude fix,
        // but i am fucking tired and want to see it just work, so fuck off
        if (file_name_mode == 0 && j != 0) {
          file_path[i + file_name_mode++] = buf[j-1];
        } else if (file_name_mode == 0 && j == 0 && last_char_of_chunk != 0) {
          file_path[i + file_name_mode++] =last_char_of_chunk;
        }
        file_path[i + file_name_mode++] = buf[j];
        continue;
      }
      if (file_size == 0) {
        printf("...done\n");
        fclose(f);
        f = NULL;
        file_size = 0;
        file_name_mode = 0;
        continue;
      }
      int remaining_buffer = num_read - j;
      int slice_size = file_size > remaining_buffer ? remaining_buffer : file_size;
      fwrite(buf + j, sizeof(char), slice_size, f);

      file_size -= slice_size;
      j += slice_size - 1;
    }
    last_char_of_chunk = buf[1023];
  }
}
