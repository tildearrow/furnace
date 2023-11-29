#include <stdexcept>
#include <tuple>
namespace cpt100 {
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

    class Byte2 {
    public:
        // 2バイト型(0~65535)
        Byte2(int val) {
            if (val >= 0 && val <= 65535) {
                value = val;
            } else {
                throw std::out_of_range("byte2 type can store only int values from 0 to 65535");
            }
        }

        std::tuple<Byte, Byte> toByteArr() {
            return std::make_tuple(Byte(value / 256), Byte(value % 256));
        }

    private:
        int value;
    };

    class Byte3 {
    public:
        // 3バイト型(0~16777215)
        Byte3(int val) {
            if (val >= 0 && val <= 16777215) {
                value = val;
            } else {
                throw std::out_of_range("byte3 type can store only int values from 0 to 16777215");
            }
        }

        std::tuple<Byte, Byte, Byte> toByteArr() {
            return std::make_tuple(Byte(value / 65536), Byte(value / 256 % 256), Byte(value % 256));
        }

    private:
        int value;
    };

    class Bit {
    public:
        // ビット型(0~1)
        Bit(int val) {
            if (val >= 0 && val <= 1) {
                value = val;
            } else {
                throw std::out_of_range("bit type can store only int values from 0 to 1");
            }
        }

    private:
        int value;
    };

    class Bit2 {
    public:
        // 2ビット型(0~3)
        Bit2(int val) {
            if (val >= 0 && val <= 3) {
                value = val;
            } else {
                throw std::out_of_range("bit2 type can store only int values from 0 to 3");
            }
        }

    private:
        int value;
    };

    class Bit4 {
    public:
        // 4ビット型(0~15)
        Bit4(int val) {
            if (val >= 0 && val <= 15) {
                value = val;
            } else {
                throw std::out_of_range("bit4 type can store only int values from 0 to 15");
            }
        }

    private:
        int value;
    };
}