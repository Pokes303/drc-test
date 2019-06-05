#include "utils.hpp"

void write(uint32_t row, uint32_t column, const char* format, ...) { //const char* write
	OSScreenPutFontEx(SCREEN_TV, row, column, format);
	OSScreenPutFontEx(SCREEN_DRC, row, column, format);
}

void swrite(uint32_t row, uint32_t column, std::string format, ...) { //string write
	OSScreenPutFontEx(SCREEN_TV, row, column, format.c_str());
	OSScreenPutFontEx(SCREEN_DRC, row, column, format.c_str());
}

std::string b_tostring(bool b) {
	return (b) ? "YES" : "NO";
}

std::string b_tomsg(bool b) {
	return (b) ? "Activated" : "Deactivated";
}

std::string hex_tostring(unsigned int i) {
	std::string result;
	while (true) {
		int div = i / 16;
		int remainder = i % 16;
		std::string wRemainder = std::to_string(remainder);
		if (remainder > 9) {
			switch (remainder) {
			case 10:
				wRemainder = "A";
				break;
			case 11:
				wRemainder = "B";
				break;
			case 12:
				wRemainder = "C";
				break;
			case 13:
				wRemainder = "D";
				break;
			case 14:
				wRemainder = "E";
				break;
			case 15:
				wRemainder = "F";
				break;
			}
		}
		result = wRemainder + result;

		if (div != 0)
			i = div;
		else
			break;
	}
	result = "0x" + result;
	return result;
}
std::string hex_tostring(unsigned int i, unsigned int digits) {
	std::string result;
	while (true) {
		int div = i / 16;
		int remainder = i % 16;
		std::string wRemainder = std::to_string(remainder);
		if (remainder > 9) {
			switch (remainder) {
			case 10:
				wRemainder = "A";
				break;
			case 11:
				wRemainder = "B";
				break;
			case 12:
				wRemainder = "C";
				break;
			case 13:
				wRemainder = "D";
				break;
			case 14:
				wRemainder = "E";
				break;
			case 15:
				wRemainder = "F";
				break;
			}
		}
		result = wRemainder + result;

		if (div != 0)
			i = div;
		else
			break;
	}
	if (result.size() > digits)
		return std::string("too few digits error");
	for (int i = digits - result.size(); i > 0; i--)
		result = "0" + result;
	result = "0x" + result;
	return result;
	//Thanks to "https://www.permadi.com/tutorial/numDecToHex/"
}
std::string hex_tostring0(unsigned int i, unsigned int digits) {
	std::string result;
	while (true) {
		int div = i / 16;
		int remainder = i % 16;
		std::string wRemainder = std::to_string(remainder);
		if (remainder > 9) {
			switch (remainder) {
			case 10:
				wRemainder = "A";
				break;
			case 11:
				wRemainder = "B";
				break;
			case 12:
				wRemainder = "C";
				break;
			case 13:
				wRemainder = "D";
				break;
			case 14:
				wRemainder = "E";
				break;
			case 15:
				wRemainder = "F";
				break;
			}
		}
		result = wRemainder + result;

		if (div != 0)
			i = div;
		else
			break;
	}
	if (result.size() > digits)
		return std::string("too few digits error");
	for (int i = digits - result.size(); i > 0; i--)
		result = "0" + result;
	return result;
}