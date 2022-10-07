// Tool used to generate precomputed look up tables
#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <linearMath.h>
#include <functional>

void appendLUT(std::function<void(int ndx, std::ostream& dst)> fn, std::string type, std::string name, int domainSize, std::ostream& header, std::ostream& cpp)
{
    header << "extern const " << type << " " << name << "[];\n";
    cpp << "extern const " << type << " " << name << "[" << domainSize << "] = {\n";
    
    for (auto x = 0; x < domainSize; ++x)
    {
        cpp << "\t";
        fn(x, cpp);
        if (x < domainSize-1)
            cpp << ",";
        cpp << "\n";
    }
    cpp << "};\n";
}

void generateSinP9Lut(std::ostream& header, std::ostream& cpp)
{
    header << "// LUT table that returns the sin(x) as sNorm_16 (i.e. 1 bit for sign, 1 bit integer part, 14 bits precision.\n";
    header << "// x maps the range of [0,2*pi) to the range[0,0x1ff].\n";

    constexpr auto tableSize = 1 << 9;
    auto op = [](int x, std::ostream& dst)
    {
        double radians = double(x) / tableSize * std::numbers::pi * 2;
        auto fx = sin(radians);
        auto signX = fx < 0 ? -1 : 1;
        int quantized = signX * int(abs(fx) * (1 << 15) + 1);
        int16_t quantp14 = int16_t(quantized / 2);

        dst << quantp14;
    };

    appendLUT(op, "int16_t", "SinP9LUT", tableSize, header, cpp);
}

void generateCoTanP9Lut(std::ostream& header, std::ostream& cpp)
{
    header << "// LUT table that returns the cotan(x)=cos(x)/sin(x) as sNorm_16 (i.e. 1 bit for sign, 1 bit integer part, 14 bits precision.\n";
    header << "// Only valid for cotan(x)<=1.\n";
    header << "// x maps the range of [pi/4,3*pi/4) to the range[0,0x1ff].\n";

    constexpr auto tableSize = 1 << 9;
    auto op = [](int x, std::ostream& dst)
    {
        double radians = double(x) / tableSize * std::numbers::pi / 2 + std::numbers::pi/4;
        auto sx = sin(radians);
        auto cx = cos(radians);
        auto fx = cx/sx;
        auto signX = fx < 0 ? -1 : 1;
        int quantized = signX * int(abs(fx) * (1 << 15) + 1);
        int16_t quantp14 = int16_t(quantized / 2);

        dst << quantp14;
    };

    appendLUT(op, "int16_t", "CotanP9LUT", tableSize, header, cpp);
}

int main(int _argc, const char** _argv)
{
    // Open the dst files
    std::string fileName = "mercuryLUT";
    std::string headerName = fileName + ".h";
    std::string cppName = fileName + ".cpp";

    auto header = std::ofstream(headerName);
    auto cpp = std::ofstream(cppName);

    header << "#pragma once\n\n";
    header << "#include \"linearMath.h\"\n\n";
    cpp << "#include \"" << headerName << "\"\n\n";

    generateSinP9Lut(header, cpp);
    generateCoTanP9Lut(header, cpp);
    
    return 0;
}