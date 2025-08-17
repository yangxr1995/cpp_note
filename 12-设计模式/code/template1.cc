// 背景
// 某个品牌动物园,有一套固定的表演流程,但是其中有若干个表演子流程可创新替换,以尝试迭代更新表演流程;
 
/*
 * 符合原则:
 *    面向接口
 *    接口隔离
 * 破环原则:
 *    开闭
 */

#include <iostream>
using namespace std;

class ZooShow {
public:
    // 使用 type 以尝试迭代更新表演流程
    ZooShow(int type = 1) : _type(type) {}

public:
    // 有一套固定的表演流程
    void Show() {
        if (Show0())
            PlayGame();
        Show1();
        Show2();
        Show3();
    }

private:
    int _type;
    
    void PlayGame() {
        cout << "after Show0, then play game" << endl;
    }
    
    // 每次迭代需要修改已有类
    bool Show0() {
        if (_type == 1) {
            return true;
        }
        else if (_type == 2) {
            // ...
        }
        else if (_type == 3) {
            // ...
        }
        cout << _type << " show0" << endl;
        return true;
    }

    void Show1() {
        if (_type == 1) {
            cout << _type << " show1" << endl;
        }
        else if (_type == 2) {
            cout << _type << " show1" << endl;
        }
        else if (_type == 3) {
            cout << _type << " show1" << endl;
        }
    }
    void Show2() {
        if (_type == 1) {
            cout << _type << " show2" << endl;
        }
        else if (_type == 2) {
            cout << _type << " show2" << endl;
        }
        else if (_type == 3) {
            cout << _type << " show2" << endl;
        }
    }
    void Show3() {
        if (_type == 1) {
            cout << _type << " show3" << endl;
        }
        else if (_type == 2) {
            cout << _type << " show3" << endl;
        }
        else if (_type == 3) {
            cout << _type << " show3" << endl;
        }
    }
};

int main (int argc, char *argv[]) {
    ZooShow *zs = new ZooShow(2);
    zs->Show();
    return 0;
}
