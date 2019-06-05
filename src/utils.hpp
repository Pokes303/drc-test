#include <stdio.h>
#include <string>

#include <coreinit/screen.h>
#include <vpad/input.h>

void write(uint32_t row, uint32_t column, const char* format, ...);
void swrite(uint32_t row, uint32_t column, std::string format, ...);

std::string b_tostring(bool b);
std::string b_tomsg(bool b);
std::string hex_tostring(unsigned int i);
std::string hex_tostring(unsigned int i, unsigned int digits);
std::string hex_tostring0(unsigned int i, unsigned int digits);