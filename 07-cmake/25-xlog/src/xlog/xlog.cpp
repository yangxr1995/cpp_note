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

#include <sstream>
#include "xlog_thread.h"
#include <sstream>
#include <iomanip>
#include <memory>
#include <time.h>
#include "xlog.h"
using namespace std;

_XCPP_NAMESPACE_ 

unique_ptr<XWrite> XLogMessage::write_;
static const char* LogLevelName[]{
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"FATAL"
};
	
//只在测试时用
bool CloseXLog()
{
	XLogMessage::set_write(nullptr);
	return true;
}
bool InitXLog(const char* appname, const char* logpath, const char* filename)
{
	cout << "Init XLog appname = " << appname <<" path = "<< logpath << endl;
	unique_ptr<XLogThread> xt(new XLogThread());
	if (!xt->Init(logpath, filename))
		return false;
	xt->Start();
	XLogMessage::set_write(move(xt));
	return true;
}
XLogMessage::~XLogMessage()
{
	if(write_)
		write_->Write(stream_.str().c_str());
}

XLogMessage::XLogMessage(const char* file, int line, LogLevel level)
{
	auto time_now = chrono::system_clock::now();
	time_t t = std::chrono::system_clock::to_time_t(time_now);

	auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());

	tm tt;
	//带时区时间
	#if _WIN32
	::localtime_s(&tt, &t);
	#else 
	::localtime_r(&t, &tt);
	#endif
     
	LogLevelName[1];
	int le = static_cast<int>(level);

	stream_ << "["<<LogLevelName[le] << ' ' << setfill('0')
		<< setw(4) << tt.tm_year + 1900<<"-"
		<< setw(2) << tt.tm_mon + 1 << "-"
		<< setw(2) << tt.tm_mday
		<< ' '
		<< setw(2) << tt.tm_hour << ':'
		<< setw(2) << tt.tm_min << ':'
		<< setw(2) << tt.tm_sec << '.'
		<< setw(6) << (int)((duration_in_ms % 1000).count())
		<< ' '
		<< setfill(' ') <<" (" << this_thread::get_id()<<")" << setfill('0')
		<< ' '
		<< file << ':' << line << "] ";

}
XLogMessage::XLogMessage()
{

}

_END_NAMESPACE_
