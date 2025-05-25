#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>

Channel::Channel(EventLoop *_loop, int _fd) : loop(_loop), fd(_fd), events(0), ready(0), inEpoll(false){
    // Constructor implementation
}

Channel::~Channel() {
    // Destructor implementation
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Channel::handleEvent() {
    if (ready & (EPOLLIN | EPOLLPRI)) {
        readCallback();
    }
    if (ready & EPOLLOUT) {
        writeCallback();
    }
}

void Channel::enableReading() {
    events |= EPOLLIN | EPOLLPRI;
    loop->updateChannel(this);
}

void Channel::setReady(uint32_t _ev) {
    ready = _ev;
}

uint32_t Channel::getReady() {
    return ready;
}

int Channel::getFd() {
    return fd;
}

uint32_t Channel::getEvents() {
    return events;
}

bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setInEpoll(bool _in) {
    inEpoll = _in;
}

void Channel::setReadCallback(std::function<void()> _cb) {
    readCallback = _cb;
}

void Channel::useET() {
    events |= EPOLLET;
    loop->updateChannel(this);
}