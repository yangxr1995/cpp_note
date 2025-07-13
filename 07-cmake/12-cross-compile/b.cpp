    #include <iostream>
    void A();
    void B() {
        A();
    std::cout << "B : " << A_VAR << std::endl;
    // std::cout << "B : " << STATIC << std::endl; // error
    }
    