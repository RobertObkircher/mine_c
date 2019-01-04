#include "filereload.h"
#include "allocorexit.h"

#include <log.h>

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    const char *directory_name;
    const char *filename;
    char *path;
    int watch_descriptor;

    void (*callback)(const char *path);
} WatchedFile;

static int fd = -1;
static int watched_files_count;
static WatchedFile watched_files[FILERELOAD_MAX_WATCHED_FILES];

static void reset() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    while (watched_files_count > 0) {
        free(watched_files[--watched_files_count].path);
    }
}

void init_filereload() {
    log_debug("init");
    reset(); // to free paths

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }
}

void close_filereload() {
    log_debug("close");

    reset();
}

void listen_for_file_changes(const char *directory_name, const char *filename,
                             void (*callback)(char *path)) {
    if (watched_files_count < FILERELOAD_MAX_WATCHED_FILES) {
        WatchedFile *wf = &watched_files[watched_files_count++];
        wf->directory_name = directory_name;
        wf->filename = filename;
        wf->watch_descriptor = inotify_add_watch(fd, directory_name, IN_CREATE | IN_MODIFY | IN_MOVED_TO);
        wf->callback = callback;
        {
            /* concatenate directory_name and filename */

            const size_t dir_len = strlen(directory_name);
            const size_t name_len = strlen(filename);

            char *path = malloc_or_exit(dir_len + name_len + 2); // extra / and 0

            memcpy(path, directory_name, dir_len);
            path[dir_len] = '/';
            memcpy(path + dir_len + 1, filename, name_len + 1); // +1 to copy the null-terminator

            wf->path = path;
        }
        log_debug("Registered file %s/%s", directory_name, filename);
    } else {
        log_error("Watching too many files");
    }
}

static void handle_events() {
    /* Some systems cannot read integer variables if they are not
       properly aligned. On other systems, incorrect alignment may
       decrease performance. Hence, the buffer used for reading from
       the inotify file descriptor should have the same alignment as
       struct inotify_event. */

    char buf[4096]
            __attribute__ ((aligned(__alignof__(struct inotify_event))));

    /* Loop while events can be read from inotify file descriptor. */

    for (;;) {

        /* Read some events. */

        ssize_t len = read(fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN) {
            log_error("%s", strerror(errno));
            break;
        } else {

            /* If the nonblocking read() found no events to read, then
               it returns -1 with errno set to EAGAIN. In that case,
               we exit the loop. */

            if (len <= 0)
                break;

            /* Loop over all events in the buffer */

            const struct inotify_event *event;
            for (char *ptr = buf; ptr < buf + len;
                 ptr += sizeof(struct inotify_event) + event->len) {

                event = (const struct inotify_event *) ptr;

                /* Find WatchedFile with matching watch descriptor and filename */

                const char *debug_dir_name = "";
                for (int i = 0; i < watched_files_count; ++i) {
                    WatchedFile it = watched_files[i];

                    if (it.watch_descriptor == event->wd) {
                        debug_dir_name = it.directory_name;

                        if ((event->mask & IN_ISDIR) == 0 && event->len > 0 && strcmp(it.filename, event->name) == 0) {
                            it.callback(it.path);
                            break;
                        }
                    }

                }

                log_trace("wd: %x, mask: %x, cookie: %x, len: %x, name: %s, dir: %s",
                          event->wd,
                          event->mask,
                          event->cookie,
                          event->len,
                          event->name,
                          debug_dir_name);
            }
        }
    }
}

int update_filereload() {
    nfds_t nfds = 1;

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    int poll_num = poll(fds, nfds, 0); // no timeout

    if (poll_num == -1) {
        log_error("%s", strerror(errno));
    } else if (poll_num > 0) {
        if (fds[0].revents & POLLIN) {
            handle_events();
        }
    }
}


