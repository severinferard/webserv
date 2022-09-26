#ifndef FAKE_EPOLL_H
#define FAKE_EPOLL_H

# include <sys/types.h> 

#ifndef __APPLE__
# include <sys/epoll.h>
#else

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

int epoll_create(int size);
int epoll_create1(int flags);
int epoll_wait(int epfd, struct epoll_event *events,
                      int maxevents, int timeout);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* Epoll events */
    epoll_data_t data;        /* User data variable */
};

#endif

#endif /* FAKE_EPOLL_H */
