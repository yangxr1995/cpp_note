#include <iostream>
#include <bitset>
#include <string>
using namespace std;

int main (int argc, char *argv[]) {

    bitset<8> a(string("10100101"));
    cout << "orig data : " << a.to_string() << endl;
    for (int i = 0; i < 4; i++) {
        a.flip(i);
    }
    cout << "new data  : " << a.to_string() << endl;
    a ^= bitset<8>(string("00001111"));
    cout << "new data  : " << a.to_string() << endl;
    
    return 0;
}
