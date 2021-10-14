#ifndef ino_h
#define ino_h

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/param.h>
#include "csapp.h"

/** explain void (*routine)(void *)
 *
 *  declare routine as
 *    Pointer To Function (Pointer To Void) Returning Void
 */
typedef void (*ptfptvrv)(void *);

// if the file changed, call the routine with data args
static void handle_events(int, int, char *, ptfptvrv, void *);

// if the filepath file changed, call the routine with data args
int inotify(char *filepath, ptfptvrv, void *);

#ifndef ino_def
#define ino_def

void handle_events(int fd, int wd, char *filename, ptfptvrv routine, void *data) {
  char buf[4096]
      __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t len;
  char *ptr;
  for (;;) {
    if ((len = read(fd, buf, sizeof buf)) == -1 && errno != EAGAIN) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    if (len <= 0) break;
    for (ptr = buf;
         ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *) ptr;
      if (wd == event->wd &&
          event->len &&
          strcmp(filename, event->name) == 0) {
        routine(data);
      }
    }
  }
}


int inotify(char *filepath, ptfptvrv routine, void *data) {
  int fd, poll_num, wd;
  struct pollfd fds[1];
  nfds_t nfds = 1; // number of file descriptors in fds

  /* Create the file descriptor for accessing the inotify API */
  if ((fd = inotify_init1(IN_NONBLOCK)) == -1) {
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }

  char b[PATH_MAX];
  strcpy(b, filepath);
  /*b =*/ dirname(b); // note reassignment!!!
  // note use IN_ALL_EVENTS if you wanna wait on all,
  //  but that would be unwise because there's no backpressure
  //  in the while loop below. you'll get a lot of events. set
  //  a breakpoint! use the inotify executable built with cmake
  //  to ensure you're watching the correct events.
  if ((wd = inotify_add_watch(fd, b, IN_CLOSE_WRITE)) == -1) {
    fprintf(stderr, "Cannot watch '%s'\n", filepath);
    perror("inotify_add_watch");
    exit(EXIT_FAILURE);
  }

  fds[0].fd = fd;
  fds[0].events = POLLIN;

  info("listening for changes to %s", filepath);
  while (1) {
    info("polling sleeps this thread waiting for changes to %lu file descriptor: %i ", nfds, fds[0].fd);
    if ((poll_num = poll(fds, nfds, -1)) == -1) {
      if (errno == EINTR) continue;
      perror("failed to poll");
      exit(EXIT_FAILURE);
    }
    if (poll_num > 0 && fds[0].revents & POLLIN) {
      info("we got an event %hx", fds[0].revents);
      handle_events(fd, wd, basename(filepath), routine, data);
    }
  }
}

#endif // ino_def
#endif // ino_h


