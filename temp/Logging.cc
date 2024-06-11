#include "Logging.h"
#include "CurrentThread.h"

#include <errno.h>

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

void defaultOutput(const char *msg, int len)
{
    fwrite(msg, 1, len, stdout);
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::LogLevel g_logLevel = Logger::INFO;
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;


const char *strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

const char *LogLevelName[Logger::NUM_LOG_LEVELS] =
{
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
};

class T
{
public:
    explicit T(const char *str, unsigned len)
        : str_(str),
          len_(len)
    {
    }

    const char *str_;
    const unsigned len_;
};

inline LogStream &operator<<(LogStream &s, T v)
{
    s.append(v.str_, v.len_);
    return s;
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file, int line)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file)
{
    CurrentThread::tid();
    stream_ << Timestamp::now().toString() << " ";
    stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
    stream_ << T(LogLevelName[level], 6);
    if (savedErrno != 0)
    {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

Logger::Logger(SourceFile file, int line)
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{
}


Logger::~Logger()
{
    impl_.finish();
    const LogStream::Buffer &buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL)
    {
        g_flush();
        abort();
    }
}


inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
  s.append(v.data_, v.size_);
  return s;
}

void Logger::Impl::finish()
{
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}