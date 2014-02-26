
typedef struct file_item {
    char *file_name;
    char *file_name_new;
    bool collision_avoided;
    time_t date_time;
    struct file_item *next;
} file_item_t;

int file_item_generate_new_filename(file_item_t *item, char *base, char separator, unsigned int width, unsigned int sequence_start, unsigned int sequence_number); /*, bool interactive, int verbosity ); */
int file_item_rename_file( file_item_t *item);
void file_item_print(const file_item_t * restrict item, bool dry_run, int verbosity);
