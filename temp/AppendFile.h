#pragma once
#include <string>

class AppendFile
{
 public:
  explicit AppendFile(std::string filename);

  ~AppendFile();

  void append(const char* logline, size_t len);

  void flush();

  off_t writtenBytes() const { return writtenBytes_; }

 private:

  size_t write(const char* logline, size_t len);

  FILE* fp_;
  char buffer_[64*1024];
  off_t writtenBytes_;          // 已经写入了多少字节
};
