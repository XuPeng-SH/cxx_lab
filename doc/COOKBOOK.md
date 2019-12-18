# COOKBOOK

## [Pointer to class data member](https://stackoverflow.com/questions/670734/pointer-to-class-data-member)

> The following code illustrates the use of "pointer to a member"
```cpp
#include <iostream>
using namespace std;

class Car
{
    public:
    int speed;
};

int main()
{
    int Car::*pSpeed = &Car::speed;

    Car c1;
    c1.speed = 1;       // direct access
    cout << "speed is " << c1.speed << endl;
    c1.*pSpeed = 2;     // access via pointer to member
    cout << "speed is " << c1.speed << endl;
    return 0;
}
```
## [SFINAE - Substitution Failure Is Not An Error](https://en.cppreference.com/w/cpp/language/sfinae)

[Example](../template_programming/src/serializer.hpp)
```cpp
#include "serializer.hpp"

int main() {
    int int_val = 3;
    const char* char_val = "this is const char";

    Serializer<int>::type::serialize(int_val);
    Serializer<const char*>::type::serialize(char_val);
    Serialize<int>(int_val);

    return 0;
}
```
