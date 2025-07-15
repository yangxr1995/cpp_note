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

#include <iostream>
#include "xthread_pool.h"
#include "gtest/gtest.h"
using namespace std;

 
//namespace {

// The fixture for testing class Foo.
class XThreadPoolTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if their bodies would
  // be empty.

  XThreadPoolTest() {
     // You can do set-up work for each test here.
  }

  ~XThreadPoolTest() override {
     // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:
  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
     cout<<"SetUp XThreadPoolTest"<<endl;
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
     cout<<"TearDown XThreadPoolTest"<<endl;
  }
  // Class members declared here can be used by all tests in the test suite
  // for Foo.
};

// Tests that the Foo::Bar() method does Abc. 测试
TEST_F(XThreadPoolTest, Init) {
  EXPECT_EQ(7 * 6, 42);
}

int main(int argc,char *argv[])
{
    cout<<"test xthread"<<endl;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
