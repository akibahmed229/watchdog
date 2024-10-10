#include <signal.h> // library for signal handling (e.g., catching signals, sending signals, stopping processes)
#include <stdbool.h> // library for boolean type
#include <stdint.h>  // library for fixed-width integer types
#include <stdio.h>   // library for standard input and output
#include <stdlib.h> // library for memory allocation, process control, conversions and others
#include <string.h>

#include <libnotify/notify.h> // library for sending desktop notifications
#include <sys/inotify.h>      // library for monitoring file system events

/* access to the POSIX (Portable Operating System Interface) operating system
 API. It contains declarations for various functions that allow for system-level
 programming, especially for interacting with the operating system. */
#include <unistd.h>

// Macros for exit status codes
#define EXT_SUCCESS 0
#define EXT_ERR_TOO_FEW_ARGS 1
#define EXT_ERR_INIT_INOTIFY 2
#define EXT_ERR_ADD_WATCH 3
#define EXT_ERR_BASE_PATH_NULL 4
#define EXT_ERR_READ_INOTIFY 5
#define EXT_ERR_INIT_LIBNOTIFY 6

// Global variables to store the file descriptor for the inotify instance and
// the watch descriptor for the file or directory being monitored by the inotify
// instance respectively.
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

/* The argc parameter is the number of command line arguments.
 * The argv  parameter is an array of strings that represent the individual
 * arguments provided on the command line. */
int main(int argc, char **argv) {
  char *basePath = NULL;
  char *token = NULL;
  char *notificationMessage = NULL;

  bool libnotifyInitStatus = false;
  NotifyNotification *notifyHandler; // A pointer to a NotifyNotification
                                     // structure that represents a notification
                                     // message

  char buffer[4096]; // 4kb buffer to store charecters read from the inotify
                     // instance
  int readLength;    // The number of bytes read from the inotify instance

  const struct inotify_event
      *watchEvent; // inotify_event structures that contain information about
                   // the events that occurred

  /* The watchMask variable is assigned a value that is a bitwise OR of the IN_
   * constants. These constants are used to specify the events that the
   * inotify instance will monitor. */
  const uint32_t watchMask = IN_CREATE | IN_DELETE | IN_ACCESS |
                             IN_CLOSE_WRITE | IN_MODIFY | IN_MOVE_SELF;

  if (argc < 2) {
    // fprintf is a function that sends formatted output to a stream.
    // stderr is the standard error stream
    // argv[0] is the name of the program itself
    fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
    exit(EXT_ERR_TOO_FEW_ARGS);
  }

  // The basePath variable is assigned a block of memory that is the size of
  // the string in argv[1] plus one byte for the null terminator.
  basePath = (char *)malloc(sizeof(char) * strlen(argv[1]) + 1);
  strcpy(basePath, argv[1]);

  // strtok is a function that splits a string into tokens.
  // it store the tokens in a null-terminated string, and it returns a pointer
  token =
      strtok(basePath, "/"); // arguments: the string to split and the delimiter

  // Get the last token in the string (the base path)
  while (token != NULL) {
    basePath = token; // The basePath variable is assigned the current token
    token = strtok(NULL, "/"); // The token variable is assigned the next token
  }

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

  // Loop while events can be read from inotify file descriptor.
  while (true) {
    printf("Waiting for inotify events...\n");

    /* Read some events
     * read() system call interact with the file descriptor IeventQueue and
     * store the data in the buffer */
    readLength = read(IeventQueue, buffer, sizeof(buffer));
    if (readLength == -1) {
      fprintf(stderr, "Error reading from inotify instance!!!\n");
      exit(EXT_ERR_READ_INOTIFY);
    }

    /* Loop over all events in the buffer. The buffer contains one or more
     * inotify_event structures. The length of the buffer is the number of bytes
     * read from the inotify instance. */
    for (char *buffPointer = buffer; buffPointer < buffer + readLength;
         buffPointer += sizeof(struct inotify_event) + watchEvent->len) {

      notificationMessage = NULL; // Set the notification message to NULL
      watchEvent = (const struct inotify_event *)
          buffPointer; // cast the buffer  pointer to an inotify_event structure

      // Based on the event type, set the notification message
      switch (watchEvent->mask) {
      case IN_CREATE:
        notificationMessage = "File created\n";
        break;
      case IN_DELETE:
        notificationMessage = "File deleted\n";
        break;
      case IN_ACCESS:
        notificationMessage = "File accessed\n";
        break;
      case IN_CLOSE_WRITE:
        notificationMessage = "File closed for writing\n";
        break;
      case IN_MODIFY:
        notificationMessage = "File modified\n";
        break;
      case IN_MOVE_SELF:
        notificationMessage = "File moved\n";
        break;
      }

      // If the notification message is NULL, continue to the next event
      if (notificationMessage == NULL) {
        continue;
      }

      // Create a new notification message
      notifyHandler = notify_notification_new(basePath, notificationMessage,
                                              "dialog-information");
      if (notifyHandler == NULL) {
        fprintf(stderr, "Error creating notification message!!!\n");
        continue;
      }
      notify_notification_set_urgency(notifyHandler, NOTIFY_URGENCY_CRITICAL);
      notify_notification_show(notifyHandler, NULL);
    }
  }
}
