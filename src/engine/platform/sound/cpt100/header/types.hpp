#include <stdexcept>
#include <tuple>

class Byte {
public:
    // バイト型(0~255)
    Byte(int val) {
        if (val >= 0 && val <= 255) {
            value = val;
        } else {
            throw std::out_of_range("byte type can store only int values from 0 to 255");
        }
    }

    int toInt() {
        return value;
    }

private:
    int value;
};

