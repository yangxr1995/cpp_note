class DisplayA {
public:
    void Show(float temperature);
};

class DisplayB {
public:
    void Show(float temperature);
};

class DisplayC {
public:
    void Show(float temperature);
};

class WeatherData {
};

class DataCenter {
public:
    // 缺陷: 增减Display 都会导致 DataCenter 类被修改
    void notify() {
        DisplayA *da = new DisplayA;
        DisplayB *db = new DisplayB;
        DisplayC *dc = new DisplayC;
        float temper = this->CalcTemperature();
        da->Show(temper);
        db->Show(temper);
        dc->Show(temper);
    }
private:
    float CalcTemperature() {
        WeatherData * data = GetWeatherData();
        // ...
        float temper/* = */; 
        return temper;
    }

    WeatherData * GetWeatherData(); // 不同的方式
};

// 订阅发布
int main() {
    DataCenter *dataCenter = new DataCenter();
    dataCenter->notify();

    return 0;
}
