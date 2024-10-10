#include "header.h"
#include "lib.c"

#include <signal.h> // library for signal handling (e.g., catching signals, sending signals, stopping processes)
#include <stdint.h>  // library for fixed-width integer types
#include <unistd.h> // lib for POSIX operating system API


int IeventQueue = -1;
int IeventStatus = -1;

char *ProgramName = "Watchdog"; // The name of the program

/* Catch shutdown signals so we can
 * cleanup exit properly */
void signal_handler(int signal) {
  int closeStatus = -1;

  printf("Signal received closing inotify instance...\n");

  // don't need to check if they're valid because
  // the signal handlers are registered after they're created;
  // we can assume they are set up correctly.
  closeStatus = inotify_rm_watch(IeventQueue, IeventStatus);
  if (closeStatus == -1) {
    fprintf(stderr, "Error closing inotify instance!!!\n");
  }
  close(IeventQueue);

  notify_uninit();
  exit(EXT_SUCCESS);
}

int main(int argc, char **argv) {
  char *basePath = NULL;
  bool libnotifyInitStatus = false;

  const uint32_t watchMask = IN_CREATE | IN_DELETE | IN_ACCESS |
                             IN_CLOSE_WRITE | IN_MODIFY | IN_MOVE_SELF;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
    exit(EXT_ERR_TOO_FEW_ARGS);
  }

  // The basePath variable is assigned a block of memory that is the size of
  // the string in argv[1] plus one byte for the null terminator.
  basePath = prepare_base_path(argv[1]);

  // If basePath is NULL exit the program
  if (basePath == NULL) {
    fprintf(stderr, "Error getting base path!!!\n");
    exit(EXT_ERR_BASE_PATH_NULL);
  }

  // Initialize the libnotify library
  libnotifyInitStatus = notify_init(ProgramName);
  if (!libnotifyInitStatus) {
    fprintf(stderr, "Error initializing libnotify!!!\n");
    exit(EXT_ERR_INIT_LIBNOTIFY);
  }

  // Initialize the inotify instance
  IeventQueue = inotify_init(); // inotify_init() returns a file descriptor for
                                // the inotify instance
  if (IeventQueue == -1) {
    fprintf(stderr, "Error initialize inotify instance!!!\n");
    exit(EXT_ERR_INIT_INOTIFY);
  }

  // Add a watch to the inotify instance for the specified file or directory and
  // events to monitor
  IeventStatus = inotify_add_watch(
      IeventQueue, argv[1],
      watchMask); // inotify_add_watch() returns a watch descriptor
  if (IeventStatus == -1) {
    fprintf(stderr, "Error adding file to watch instance!!!\n");
    exit(EXT_ERR_ADD_WATCH);
  }

  // Register the signal handler function to handle the SIGINT signal
  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
    
   start_event_loop(IeventQueue, basePath); 
}
