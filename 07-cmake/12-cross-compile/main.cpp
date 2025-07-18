    #include <iostream>
    void B();
    int main() {
        B();
        std::cout << "main : " << STATIC << std::endl;
        return 0;
    }
    