# 
```cc
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
using namespace std;

int main (int argc, char *argv[]) {

    // 文本文件，创建新文件，并写入
    ofstream outFile("1.tmp");
    outFile << "Hello\n123\n";
    outFile.close();

    // 文本文件，读取
    ifstream inFile("1.tmp");
    string line;
    while (getline(inFile, line)) {
        cout << line << endl;
    }
    inFile.close();

    // 二进制文件，写入
    int data = 42;
    ofstream binOut("1.bin", ios::binary);
    binOut.write(reinterpret_cast<char *>(&data), sizeof(data));
    binOut.close();

    // 二进制文件，读
    int readData;
    ifstream binIn("1.bin", ios::binary);
    binIn.read(reinterpret_cast<char *>(&readData), sizeof(readData));
    binIn.close();
    cout << readData << endl;

    // 同时读写
    {
        fstream file("2.tmp", ios::in | ios::out | ios::trunc);
        file << "111\n222\n";
        file.seekg(0, ios::beg);
        string line;
        while (getline(file, line)) {
            cout << "Read: " << line << endl;
        }
        file.close();
    }

    
    return 0;
}
```

**模式组合说明**：
- `std::ios::in | std::ios::out`：允许同时读写
- `std::ios::trunc`：若文件存在则清空内容
- `std::ios::app`：保留原有内容（追加模式）
- `std::ios::ate`：打开时定位到文件尾部


```cc
/// Request a seek relative to the beginning of the stream.
static const seekdir beg =		_S_beg;
/// Request a seek relative to the current position within the sequence.
static const seekdir cur =		_S_cur;
/// Request a seek relative to the current end of the sequence.
static const seekdir end =		_S_end;
```
