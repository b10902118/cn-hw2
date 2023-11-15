#include <iostream>
#include <cstring>
using namespace std;

int main() {
    int ret = strncasecmp("abc", "ABC", 3);
    cout << ret << endl;

    return 0;
}
