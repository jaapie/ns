#include <stdbool.h>

int get_file_name_length(const char *file_spec);
int get_file_name_from_file_spec(const char *file_spec, char *file_name);
int get_path_from_file_spec(char *file_spec, char *path);
char *get_file_ext(const char *filename);
int file_exists(const char *file_name);
char *create_new_file_name(char *path, char *base, char separator, unsigned int width, unsigned int sequence, char *ext, bool *collision_avoided);
