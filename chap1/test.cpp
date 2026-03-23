#include <iostream>

int x = 1919810, y = 1;

int f(int x) {
    std::cout << "addr(x) = " << &x << std::endl;
    std::cout << "x = " << x << std::endl;
    int j = 123; *(&j + 1) = 3872;
    std::cout << "addr(j) = " << &j << std::endl;
    std::cout << "x = " << x << std::endl;
}

int main() {
    std::cout << "addr(x) = " << &x << std::endl;
    std::cout << "addr(y) = " << &y << std::endl;
    std::cout << "x = " << x << std::endl;
    *(&y + 1) = 114514;
    std::cout << "x = " << x << std::endl;
    f(x);
}
