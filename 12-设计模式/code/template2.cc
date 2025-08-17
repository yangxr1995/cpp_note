#include <iostream>
using namespace std;

class ZooShow {
public:
    void Show() {
        // 如果子表演流程没有超时的话，进行一个中场游戏环节；
        if (Show0())
            PlayGame();
        Show1();
        Show2();
        Show3();
    }

private:
    void PlayGame() {
        cout << "after Show0, then play game" << endl;
    }
    // 对其他用户关闭，但是子类开放的
protected:
    bool expired;
    virtual bool Show0() {
        cout << "show0" << endl;
        if (!expired) {
            return true;
        }
        return false;
    }
    virtual void Show2() {
        cout << "show2" << endl;
    }
    virtual void Show1() {
    }
    virtual void Show3() {
    }
};

// ZooShowEx10 类
class ZooShowEx10 : public ZooShow {
protected:
    virtual bool Show0() {  // 修正返回值以匹配基类的 virtual bool Show0()
        if (!expired) {
            return true;
        }
        return false;
    }
};

// ZooShowEx1 类
class ZooShowEx1 : public ZooShow {
protected:
    virtual bool Show0() {
        cout << "ZooShowEx1 show0" << endl;
        if (!expired) { // 里氏替换注释
            return true;
        }
        return false;
    }
    virtual void Show2() {
        cout << "show3" << endl;
    }
};

// ZooShowEx2 类
class ZooShowEx2 : public ZooShow {
protected:
    virtual void Show1() {
        cout << "show1" << endl;
    }
    virtual void Show2() {
        cout << "show3" << endl;
    }
};

// ZooShowEx3 类
class ZooShowEx3 : public ZooShow {
protected:
    virtual void Show1() {
        cout << "show1" << endl;
    }
    virtual void Show3() {
        cout << "show3" << endl;
    }
    virtual void Show4() {
        // 空实现
    }
};

int main (int argc, char *argv[]) {
    ZooShow *zs = new ZooShowEx2();
    zs->Show();
    zs = new ZooShowEx3();
    zs->Show();
    return 0;
}
