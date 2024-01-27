#include <iostream>
#include <string>
using namespace std;

class Testing {
    string n;

public:
    Testing() {
        n = "ABC";
    };

    Testing(string name = "ABC") {
        n = name;
    }

    void print() {
        cout << "The value of the string is----> " << n;
    }
};

int main() {
    Testing test1;
    test1.print();

    return 0;
}