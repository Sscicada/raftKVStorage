#include <chrono>
#include <iostream>

#include <raftCore/timer.h>

using namespace std;
using namespace std::chrono;

void onHeartbeat() {
    cout << "Heartbeat sent at " << duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count() << " ms" << endl;
}

int main() {
    Timer heartbeatTimer(onHeartbeat, milliseconds(1000));  // 每隔 1 秒触发
    heartbeatTimer.start();

    this_thread::sleep_for(seconds(5));  // 主线程等待 5 秒
    heartbeatTimer.stop();               // 停止定时器
    return 0;
}
