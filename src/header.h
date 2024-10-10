#ifndef HEADER_H
#define HEADER_H

// Macros for exit status codes
#define EXT_SUCCESS 0
#define EXT_ERR_TOO_FEW_ARGS 1
#define EXT_ERR_INIT_INOTIFY 2
#define EXT_ERR_ADD_WATCH 3
#define EXT_ERR_BASE_PATH_NULL 4
#define EXT_ERR_READ_INOTIFY 5
#define EXT_ERR_INIT_LIBNOTIFY 6

// Declare global variables
extern int IeventQueue;
extern int IeventStatus;

// Declare function prototypes
char *prepare_base_path(char *path);
void start_event_loop(int IeventQueue, char *basePath);

#endif // HEADER_H
