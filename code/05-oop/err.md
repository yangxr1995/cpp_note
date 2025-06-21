# C++异常处理与日志系统的实现方案

在C++开发中，虽然异常机制与C语言的错误码返回方式有本质差异，但通过合理设计异常处理流程和日志系统，可以实现类似C语言中逐层检查错误并记录日志的效果。以下是具体实现方案及技术要点：

## 一、自定义异常类携带上下文信息

通过继承`std::exception`创建自定义异常类，在抛出时封装错误描述、代码位置等关键信息：

```cpp
class LoggableException : public std::exception {
public:
    LoggableException(const std::string& msg, const char* file, int line) 
        : msg_(msg + " [File:" + file + " Line:" + std::to_string(line) + "]") {}
    
    const char* what() const noexcept override { 
        return msg_.c_str(); 
    }
private:
    std::string msg_;
};

// 使用宏简化抛出操作
#define THROW_EX(msg) throw LoggableException(msg, __FILE__, __LINE__)

void riskyFunction() {
    if (error_occurred) {
        THROW_EX("Failed to open file"); // 自动记录文件路径和行号
    }
}
```

此方法通过异常对象携带错误发生的具体位置（文件、行号），便于日志定位问题。

---

## 二、函数级异常捕获与日志记录

在每个可能抛出异常的函数入口处添加`try-catch`块，记录错误上下文后重新抛出：

```cpp
void wrappedFunction() {
    try {
        riskyFunction(); // 实际业务逻辑
    } 
    catch (const LoggableException& e) {
        std::cerr << "Error in wrappedFunction: " << e.what() << std::endl;
        throw; // 重新抛出供上层处理
    }
}
```

通过逐层包装函数调用，可在每个层级记录错误日志，形成类似C语言逐层检查的效果。

---

## 三、利用RAII实现自动堆栈追踪

结合智能指针和析构函数特性，在对象销毁时自动记录调用链：

```cpp
class StackTracer {
public:
    StackTracer(const std::string& funcName) : funcName_(funcName) {
        callStack_.push(funcName_);
    }
    ~StackTracer() {
        if (std::uncaught_exceptions()) { // C++17特性
            std::cerr << "Unwinding stack: ";
            while (!callStack_.empty()) {
                std::cerr << callStack_.top() << " <- ";
                callStack_.pop();
            }
        }
    }
private:
    static std::stack<std::string> callStack_;
    std::string funcName_;
};

// 使用宏自动注入函数名
#define FUNCTION_TRACE StackTracer stackTracer(__func__)

void exampleFunction() {
    FUNCTION_TRACE;
    // 函数逻辑...
}
```

当异常未被捕获时，析构函数会输出未完成调用的函数链，辅助定位问题根源。

---

## 四、全局异常处理器统一记录

通过`std::set_terminate`设置全局异常处理，结合`backtrace`库获取完整堆栈：

```cpp
#include <execinfo.h> // Linux专用

void globalHandler() {
    void* array[50];
    size_t size = backtrace(array, 50);
    char** symbols = backtrace_symbols(array, size);
    
    std::ofstream log("crash.log");
    for (size_t i = 0; i < size; ++i) {
        log << symbols[i] << std::endl;
    }
    free(symbols);
    std::abort();
}

int main() {
    std::set_terminate(globalHandler);
    // 主程序逻辑...
}
```

此方案适用于Linux系统，可生成完整的调用堆栈快照。

---

## 五、异常与日志系统的深度整合

### 1. 结构化日志输出
使用日志库（如`spdlog`）格式化输出异常信息：

```cpp
#include <spdlog/spdlog.h>

try {
    riskyOperation();
} catch (const std::exception& e) {
    spdlog::error("Exception caught: {} - Call stack: {}", e.what(), getStackTrace());
}
```

### 2. 异步日志记录
通过队列实现非阻塞日志写入，避免异常处理影响主程序性能。

---

## 对比C语言方案的优劣

| 特性                | C语言返回值检查       | C++异常机制          |
|---------------------|----------------------|----------------------|
| 错误传播效率        | 显式逐层返回         | 自动栈展开           |
| 代码侵入性          | 高（大量`if`判断）   | 低（集中处理）       |
| 上下文信息丰富度    | 需手动传递           | 异常对象封装         |
| 性能开销            | 无额外开销           | 栈展开有运行时成本   |
| 多线程安全性        | 容易遗漏同步         | 自动保证异常安全     |

---

## 最佳实践建议

1. **异常粒度控制**：仅在真正异常场景（如IO失败、内存不足）使用异常，避免用于常规控制流。
2. **异常安全等级**：遵循基本保证（No-leak）→ 强保证（Transactional）→ 不抛保证（No-throw）的层次设计。
3. **结合新特性**：C++23的`std::expected`可替代部分异常场景，实现零开销错误处理。

通过上述方法，C++异常机制不仅能实现C语言风格的细粒度错误追踪，还能通过面向对象特性提供更强大的诊断信息，同时保持代码的简洁性。实际项目中建议结合具体需求选择混合策略，例如关键路径使用异常+日志，性能敏感模块改用错误码。

