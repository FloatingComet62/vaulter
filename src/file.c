#include <dirent.h>
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
  unsigned char in_buf[1024];
  int num_read;

  while ((num_read = fread(in_buf, 1, sizeof(in_buf), input_fd)) > 0) {
    _ = write(buffer_fd, in_buf, num_read);
  }
  _ = write(buffer_fd, &_EOT, 1);

  fclose(input_fd);
}

void push(char directories[1024][1024], int cursor, const char* to_add) {
  int i = 0;
  for (; to_add[i] != 0; i++) {
    directories[cursor][i] = to_add[i];
  }
  while (i < 1024) {
    directories[cursor][i++] = 0;
  }
}

void handle_input_directory(const char* input_path, int buffer_fd) {
  int cursor = 0;
  char directories[1024][1024] = {0};
  push(directories, cursor, input_path);
  int input_shift = sizeof(input_path) + 1;
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
        printf("%s\n", full_path);
        handle_input_file(full_path, input_shift, buffer_fd);
        continue;
      }
      push(directories, ++cursor, full_path);
    }
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
  printf("Creating directory %s\n", path);
}

void verify_parent_dirs_exist(char* file_path) {
  for (int i = 0; file_path[i] != 0; i++) {
    if (file_path[i] == '/') file_path[i] = 0;
    if (file_path[i] != 0) continue;
    mkdir_if_not_exist(file_path);
    file_path[i] = '/';
  }
}

void handle_output(const char* output_path, int buffer_fd) {
  char file_path[2048] = {0};
  int i = 0;
  for (; output_path[i] != 0; i++) {
    file_path[i] = output_path[i];
  }
  file_path[i++] = '/';

  FILE* f;
  int num_read;
  char buf[1024];
  int file_name_mode = 0;
  while ((num_read = read(buffer_fd, buf, 1024)) > 0) {
    for (int j = 0; j < num_read; j++) {
      if (buf[j] != _EOT) {
        if (file_name_mode >= 0) file_path[i + file_name_mode++] = buf[j];
        else fwrite(buf + j, 1, 1, f);
        continue;
      }
      if (file_name_mode < 0) {
        printf("...done\n");
        fclose(f);
        f = NULL;
        file_name_mode = 0;
        continue;
      }
      file_path[i + file_name_mode] = 0;
      file_name_mode = -1;
      printf("Writing file %s\n", file_path);
      verify_parent_dirs_exist(file_path);
      f = fopen(file_path, "wb");
      if (f == NULL) {
        printf("Unable to open file: %s", file_path);
        return;
      }
    }
  }
}
