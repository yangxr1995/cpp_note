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

#include "xlog_thread.h"
#include <chrono>
using namespace std;
_XCPP_NAMESPACE_

bool XLogThread::Init(const char* log_path, const char* filename)
{
	//is_log_init = false;
	this->log_path_ = log_path;
	//测试用，后期改为滚动日志文件
	string logfile = log_path;
	logfile += filename;
	ofs_.open(logfile, ios::app);
	if (ofs_)
		return true;
	return false;
}

int XLogThread::Write(const char* msg)
{
	{
		lock_guard<mutex> lock(mux_);
		msgs_.push(msg);  
	}
	cv_.notify_one();
	return 0;
}

void XLogThread::Start()
{
	th_ = thread(&XLogThread::Run, this);
}
void XLogThread::Stop()
{
	if (!th_.joinable())
		return;
	is_exit_ = true;
	cv_.notify_all();
	th_.join();
}

void XLogThread::Run()
{
	for(;;)
	{
		string msg;
		unique_lock<mutex> lock(mux_);	
		cv_.wait(lock, [this] 
        {
            return is_exit_||!msgs_.empty();
        });
		while(!msgs_.empty())
		{
			msg = move(msgs_.front());
			msgs_.pop();
			lock.unlock();
			ofs_ << msg << endl;
			cout << msg << endl;
			lock.lock();
		}
		if(is_exit_)
			break;
	}
}


_END_NAMESPACE_