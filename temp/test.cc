#include "AsyncLogging.h"
#include "Logging.h"
#include "LogStream.h"

AsyncLogging* g_asyncLog = NULL;

inline AsyncLogging* getAsyncLog()
{
    return g_asyncLog;
}

void asyncLog(const char* msg, int len)
{
    AsyncLogging* logging = getAsyncLog();
    if (logging)
    {
        logging->append(msg, len);
    }
}

int main(int argc, char* argv[])
{

    {
        AsyncLogging log(argv[0]);
        g_asyncLog = &log;
        Logger::setOutput(asyncLog);
        LOG_INFO << "hello log, i am testing...";
        log.start();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}