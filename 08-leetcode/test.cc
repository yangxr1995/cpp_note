#include <algorithm>
#include <iostream>
#include <iterator>
#include <unordered_map>


using namespace std;

int main (int argc, char *argv[]) {
    
    unordered_map<char , int > m = {
        {'a', 1},
        {'b', 2},
        {'c', 3},
    };

    auto pos = m.find('b');
    if (pos != m.end()) {
        m.erase(m.begin(), pos);
    }

    for (const auto &pair  : m) {
        cout << pair.first << " ";
    }
    cout << endl;
    return 0;
}
