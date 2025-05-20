#pragma once
class Epoll;
class Channel;
class EventLoop
{
private:
    Epoll *ep;
    ThreadPool *threadPool;
    bool quit;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);
    void addThread(std::function<void()>);
};