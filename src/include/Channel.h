#pragma once
#include <sys/epoll.h>
#include <functional>
class EventLoop;
class Channel
{   
private:
    EventLoop *loop;
    int fd;
    uint32_t events;
    uint32_t ready;
    bool inEpoll; // whether the channel is in epoll
    std::function<void()> readCallback; // read callback function to be called when the event occurs
    std::function<void()> writeCallback; // write callback function to be called when the event occurs
public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void handleEvent();
    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getReady();
    bool getInEpoll();
    void setInEpoll(bool _in = true);
    void useET(); // Use Edge Triggered mode
    // void setEvents(uint32_t);
    void setReady(uint32_t);
    void setReadCallback(std::function<void()>);
};