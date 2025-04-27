#ifndef _FIBER_H__
#define _FIBER_H__

#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
#include <functional>
#include <iostream>
#include <memory>
#include "utils.hpp"

// 协程是一个轻量级的线程，协程的切换不需要内核的参与，
// 只需要用户态的上下文切换即可，协程的切换速度比线程快得多
// 协程的切换是通过保存和恢复上下文来实现的，
// 协程的上下文包括寄存器的值、栈指针、程序计数器等
// 协程的栈是由用户自己分配的，
// 协程的栈大小可以通过参数传入，

// 继承std::enable_shared_from_this类，
// 使得协程对象可以通过shared_from_this()函数获取自身的shared_ptr
// 这样可以避免协程对象在栈上创建时，
// 由于协程对象的生命周期比协程函数的生命周期长，
// 导致协程对象被销毁而协程函数仍然在执行的情况

class Fiber : public std::enable_shared_from_this<Fiber> {
 public:
  typedef std::shared_ptr<Fiber> ptr;
  // Fiber状态机
  enum State {
    // 就绪态，刚创建后者yield后状态
    READY,
    // 运行态，resume之后的状态
    RUNNING,
    // 结束态，协程的回调函数执行完之后的状态
    TERM,
  };

 private:
  // 初始化当前线程的协程功能，构造线程主协程对象
  Fiber();

 public:
  // 构造子协程
  Fiber(std::function<void()> cb, size_t stackSz = 0, bool run_in_scheduler = true);
  ~Fiber();
  // 重置协程状态，复用栈空间
  void reset(std::function<void()> cb);
  // 切换协程到运行态
  void resume();
  // 让出协程执行权
  void yield();
  // 获取协程Id
  uint64_t getId() const { return id_; }
  // 获取协程状态
  State getState() const { return state_; }

  // 设置当前正在运行的协程
  static void SetThis(Fiber *f);
  // 获取当前线程中的执行线程
  // 如果当前线程没有创建协程，则创建第一个协程，且该协程为当前线程的
  // 主协程，其他协程通过该协程来调度
  static Fiber::ptr GetThis();
  // 协程总数
  static uint64_t TotalFiberNum();
  // 协程回调函数
  static void MainFunc();
  // 获取当前协程Id
  static uint64_t GetCurFiberID();

 private:
  // 协程ID
  uint64_t id_ = 0;
  // 协程栈大小
  uint32_t stackSize_ = 0;
  // 协程状态
  State state_ = READY;
  // 协程上下文
  ucontext_t ctx_;
  // 协程栈地址
  void *stack_ptr = nullptr;
  // 协程回调函数
  std::function<void()> cb_;
  // 本协程是否参与调度器调度
  bool isRunInScheduler_;
};

#endif