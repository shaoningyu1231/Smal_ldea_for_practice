# include "Epoll.h"
#include "util.h"
#include <unistd.h>
#include <string.h>
#include "Channel.h"

#define MAX_EVENTS 1000

// Epoll class constructor
Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}

// Add a file descriptor to the epoll instance
// void Epoll::addFd(int fd, uint32_t op) {
//     struct epoll_event ev;
//     bzero(&ev, sizeof(ev));
//     ev.data.fd = fd;
//     ev.events = op;
//     errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error");
// }

/* Wait for events on the epoll instance
std::vector<epoll_event> Epoll::poll(int timeout) {
    std::vector<epoll_event> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        activeEvents.push_back(events[i]);
    }
    return activeEvents;
} */

// Wait for events on the epoll instance and return active channels
std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> activeChannels;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        Channel *ch = (Channel*)events[i].data.ptr;
        ch->setReady(events[i].events);
        activeChannels.push_back(ch);
    }
    return activeChannels;
}

// Update the epoll instance with the channel's events
void Epoll::updateChannel(Channel *channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error");
        channel->setInEpoll();
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify event error");
    }
}

// Delete the channel from the epoll instance
void Epoll::deleteChannel(Channel *channel) {
    int fd = channel->getFd();
    errif(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1, "epoll delete event error");
    channel->setInEpoll(false);
}
