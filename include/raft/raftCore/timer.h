#ifndef TIMER_H
#define TIMER_H

#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <functional>
#include <condition_variable>

class Timer {
public:
    // 构造函数，传入回调函数和时间间隔
    Timer(std::function<void()> callback, std::chrono::milliseconds interval, bool repeat = true) 
        : callback_(callback), interval_(interval), repeat_(repeat),active_(false) {}
    
    void start() {
        std::lock_guard<std::mutex> lck(mutex_);
        if (active_) return;  // 避免重复调用 start
        active_ = true;
        // worker_ 持有线程对象
        worker_ = std::thread([this]() {
            std::unique_lock<std::mutex> lock(mutex_);
            do {
                if (cv_.wait_for(lock, interval_, [this] { return !active_; }))
                    break;  // 被 stop() 提前唤醒
                lock.unlock();       // 执行任务前前先释放锁
                callback_();         // 执行任务
                lock.lock();         // 重新上锁以便下轮等待
            } while (repeat_ && active_);
        });
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!active_) {
            return;
        }
        active_ = false;
        cv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    ~Timer() {
        stop();
    }        
    
private:
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::function<void()> callback_;
    std::chrono::milliseconds interval_;
    std::atomic<bool> active_, repeat_;              // 由多个线程读写（主线程和定时线程）
};

#endif // TIMER_H