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

#ifndef XTHREAD_POOL_H
#define XTHREAD_POOL_H
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include <functional>
#include "xlib.h"
namespace xcpp
{  
    //线程池 
    class  XCPP_API XThreadPool
    {
    public:
        //启动所有线程
        void Start(int num);
        void Stop();

        template<class F, class...Args>
        // future 获取返回值
        auto Exec(F&& f, Args&&...args)->
            std::future<decltype(f(args...))>
        {
            using Ret = decltype(f(args...));
            //auto f1 = std::bind(f, args...);
            //f1()
            auto task = std::make_shared<std::packaged_task<Ret()>>(
                std::bind(f, args...));
            auto task_func = [task] {
                (*task)();
            };
            tasks_.push(task_func);
            cv_.notify_one();
            return task->get_future();
            //task();
        }


    private:

        //线程池函数
        void Run();
        //所有的线程句柄
        std::vector<std::shared_ptr<std::thread> >threads_;
        //任务队列
        std::queue<std::function<void()>> tasks_;
        std::mutex mux_;

        std::condition_variable cv_;
        bool is_exit_ = false;
    };
}
#endif  
