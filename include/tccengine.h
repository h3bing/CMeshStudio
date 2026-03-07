#ifndef TCCENGINE_H
#define TCCENGINE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "libtcc.h"

class PTCCEngine
{
private:
  TCCState* m_tccState;
  std::string m_errorMessage;
  std::string m_libPath;
  
  // 错误处理回调
  static void errorCallback(void* opaque, const char* msg);
  void registerSymbols();
  
  // 错误消息回调函数类型
  using ErrorCallbackFunc = std::function<void(const std::string&)>;
  ErrorCallbackFunc m_errorCallbackFunc;

public:
  PTCCEngine();
  ~PTCCEngine();
  
  bool initialize();
  bool compile(const std::string& code);
  bool execute();
  std::string errorMessage() const { return m_errorMessage; }
  
  void addIncludePath(const std::string& path);
  void addLibraryPath(const std::string& path);
  void addLibrary(const std::string& library);
  void setLibPath(const std::string& path) { m_libPath = path; }
  
  // 设置错误回调函数
  void setErrorCallback(ErrorCallbackFunc callback) { m_errorCallbackFunc = callback; }
};

// 全局句柄管理器
class PHandleManager
{
private:
  static std::map<int, std::shared_ptr<void>> m_handles;
  static int m_nextHandle;

public:
  static int addHandle(std::shared_ptr<void> object);
  static std::shared_ptr<void> getHandle(int handle);
  static void removeHandle(int handle);
  static bool isValidHandle(int handle);
};

#endif // TCCENGINE_H