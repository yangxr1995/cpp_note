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
#include "xlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <cstdio>
using namespace std;
using namespace xcpp;
#define TESTLOG "testlog.txt"
class XLogTest : public ::testing::Test {
 protected:
  XLogTest() {

  }

  ~XLogTest() override {
  }

  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
     cout<<"SetUp"<<endl;
     std::remove(TESTLOG);
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
     cout<<"TearDown"<<endl;
  }
  // Class members declared here can be used by all tests in the test suite
  // for Foo.
};
TEST_F(XLogTest, InitXLog) {
    auto re = xcpp::InitXLog("MyApp","./", TESTLOG);
    ASSERT_TRUE(re);
    xcpp::CloseXLog();
    ifstream ifs(TESTLOG);
    ASSERT_TRUE(ifs);
    if(ifs)ifs.close();
    std::remove(TESTLOG);
    //Fatal assertion	Nonfatal assertion
    //ASSERT_TRUE(condition);	EXPECT_TRUE(condition);
}

TEST_F(XLogTest, WriteLog) {
    auto re = xcpp::InitXLog("MyApp", "./", TESTLOG);
    ASSERT_TRUE(re);
    xcpp::CloseXLog();
    ifstream ifs(TESTLOG);
    ASSERT_TRUE(ifs);
    if (ifs)ifs.close();
    std::remove(TESTLOG);
    //Fatal assertion	Nonfatal assertion
    //ASSERT_TRUE(condition);	EXPECT_TRUE(condition);
}

int main(int argc,char *argv[])
{
#ifdef XCPP_EXPORTS
    cout << " has XCPP_EXPORTS" << endl;
#else
    cout << " no XCPP_EXPORTS" << endl;
#endif
#ifdef XCPP_STATIC
    cout << " has XCPP_STATIC" << endl;
#else
    cout << " no XCPP_STATIC" << endl;
#endif
    
    //bool re = InitXLog("MyApp", "./", "test.ini");
    ////ASSERT_TRUE(re);
    //xcpp::CloseXLog();
    ////ifstream ifs(TESTLOG);
    ////ASSERT_TRUE(ifs);
    ////std::remove(TESTLOG);
    //int x;
    //cin >> x;
    //cout<<"test xlog"<<endl;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
