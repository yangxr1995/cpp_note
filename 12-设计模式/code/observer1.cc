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
    float CalcTemperature() {
        WeatherData * data = GetWeatherData();
        // ...
        float temper/* = */; 
        return temper;
    }
private:
    WeatherData * GetWeatherData(); // 不同的方式
};

// 订阅发布
int main() {
    DataCenter *center = new DataCenter;
    DisplayA *da = new DisplayA;
    DisplayB *db = new DisplayB;
    DisplayC *dc = new DisplayC;
    float temper = center->CalcTemperature();
    da->Show(temper);
    db->Show(temper);
    dc->Show(temper);
    return 0;
}
