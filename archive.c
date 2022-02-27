#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void CopyFile(char* inputPath, FILE* out) {
  FILE* in = fopen(inputPath, "r");
  if (in == NULL) {
    fprintf(stderr, "Read error: %s\n", inputPath);
    return;
  }
  struct stat fileStat;
  stat(inputPath, &fileStat);
  char date[100];
  strcpy(date, ctime(&fileStat.st_mtime));
  fprintf(out, "file_name = %s, date = %s\n", inputPath, date);
  char buffer[128];
  while (!feof(in)) {
    if (fgets(buffer, sizeof(buffer), in)) {
      fprintf(out, "%s", buffer);
    }
  }
  if (fclose(in) != 0) printf("Close error: %s", inputPath);
}

void PrintDir(char* dir, int depth, FILE* out) {
  DIR* dp;
  struct stat statbuff;
  struct dirent* entry;
  if ((dp = opendir(dir)) == NULL) {
    // fprintf(stderr, "cannot open direcotry: %s\n", dir);
    return;
  }

  chdir(dir);
  while ((entry = readdir(dp)) != NULL) {
    lstat(entry->d_name, &statbuff);
    if (S_ISDIR(statbuff.st_mode)) {
      if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
        continue;
      fprintf(out, "%*s%s/\n", depth, "", entry->d_name);
      PrintDir(entry->d_name, depth + 4, out);
    } else if (S_ISREG(statbuff.st_mode)) {
      CopyFile(entry->d_name, out);
    }
  }
}

void Zip(char* path) {
  char* outputFile = "out.zip";
  FILE* out = fopen(outputFile, "w");
  if (out == NULL) {
    fprintf(stderr, "Create error: %s\n", outputFile);
    return;
  }
  PrintDir(path, 0, out);
  fclose(out);
}

int main() {
  // CopyFile("file.in");
  Zip("/home/ilya/Desktop/git/laba1");
  exit(0);
}