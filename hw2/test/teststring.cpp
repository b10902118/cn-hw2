#include <iostream>
#include <string>
using namespace std;

int main() {
    string s = "abc";
    s += "\0";
    s += "def";
    cout << s.length() << endl;
    cout << s << endl;
    return 0;
}
