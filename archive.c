#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const char* FILE_TYPE = "FILE";
const char* DIR_TYPE = "DIR";
const char* DIR_END = "DIR_END";

void copyFile(char* fileName, FILE* out) {
  FILE* in = fopen(fileName, "r");
  if (in == NULL) {
    fprintf(stderr, "Read error: %s\n", fileName);
    return;
  }
  struct stat fileStat;
  stat(fileName, &fileStat);
  char date[100];
  strcpy(date, ctime(&fileStat.st_mtime));
  fprintf(out, "%s\n%s\n%li,%li\n", fileName, FILE_TYPE, fileStat.st_size,
          fileStat.st_mtime);
  char buffer[128];
  while (!feof(in)) {
    if (fgets(buffer, sizeof(buffer), in)) {
      fprintf(out, "%s", buffer);
    }
  }
  fclose(in);
}

void copyDir(char* dirName, FILE* out) {
  fprintf(out, "%s\n%s\n", dirName, DIR_TYPE);
}

void zipDir(char* dir, FILE* out) {
  DIR* dp;

  struct dirent* entry;
  struct stat statbuf;
  if ((dp = opendir(dir)) == NULL) {
    fprintf(stderr, "Cannot open directory: %s\n", dir);
    exit(1);
  }

  chdir(dir);
  while ((entry = readdir(dp)) != NULL) {
    lstat(entry->d_name, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
      if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
        continue;
      copyDir(entry->d_name, out);
      zipDir(entry->d_name, out);
      fprintf(out, "%s\n", DIR_END);
    } else
      copyFile(entry->d_name, out);
  }
  chdir("..");
  closedir(dp);
}

void Zip(char* in_path, char* out_path) {
  FILE* out = fopen(out_path, "w");
  if (out == NULL) {
    fprintf(stderr, "Create error: %s\n", out_path);
    exit(1);
  }
  zipDir(in_path, out);
  fclose(out);
}

void createFile(char* fileName, FILE* in) {
  size_t fileSize;
  size_t createTime;
  FILE* out = fopen(fileName, "w");
  if (out == NULL) {
    fprintf(stderr, "Error, cann`t create file: %s\n", fileName);
    exit(1);
  }
  fscanf(in, "%li,%li\n", &fileSize, &createTime);
  if (fileSize > 0) {
    size_t count = 0;
    while (count++ < fileSize) {
      fprintf(out, "%c", fgetc(in));
    }
  }

  fclose(out);
}

void unzipDir(char* path, FILE* in) {
  char title[100];
  char type[8];
  chdir(path);
  while (fscanf(in, "%s", title) != EOF) {
    if (strcmp(title, DIR_END) == 0) {
      chdir("..");
      continue;
    }
    fscanf(in, "%s", type);
    if (strcmp(type, DIR_TYPE) == 0) {
      mkdir(title, S_IRUSR | S_IWUSR | S_IXUSR);
      chdir(title);
    } else if (strcmp(type, FILE_TYPE) == 0) {
      createFile(title, in);
    } else {
      fprintf(stderr, "Type error: %s\n", type);
    }
  }
}

void UnZip(char* archPath, char* path) {
  if (access(archPath, F_OK) != 0) {
    fprintf(stderr, "Error, archive file doesn`t exist: %s\n", archPath);
    exit(1);
  }
  FILE* in = fopen(archPath, "r");
  if (in == NULL) {
    fprintf(stderr, "Error, cann`t open archive file: %s\n", archPath);
    exit(1);
  }

  unzipDir(path, in);

  fclose(in);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Error, unexpected number of elements: %i\n", argc - 1);
    exit(1);
  }
  char* command = argv[1];
  char* in_path = argv[2];
  char* out_path = argv[3];
  if (strcmp(command, "zip") == 0)
    Zip(in_path, out_path);
  else if (strcmp(command, "unzip") == 0) {
    UnZip(in_path, out_path);
  } else {
    fprintf(stderr, "Error, unknown command: %s\n", command);
  }
  exit(0);
}