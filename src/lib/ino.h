#ifndef ino_h
#define ino_h

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/param.h>


static void
handle_events(int fd, int wd, char *string);

int
inotify(char *filepath, int signal);

#ifndef ino_def
#define ino_def

int sig;

void handle_events(int fd, int wd, char *filename) {
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
        kill(getppid(), sig);

      }
    }
  }
}

int inotify(char *filepath, int s) {
  sig = s;
  int fd, poll_num, wd;
  nfds_t nfds;
  struct pollfd fds[1];

  /* Create the file descriptor for accessing the inotify API */
  if ((fd = inotify_init1(IN_NONBLOCK)) == -1) {
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }

  char b[PATH_MAX];
  strcpy(b, filepath);
  /*b =*/ dirname(b); // note reassignment!!!
  // note use IN_ALL_EVENTS if you wanna wait on all
  if ((wd = inotify_add_watch(fd, b, IN_ATTRIB)) == -1) {
    fprintf(stderr, "Cannot watch '%s'\n", filepath);
    perror("inotify_add_watch");
    exit(EXIT_FAILURE);
  }

  nfds = 1;
  fds[0].fd = fd;
  fds[0].events = POLLIN;

  info("Listening for changes to %s", filepath);
  while (1) {
    if ((poll_num = poll(fds, nfds, -1)) == -1) {
      if (errno == EINTR) continue;
      perror("failed to poll");
      exit(EXIT_FAILURE);
    }
    if (poll_num > 0 && fds[0].revents & POLLIN) {
      handle_events(fd, wd, basename(filepath));
    }
  }
}

#endif // ino_def
#endif // ino_h
