#include "utils.hpp"
#include <windows.h>

class work {
private:
    HANDLE m_work_event_handle;
public:
    static work &instance() {
        static work inst;
        return inst;
    }
    void run() {
        m_work_event_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
        WaitForSingleObject(m_work_event_handle, INFINITE); // 无限等待，用于模拟一个服务不断运行的功能
        WriteLog("wait end\n");
    }
    void stop() {
        SetEvent(m_work_event_handle);
    }    
};