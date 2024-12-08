#include "header.h"
#include <libnotify/notify.h> // library for sending desktop notifications
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // library for memory allocation, process control, conversions and others
#include <string.h>
#include <sys/inotify.h> // library for monitoring file system events
#include <unistd.h>

char *prepare_base_path(char *path) {
  char *basePath = (char *)malloc(sizeof(char) * strlen(path) + 1);
  strcpy(basePath, path);

  // strtok is a function that splits a string into tokens.
  // it store the tokens in a null-terminated string, and it returns a pointer
  char *token =
      strtok(basePath, "/"); // arguments: the string to split and the delimiter

  // Get the last token in the string (the base path)
  while (token != NULL) {
    basePath = token; // The basePath variable is assigned the current token
    token = strtok(NULL, "/"); // The token variable is assigned the next token
  }

  return basePath;
}

void start_event_loop(int IeventQueue, char *basePath) {
  const struct inotify_event
      *watchEvent;   // inotify_event structures that contain information about
                     // the events that occurred
  char buffer[4096]; // 4kb buffer to store charecters read from the inotify
                     // instance
  int readLength;    // The number of bytes read from the inotify instance

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

      watchEvent = (const struct inotify_event *)
          buffPointer; // cast the buffer  pointer to an inotify_event structure
      char *notificationMessage = NULL;

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
        execlp("/bin/sh", "./notify.sh", NULL);
        break;
      case IN_MODIFY:
        notificationMessage = "File modified\n";
        break;
      }

      // Create a new notification message
      if (notificationMessage) {
        NotifyNotification *notifyHandler = notify_notification_new(
            basePath, notificationMessage, "dialog-information");
        if (notifyHandler) {
          notify_notification_set_urgency(notifyHandler,
                                          NOTIFY_URGENCY_CRITICAL);
          notify_notification_show(notifyHandler, NULL);
        }
      }
    }
  }
}
