#include <iostream>
using namespace std;

#include "string.h"
#include "vector.h"

int main (int argc, char *argv[]) {
    Vector<String> v1;

    v1.push_back(String("111"));
    v1.push_back(String("222"));

    for (String &it : v1) {
        cout << it << endl;
    }

    return 0;
}
