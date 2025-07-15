//////////////////////////////////  @版权说明  //////////////////////////////////////////////////
///						Jiedi(China nanjing)Ltd.                                    
/// @版权说明 代码和课程版权有夏曹俊所拥有并已经申请著作权，此代码可用作为学习参考并可在项目中使用，
/// 课程中涉及到的其他开源软件，请遵守其相应的授权
/// 课程源码不可以直接转载到公开的博客，或者其他共享平台，不可以用以制作在线课程。
/// 课程中涉及到的其他开源软件，请遵守其相应的授权  @@              
/////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////  源码说明  //////////////////////////////////////////////////
/// 项目名称: CMake构建大型c++项目
/// 博客   : 			    http://blog.csdn.net/jiedichina
/// 腾讯课堂			    https://jiedi.ke.qq.com/
/// 学浪				    搜索 夏曹俊
/// 老夏课堂			    http://cppds.com 
/// CMake手册    	        http://cmake.org.cn
/// ！！！请加入课程qq群 【296249312】与同学交流 
/// 欢迎加夏曹俊老师的微信：cppxcj
///加入老夏课堂群【296249312】直接联系群里客服下载课程资料
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// 课程交流qq群296249312 //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////// COMMENT ///////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/// @mainpage xcpp 开源精简c++库 日志、线程池 、网络
/// @brief
/// - 项目有夏曹俊老师发起，所有的学员都可以参与此开源项目，做出贡献，并写在项目贡献列表中
/// - 项目基于c++ 11 14 17 版本开发（20 22 普及后引入），使用c++新思维开发，区别于c风格c++
/// - 项目构建工具基于cmake ，项目跨平台已经支持windows linux（x86） macos Android openharmony harmonyos 
/// - 计划支持linux(arm) ios
/// 
/// @author 夏曹俊 （参与学员。。。） 
/// @version 1.0.1
/// @date 2022年08月10日
///////////////////////////////////////////////////////////////////////////
#ifndef XLOG_H
#define XLOG_H
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>

#include "xconfig.h"

#ifdef _WIN32
#define DEFAULT_LOG_PATH "./"
#else
#define DEFAULT_LOG_PATH "/var/log/xcpp/"
#endif
#define DEFAULT_LOG_FILENAME  "xlog.txt"

/////////////////////////////////////////////////
/**
 * @file xlog.h
 * @brief  日志接口类文件
 *
 * 使用说明:
 * - 1
 * - 2
 * >- 2.1
 * >- 2.2
 * @code{.cpp}
 * LOGDEBUG<<"test log debug"<<10;
 * LOGINFO<<"test log info "<<10.1;
 * @endcode
 * @author xiacaojun@qq.com
 * @copyright { 版权由夏曹俊和开源贡献者共同拥有，不可用于在线教育，可用于其他商业应用。 }
 */
 
_XCPP_NAMESPACE_

/** @brief 日志级别 !<生成表格
*/
enum class LogLevel
{
	DEBUG,			//!< 调试级别日志
	INFO,			//!< 正常操作日志
	WARNING,		//!< 警告
	ERROR,			//!< 错误
	FATAL			//!< 导致中断的错误 
};
#define LOG(level) xcpp::XLogMessage(__FILE__, __LINE__, xcpp::LogLevel::level).stream()
#define LOGDEBUG LOG(DEBUG)	
#define LOGINFO LOG(INFO)
#define LOGWARNING LOG(WARNING)
#define LOGERROR LOG(ERROR)
#define LOGFATAL LOG(FATAL)
	

/**
 * @brief  初始化日志模块，会开启日志写入线程
 * @param appname	产生日志的应用名称
 * @param logpath	日志的写入路径，以后支持网络路径
 * @param filename 日志写入的文件命令，以后支持格式化名称，滚动生成日志
 * @return 初始化成功失败 单元测试用
 */
XCPP_API bool InitXLog(const char* appname,const char *logpath = DEFAULT_LOG_PATH, const char* filename = DEFAULT_LOG_FILENAME);
	
/**
 * @brief  测试用，清理日志线程和对象
 * @return 成功返回true 
 */
XCPP_API bool CloseXLog(); 

/**
 * @brief 日志写入接口
 */
class XCPP_API XWrite
{
public:
	virtual int Write(const char* msg) = 0;
	XWrite(){}
	virtual ~XWrite() {}
};
 
/**
 * @brief 日志消息处理类.
 * 接收输入的日志消息，发送到写入线程
 */
class XCPP_API XLogMessage
{ 
public:
	/**
	 * @brief  输入流
	 * @return stringstream 字符串流，写入其中，对象析构中会发送给写入线程
	 */
	std::stringstream& stream()
	{
		return stream_;
	}
	XLogMessage();

	/**
	 * @brief 构造函数.
	 * @brief Constructor.
	 *
	 * @param[in] file		日志产生所在文件名
	 * @param[in] line		日志产生行号
	 * @param[in] level		日志级别
	 */
	XLogMessage(const char* file, int line, LogLevel level);
	~XLogMessage() ;
	static void set_write(std::unique_ptr<XWrite> &&w) { write_ = move(w); }
	void set_name(const char* name) { name_ = name; }
private:
	/**
	 * 日志字符串流
	 */
	std::stringstream stream_;
	
	/**
	 * 注入的写处理对象
	 */
	static std::unique_ptr<XWrite> write_;

	std::string name_;
};

_END_NAMESPACE_

#endif