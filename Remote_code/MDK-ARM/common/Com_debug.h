#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>
// 日志输出打印在CPU运行上非常占用资源 => 通过比特率可以计算  打印10字节左右大概需要1ms  非常影响飞机的飞行
// 所以在后续飞机需要正常飞行的时候  需要关闭打印功能
// 设计一个日志输出打印开关
#define DEBUG_LOG_ENABLE 1

#ifdef DEBUG_LOG_ENABLE

// 使用宏定义的方式 只打印文件名称 不打印路径名称
// strrchr()从后向前查找字符串中的字符
#define __FILE_NAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILE_NAME (strrchr(__FILE_NAME__, '/') ? strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)

// 使用宏定义的方式能实现打印日志之前 先添加文件名和行号
#define debug_printf(format, ...) printf("[%s:%d]  " format, __FILE_NAME, __LINE__, ##__VA_ARGS__)

#else
// 如果没有开启日志输出打印
#define debug_printf(format, ...)
#endif // ifdef


#endif //
