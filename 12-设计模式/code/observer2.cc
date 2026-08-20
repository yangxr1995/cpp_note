#include <list>
#include <iostream>
using namespace std;

// 接口类: 定义多
class IDisplay {
public:
    virtual void Show(float temperature) = 0;
    virtual ~IDisplay() {}
};

class DisplayA : public IDisplay {
public:
    virtual void Show(float temperature) {
        cout << "DisplayA Show" << endl;
    }
private:
    void jianyi();
};

class DisplayB : public IDisplay {
public:
    virtual void Show(float temperature) {
        cout << "DisplayB Show" << endl;
    }
};

class WeatherData {
};

// 应对稳定点: 抽象
// 应对变化点: 扩展(继承和组合(组合基类))
// 定义一
class DataCenter {
public:
    void Attach(IDisplay * ob) {
        //
    }
    void Detach(IDisplay * ob) {
        //
    }
    // 一变化，多跟着变化
    void Notify() {
        float temper = CalcTemperature();
        for (auto iter : obs) {
            iter->Show(temper);
        }
    }
private:
    WeatherData * GetWeatherData();
    float CalcTemperature() {
        WeatherData * data = GetWeatherData();
        // ...
        float temper/* = */;
        return temper;
    }
    // 通过组合基类实现扩展
    std::list<IDisplay*> obs;
};

int main() {
    // 单例模式
    DataCenter *center = new DataCenter;

    // A.cc
    IDisplay *da = new DisplayA();
    center->Attach(da);

    // B.cc
    IDisplay *db = new DisplayB();
    center->Attach(da);

    // main.cc
    center->Notify();

    return 0;
}
