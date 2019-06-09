#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdio.h> //sprintf()
#include <string> //std::string
#include <vector> //std::vector

#include <coreinit/screen.h>
#include <coreinit/cache.h>
#include <coreinit/dynload.h>
#include <coreinit/title.h>
#include <vpad/input.h>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>

#include "draw.hpp"
#include "utils.hpp"

size_t tvBufferSize;
size_t drcBufferSize;

void* tvBuffer;
void* drcBuffer;

VPADStatus vpad;
VPADReadError vError;

//65 MAX LEFT CHAR
//17 MAX DOWN CHAR

uint8_t menu = 0;

#define OPTIONS_MAX 13
uint8_t menuIndex = 0;
static const char* indexOptions[]{
	"Buttons and Joysticks",
	"Screen/Touchscreen",
	"Gyroscope",
	"Accelerometer",
	"Direction",
	"Angle",
	"Magnetometer",
	"Volume",
	"Battery",
	"Vibration",
	//NFC/Amiibo ? [nfc.rpl] / [ntag.rpl]
	//Camera [camera.rpl]
	"Gamepad sensor bar",
	//Infrared ? [dc.rpl]
	"Gamepad BASE (Unused/Unknown)",
	"Credits"
};

void endRefresh() {
	if (menu > 1 && menu != OPTIONS_MAX) //no main, no buttons and no exit
		write(48, 17, "Press B to return");

	DCFlushRange(tvBuffer, tvBufferSize);
	DCFlushRange(drcBuffer, drcBufferSize);

	OSScreenFlipBuffersEx(SCREEN_TV);
	OSScreenFlipBuffersEx(SCREEN_DRC);
}
bool startCleanRefresh() {
	VPADRead(VPAD_CHAN_0, &vpad, 1, &vError);

	switch (vError) {
	case VPAD_READ_SUCCESS:
		write(15, 1, "Success!");
		return true;
	case VPAD_READ_NO_SAMPLES:
		write(15, 1, "Waiting for samples...");
		return false;
	case VPAD_READ_INVALID_CONTROLLER:
		write(15, 1, "Invalid controller");
		return false;
	default:
		swrite(15, 1, std::string("Unknown error: ") + hex_tostring(vError, 8));
		return false;
	}
}
bool startRefresh() {
	OSScreenClearBufferEx(SCREEN_TV, 0x00000000);
	OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);

	write(0, 0, "drc-test by Pokes303");
	write(0, 1, "Checking drc...");
	if (menu > 0)
		swrite(0, 2, std::string("->") + indexOptions[menu - 1] + std::string(" info"));

	return startCleanRefresh();
}

bool sameFrame = false;
bool checkReturn() {
	if (vpad.trigger & VPAD_BUTTON_B && !sameFrame) {
		sameFrame = true;
		return true;
	}
	sameFrame = false;
	return false;
}

bool menuCredits() {
	while(WHBProcIsRunning()) {
		if (!startRefresh())
			continue;
		
		write(0, 4, "drc-test homebrew/app made by Pokes303");
		write(0, 5, "See the github repo: <https://github.com/Pokes303/drc-test>");
	
		write(0, 7, "Thanks to WiiUBrew.org by all of the info of the Dynamic Libs");
	
		write(0, 9, "Version 1.0");
	
		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

OSDynLoad_Module base_handle;
static const char* baseOptions[]{
	"Change CHANNEL param",
	"char VPADBASEIsInit()",
	"int  VPADBASEGetState(int channel)",
	"void VPADBASEGetCalibrationData(char buffer[0x44], int channel)",
	"void VPADBASEGetGameControllerMode(int channel, int* mode)",
	"void VPADBASESetGameControllerMode(int channel, int mode)",
	"void VPADBASEGetFactorySetting(char buffer[0x1C], int channel)",
	"void VPADBASEGetVolumeOverrideSetting(int channel, char* enabled, char* volume)",
	"void VPADBASESetVolumeOverrideSetting(int channel, char enabled, char volume)",
	"void VPADBASEInitVolumeOverrideSettingSyncTime(int channel)",
	"char VPADBASEGetVolumeOverrideStatus(int channel)",
	"int  VPADBASEGetHeadphoneStatus(int channel)",
	"See WARNING"
};
void writeDrcBaseInfo(uint8_t _index, uint32_t _channel) {
	swrite(0, 3, std::string("--->Channel: ") + hex_tostring(_channel, 8));
	swrite(0, 4, std::string("--->") + std::string(baseOptions[_index]));
}
bool menuDrcBase() {
	
	OSDynLoad_Acquire("vpadbase.rpl", &base_handle);
	void* handle;
	
	char (*VPADBASEIsInit)();
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEIsInit", &handle);
	VPADBASEIsInit = (char(*)())handle;
	
	int (*VPADBASEGetState)(int channel);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetState", &handle);
	VPADBASEGetState = (int(*)(int channel))handle;
	
	void (*VPADBASEGetCalibrationData)(char buffer[0x44], int channel);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetCalibrationData", &handle);
	VPADBASEGetCalibrationData = (void(*)(char buffer[0x44], int channel))handle;
	
	void (*VPADBASEGetGameControllerMode)(int channel, uint32_t* mode);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetGameControllerMode", &handle);
	VPADBASEGetGameControllerMode = (void(*)(int channel, uint32_t* mode))handle;
	
	void (*VPADBASESetGameControllerMode)(int channel, int mode);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASESetGameControllerMode", &handle);
	VPADBASESetGameControllerMode = (void(*)(int channel, int mode))handle;
	
	void (*VPADBASEGetFactorySetting)(char buffer[0x1C], int channel);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetFactorySetting", &handle);
	VPADBASEGetFactorySetting = (void(*)(char buffer[0x1C], int channel))handle;
	
	void (*VPADBASEGetVolumeOverrideSetting)(int channel, char* enabled, char* volume);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetVolumeOverrideSetting", &handle);
	VPADBASEGetVolumeOverrideSetting = (void(*)(int channel, char* enabled, char* volume))handle;
	
	void (*VPADBASESetVolumeOverrideSetting)(int channel, char enabled, char volume);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASESetVolumeOverrideSetting", &handle);
	VPADBASESetVolumeOverrideSetting = (void(*)(int channel, char enabled, char volume))handle;
	
	void (*VPADBASEInitVolumeOverrideSettingSyncTime)(int channel);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEInitVolumeOverrideSettingSyncTime", &handle);
	VPADBASEInitVolumeOverrideSettingSyncTime = (void(*)(int channel))handle;
	
	char (*VPADBASEGetVolumeOverrideStatus)(int channel);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetVolumeOverrideStatus", &handle);
	VPADBASEGetVolumeOverrideStatus = (char(*)(int channel))handle;
	
	int (*VPADBASEGetHeadphoneStatus)(int channel);
	OSDynLoad_FindExport(base_handle, 0, "VPADBASEGetHeadphoneStatus", &handle);
	VPADBASEGetHeadphoneStatus = (int(*)(int channel))handle;
	
	//Thanks to https://wiiubrew.org/wiki/Vpadbase.rpl
	
	uint8_t baseIndex = 0;
	uint32_t channel = 0;
	
	warningDrcMenu:
	while(WHBProcIsRunning()) {
		if (!startRefresh())
			continue;
		
		write(0, 4, "-= WARNING! =-");
		write(0, 5, "You are going to get/set some unknown values from GAMEPAD BASE,");
		write(0, 6, "which has not be used never officially.");
		write(0, 8, "There is not suposed to be any problem like bricking, but be");
		write(0, 9, "careful. Use it at your own risk!");
		write(0, 11, "Press A to continue");
		
		if (vpad.trigger & VPAD_BUTTON_A)
			goto mainDrcMenu;
	
		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
	
	mainDrcMenu:
	while(WHBProcIsRunning()) {
		if (!startRefresh())
			continue;
		
		for (int i = 0; i < 12; i++) {
			if (i == baseIndex)
				write(0, i + 4, ">");
			write(1, i + 4, baseOptions[i]);
		}
		swrite(22, 4, std::string("(Current: ") + hex_tostring(channel, 8) + std::string(")"));
		
		write(0, 17, "Press Y to see the warning");
		
		switch (vpad.trigger) {
		case VPAD_BUTTON_A: {
			switch (baseIndex) {
				case 0: { //CHANGE CHANNEL PARAM
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						swrite(0, 3, std::string("--->") + baseOptions[0]);
							
						write(0, 5, "Change number fast using L Stick LEFT and RIGHT");
						write(0, 6, "Change one-by-one number using LEFT and RIGHT on DPAD");
						
						swrite(0, 8, std::string(">Channel: ") + std::to_string(channel));
						
						if ((vpad.trigger & VPAD_BUTTON_RIGHT || vpad.hold & VPAD_STICK_L_EMULATION_RIGHT))
							channel++;
						else if ((vpad.trigger & VPAD_BUTTON_LEFT || vpad.hold & VPAD_STICK_L_EMULATION_LEFT))
							channel--;
							
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 1: { //char VPADBASEIsInit()
					char result = 99;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						result = VPADBASEIsInit();
						swrite(0, 6, std::string("Result: ") + hex_tostring(result, 2));
						
						write(0, 8, "0: VPAD BASE Dynamic lib UNLOADED");
						write(0, 9, "1: VPAD BASE Dynamic lib LOADED");
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 2: { //int VPADBASEGetState(int channel)
					int result = 999;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						result = VPADBASEGetState(channel);
						swrite(0, 6, std::string("Result: ") + hex_tostring(result, 8));
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 3: { //void VPADBASEGetCalibrationData(char buffer[0x44], int channel)
					char buffer[0x44];
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						VPADBASEGetCalibrationData(buffer, channel);
						
						for (char y = 0; y < 0x05; y++) {
							for (char x = 0; x < 0x10; x++) {
								if (y * 16 + x >= 0x44)
									break;
								swrite(0 + x * 4, 6 + y, hex_tostring0(buffer[y * 16 + x], 2));
							}
						}
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 4: { //void VPADBASEGetGameControllerMode(int channel, int* mode)
					uint32_t mode = 999;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						VPADBASEGetGameControllerMode(channel, &mode);
						
						swrite(0, 6, std::string("Mode: ") + hex_tostring(mode, 8));
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 5: { //void VPADBASESetGameControllerMode(int channel, int mode)
					uint32_t mode = 0;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						write(0, 6, "Change number fast using L Stick LEFT and RIGHT");
						write(0, 7, "Change one-by-one number using LEFT and RIGHT on DPAD");
						
						swrite(0, 9, std::string(">Mode: ") + hex_tostring(mode, 8));
						
						if ((vpad.trigger & VPAD_BUTTON_RIGHT || vpad.hold & VPAD_STICK_L_EMULATION_RIGHT))
							mode++;
						else if ((vpad.trigger & VPAD_BUTTON_LEFT || vpad.hold & VPAD_STICK_L_EMULATION_LEFT))
							mode--;
						
						write(0, 11, "Press A to set value");
						
						if (vpad.trigger & VPAD_BUTTON_A)
							VPADBASESetGameControllerMode(channel, mode);
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 6: { //void VPADBASEGetFactorySetting(char buffer[0x1C], int channel)
					char buffer[0x1C];
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						VPADBASEGetFactorySetting(buffer, channel);
						
						for (char y = 0; y < 0x02; y++) {
							for (char x = 0; x < 0x10; x++) {
								if (y * 16 + x >= 0x1C)
									break;
								swrite(0 + x * 4, 6 + y, hex_tostring0(buffer[y * 16 + x], 2));
							}
						}
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 7: { //void VPADBASEGetVolumeOverrideSetting(int channel, char* enabled, char* volume)
					char enabled = 0;
					char volume = 0;
					
					bool option1 = false;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						VPADBASEGetVolumeOverrideSetting(channel, &enabled, &volume);
						
						swrite(1, 6, std::string("Enabled: ") + hex_tostring(enabled, 2));
						swrite(1, 7, std::string("Volume: ") + hex_tostring(volume, 2));
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 8: { //void VPADBASESetVolumeOverrideSetting(int channel, char enabled, char volume)
					char enabled = 0;
					char volume = 0;
					
					bool option2 = false;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						write(0, 6, "Change number fast using L Stick LEFT and RIGHT");
						write(0, 7, "Change one-by-one number using LEFT and RIGHT on DPAD");
						write(0, 8, "Move cursor between the numbers using UP and DOWN on DPAD");
						
						swrite(1, 10, std::string("Enabled: ") + hex_tostring(enabled, 2));
						swrite(1, 11, std::string("Volume: ") + hex_tostring(volume, 2));
						
						if ((vpad.trigger & VPAD_BUTTON_RIGHT || vpad.hold & VPAD_STICK_L_EMULATION_RIGHT))
							!option2 ? volume++ : enabled++;
						else if ((vpad.trigger & VPAD_BUTTON_LEFT || vpad.hold & VPAD_STICK_L_EMULATION_LEFT))
							!option2 ? volume-- : enabled-- ;
						
						write(0, 10 + option2, ">");
						
						if (vpad.trigger & VPAD_BUTTON_UP || vpad.trigger & VPAD_BUTTON_DOWN
						|| vpad.trigger & VPAD_STICK_L_EMULATION_UP || vpad.trigger & VPAD_STICK_L_EMULATION_DOWN)
							option2 = !option2;
						
						write(0, 13, "Press A to set value");
						
						if (vpad.trigger & VPAD_BUTTON_A)
							VPADBASESetVolumeOverrideSetting(channel, enabled, volume);
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 9: { //void VPADBASEInitVolumeOverrideSettingSyncTime(int channel)
					VPADBASEInitVolumeOverrideSettingSyncTime(channel);
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						write(0, 6, "VPAD BASE Volume override setting sync time initiated successfully");
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 10: { //char VPADBASEGetVolumeOverrideStatus(int channel)
					char result = 99;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						result = VPADBASEGetVolumeOverrideStatus(channel);
						swrite(0, 6, std::string("Result: ") + hex_tostring(result, 2));
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
				case 11: { //int VPADBASEGetHeadphoneStatus(int channel)
					int result = 999;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						writeDrcBaseInfo(baseIndex, channel);
						
						result = VPADBASEGetHeadphoneStatus(channel);
						swrite(0, 6, std::string("Result: ") + hex_tostring(result, 2));
						
						write(0, 8, "0: VPAD BASE Headphones DISCONNECTED");
						write(0, 9, "1: VPAD BASE Headphones CONNECTED");
						
						if (checkReturn())
							goto loopMenuBase;

						endRefresh();
					}
					return false;
				}
			}
			break;
		}
		case VPAD_BUTTON_Y:
			goto warningDrcMenu;
			break;
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			if (baseIndex > 0)
				baseIndex--;
			else
				baseIndex = 11;
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			if (baseIndex < 11)
				baseIndex++;
			else
				baseIndex = 0;
			break;
		}
		
		if (checkReturn())
			return true;

		loopMenuBase:
		endRefresh();
	}
	return false;
}

bool menuSensorBar() {
	bool off = false;
	
	int result = 999;
	while(WHBProcIsRunning()) {
		if (!startRefresh())
			continue;
		
		write(1, 4, "Set sensor bar ON (This can be seen with a camera)");
		write(1, 5, "Set sensor bar OFF");
		write(0, 4 + off, ">");
		
		if (result != 999)
			swrite(1, 7, std::string("Result: ") + std::to_string(result));
		
		switch (vpad.trigger) {
		case VPAD_BUTTON_A:
			result = VPADSetSensorBar (VPAD_CHAN_0, !off);
			break;
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			off = !off;
			break;
		}
			
		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

static const char* vibrationOptions[]{
	"Edit motor pattern",
	"Edit motor lenght",
	"Start motor with provided params",
	"Start motor with example params",
	"Stop motor"
};
bool menuVibration() {
	uint8_t vibrationIndex = 0;
	
	uint32_t motorResult = 99;
	std::vector<uint8_t> pattern;
	pattern.push_back(0xFF);
	uint32_t lenght = 0;
	
	int result = 999;
	while(WHBProcIsRunning()) {
		if (!startRefresh())
			continue;
		
		write(0, 4, "Current params: (They stay until you return to main menu)");
		swrite(2, 5, std::string("+Pattern (Array size): ") + hex_tostring(pattern.size(), 8));
		swrite(2, 6, std::string("+Length: ") + std::to_string(lenght));
							
		for (int i = 0; i < 5; i++) {
			if (i == vibrationIndex)
				write(0, i + 8, ">");
			write(1, i + 8, vibrationOptions[i]);
		}
		
		if (result != 999)
			swrite(0, 14, std::string("Result: ") + std::to_string(result));
		
		switch (vpad.trigger) {
		case VPAD_BUTTON_A:
			switch (vibrationIndex) {
				case 0: { //HEX EDIT
					uint32_t page = 0;
					uint32_t pages = 0;
					pages = (pattern.size() - 1) / 16;
				
					uint8_t patternIndex = 0;
					
					bool initialF = true;
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						swrite(0, 3, std::string("--->") + vibrationOptions[0]);
							
						swrite(57 - std::to_string(page).size() - std::to_string(pages).size(), 0, "Page " + std::to_string(page + 1) + "/" + std::to_string(pages + 1));
						if (pages == 0);
						else if (page <= 0) {
							write(55, 1, "Press R ->");
							write(54, 2, "Press ZR >>");
						}
						else if (page >= pages) {
							write(55, 1, "<- Press L");
							write(54, 2, "<< Press ZL");
						}
						else {
							write(50, 1, "<- Press L/R ->");
							write(48, 2, "<< Press ZL/ZR >>");
						}
						write(43, 3, "Press X to clear array");
		
						for (int a = 0; a < 0x10; a++)
							swrite(a * 4, 5, hex_tostring(a, 1));
						
						for (int a = 0; a < 0x10; a++) {
							if (a == patternIndex)
								write(a * 4, 7, ">");
							std::string result;
							if (page * 16 + a < pattern.size()) {
								switch(pattern[page * 16 + a]) {
								case 0x0:
									result = "00";
									break;
								case 0xFF:
									result = "FF";
									break;
								default:
									result = "??";
									break;
								}
							}
							else
								result = "--";
							swrite(1 + a * 4, 7, result);
						}
					
						write(0, 8, "_______________________________________________________________");
						swrite(0, 10, std::string("Pattern array size: ") + hex_tostring(pattern.size()) + std::string("   (Max: ") + hex_tostring(pattern.max_size()) + std::string(")"));
						
						write(0, 12, "Press A to modify selected element (Between 0x00 and 0xFF)");
						write(0, 13, "Press DPAD to move around elements");
						swrite(0, 14, std::string("Press Y to switch initial value (Current: ") + ((initialF) ? std::string("0xFF") : std::string("0x00")) + std::string(")"));
						
						if (pattern.size() < pattern.max_size())
							write(0, 16, "Press START to add new element at end");
						if (pattern.size() > 1)
							write(0, 17, "Press SELECT to delete element at end");
						
						switch(vpad.trigger) {
						case VPAD_BUTTON_A:
							if (page * 16 + patternIndex > pattern.size())
								break;
							
							pattern[page * 16 + patternIndex] = 255 - pattern[page * 16 + patternIndex];
							break;
						case VPAD_BUTTON_X:
							pattern.clear();
							pattern.push_back((initialF) ? 0xFF : 0x00);
							break;
						case VPAD_BUTTON_Y:
							initialF = !initialF;
							break;
						case VPAD_BUTTON_PLUS:
							if (pattern.size() < pattern.max_size())
								pattern.push_back((initialF) ? 0xFF : 0x00);
							pages = (pattern.size() - 1) / 16;
							break;
						case VPAD_BUTTON_MINUS:
							if (pattern.size() > 1) {
								pattern.pop_back();
								pages = (pattern.size() - 1) / 16;
								if (page > pages)
									page = pages;
							}
							break;
						case VPAD_BUTTON_RIGHT:
						case VPAD_STICK_L_EMULATION_RIGHT:
							if (patternIndex < 0xF)
								patternIndex++;
							else
								patternIndex = 0;
							break;
						case VPAD_BUTTON_LEFT:
						case VPAD_STICK_L_EMULATION_LEFT:
							if (patternIndex > 0x0)
								patternIndex--;
							else
								patternIndex = 0xF;
							break;
						case VPAD_BUTTON_R:
							if (page < pages)
								page++;
							break;
						case VPAD_BUTTON_ZR:
							if (page < pages)
								page = pages;
							break;
						case VPAD_BUTTON_L:
							if (page > 0)
								page--;
							break;
						case VPAD_BUTTON_ZL:
							if (page > 0)
								page = 0;
							break;
						}
						
						if (checkReturn())
							goto loopMenuVibration;

						endRefresh();
					}
					return false;
				}
				case 1: //MOTOR LENGHT EDIT
					while(WHBProcIsRunning()) {
						if (!startRefresh())
							continue;
						
						swrite(0, 3, std::string("--->") + vibrationOptions[1]);
						
						write(0, 5, "Change number fast using L Stick LEFT and RIGHT");
						write(0, 6, "Change one-by-one number using LEFT and RIGHT on DPAD");
						
						swrite(0, 8, std::string(">Lenght: ") + std::to_string(lenght));
						swrite(15, 8, std::string("(") + hex_tostring(lenght, 2) + std::string(")"));
						
						swrite(0, 10, std::string("Pattern array size: ") + hex_tostring(pattern.size(), 8));
						
						if ((vpad.trigger & VPAD_BUTTON_RIGHT || vpad.hold & VPAD_STICK_L_EMULATION_RIGHT))
							lenght++;
						else if ((vpad.trigger & VPAD_BUTTON_LEFT || vpad.hold & VPAD_STICK_L_EMULATION_LEFT))
							lenght--;
						
						if (checkReturn())
							goto loopMenuVibration;

						endRefresh();
					}
					return false;
					break;
				case 2: //START MOTOR W/CUSTOM
					result = VPADControlMotor(VPAD_CHAN_0, pattern.data(), lenght);
					break;
				case 3: { //START MOTOR W/EXAMPLE
					VPADStopMotor(VPAD_CHAN_0);
					uint8_t examplePattern[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
					result = VPADControlMotor(VPAD_CHAN_0, examplePattern, 0x10);
					break;
				}
				case 4: //STOP MOTOR
					VPADStopMotor(VPAD_CHAN_0);
					result = 999;
					break;
			}
			break;
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			if (vibrationIndex > 0)
				vibrationIndex--;
			else
				vibrationIndex = 4;
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			if (vibrationIndex < 4)
				vibrationIndex++;
			else
				vibrationIndex = 0;
			break;
		}
		
		if (checkReturn())
			return true;

		loopMenuVibration:
		endRefresh();
	}
	return false;
}

const char* batteryStatus[]{
	"Charging",
	"Unknown", //Low, maybe?
	"Very low",
	"Low",
	"Medium",
	"High",
	"Full"
};
bool menuBattery() {
	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		write(0, 4, "Current battery status: ");
		swrite(24, 4, std::string(std::string("(") + std::to_string(vpad.battery) + std::string(") ") + batteryStatus[vpad.battery]));

		write(0, 6, "Battery statuses:");
		for (int i = 6; i >= 0; i--)
			swrite(1, i + 7, std::string("(") + std::to_string(i) + std::string(") ") + std::string(batteryStatus[i]));

		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

bool menuAudio() {
	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		swrite(0, 4, std::string("Headphones plugged: ") + b_tostring(vpad.usingHeadphones));
		swrite(0, 5, std::string("Slide volume value: ") + hex_tostring(vpad.slideVolume, 2));
		swrite(0, 6, std::string("Calibrated slide volume value: ") + hex_tostring(vpad.slideVolumeEx, 2));
		swrite(0, 7, std::string("Mic status: ") + std::to_string(vpad.micStatus));

		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

bool menuDir() { //Direction
	uint8_t dirMenu = 0;

	std::vector<VPADVec3D> dirA;
	std::vector<VPADVec3D> dirB;
	std::vector<VPADVec3D> dirC;

	float maxA = 1;
	float maxB = 1;
	float maxC = 1;

	VPADVec3D maxDirA { 0, 0, 0 };
	VPADVec3D maxDirB { 0, 0, 0 };
	VPADVec3D maxDirC { 0, 0, 0 };

	bool xA = true; //Active axis X
	bool yA = true;
	bool zA = true;

	bool xB = true; //Active axis Y
	bool yB = true;
	bool zB = true;

	bool xC = true; //Active axis Z
	bool yC = true;
	bool zC = true;

	uint8_t tIndexA = 0; //Toggle index A
	uint8_t tIndexB = 0; //Toggle index B
	uint8_t tIndexC = 0; //Toggle index C

	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		///A

		if (dirA.size() >= 851)
			dirA.erase(dirA.begin());

		dirA.push_back(vpad.direction.x);

		//ABSOLUTE MAX
		if (std::abs(vpad.direction.x.x) > maxA)
			maxA = std::abs(vpad.direction.x.x);
		if (std::abs(vpad.direction.x.y) > maxA)
			maxA = std::abs(vpad.direction.x.y);
		if (std::abs(vpad.direction.x.z) > maxA)
			maxA = std::abs(vpad.direction.x.z);

		//MAX PER AXIS
		if (std::abs(vpad.direction.x.x) > std::abs(maxDirA.x))
			maxDirA.x = vpad.direction.x.x;
		if (std::abs(vpad.direction.x.y) > std::abs(maxDirA.y))
			maxDirA.y = vpad.direction.x.y;
		if (std::abs(vpad.direction.x.z) > std::abs(maxDirA.z))
			maxDirA.z = vpad.direction.x.z;

		///B

		if (dirB.size() >= 851)
			dirB.erase(dirB.begin());

		dirB.push_back(vpad.direction.y);

		//ABSOLUTE MAX
		if (std::abs(vpad.direction.y.x) > maxB)
			maxB = std::abs(vpad.direction.y.x);
		if (std::abs(vpad.direction.y.y) > maxB)
			maxB = std::abs(vpad.direction.y.y);
		if (std::abs(vpad.direction.y.z) > maxB)
			maxB = std::abs(vpad.direction.y.z);

		//MAX PER AXIS
		if (std::abs(vpad.direction.y.x) > std::abs(maxDirB.x))
			maxDirB.x = vpad.direction.y.x;
		if (std::abs(vpad.direction.y.y) > std::abs(maxDirB.y))
			maxDirB.y = vpad.direction.y.y;
		if (std::abs(vpad.direction.y.z) > std::abs(maxDirB.z))
			maxDirB.z = vpad.direction.y.z;

		///C

		if (dirC.size() >= 851)
			dirC.erase(dirC.begin());

		dirC.push_back(vpad.direction.z);

		//ABSOLUTE MAX
		if (std::abs(vpad.direction.z.x) > maxC)
			maxC = std::abs(vpad.direction.y.x);
		if (std::abs(vpad.direction.z.y) > maxC)
			maxC = std::abs(vpad.direction.y.y);
		if (std::abs(vpad.direction.z.z) > maxC)
			maxC = std::abs(vpad.direction.z.z);

		//MAX PER AXIS
		if (std::abs(vpad.direction.z.x) > std::abs(maxDirC.x))
			maxDirC.x = vpad.direction.z.x;
		if (std::abs(vpad.direction.z.y) > std::abs(maxDirC.y))
			maxDirC.y = vpad.direction.z.y;
		if (std::abs(vpad.direction.z.z) > std::abs(maxDirC.z))
			maxDirC.z = vpad.direction.z.z;

		write(28, 4, "r");
		write(49, 4, "Press Y to clear");

		switch (dirMenu) {
		case 0:
			write(62, 2, "(X)");
			write(57, 0, "Page 1/3");
			write(55, 1, "Press R ->");

			swrite(0, 4, std::string("[") + std::to_string(maxA) + std::string("]"));

			swrite(0, 14, std::string("X: ") + std::to_string(vpad.direction.x.x));
			swrite(0, 15, std::string("Y: ") + std::to_string(vpad.direction.x.y));
			swrite(0, 16, std::string("Z: ") + std::to_string(vpad.direction.x.z));

			swrite(16, 14, std::string("[X]: ") + std::to_string(maxDirA.x));
			swrite(16, 15, std::string("[Y]: ") + std::to_string(maxDirA.y));
			swrite(16, 16, std::string("[Z]: ") + std::to_string(maxDirA.z));

			write(33, tIndexA + 14, ">");

			swrite(34, 14, std::string("Toggle X [") + b_tomsg(xA) + std::string("]"));
			swrite(34, 15, std::string("Toggle Y [") + b_tomsg(yA) + std::string("]"));
			swrite(34, 16, std::string("Toggle Z [") + b_tomsg(zA) + std::string("]"));

			drawV3DGraphic(160, 200, dirA, maxA, xA, yA, zA);
			break;
		case 1:
			write(62, 2, "(Y)");
			write(57, 0, "Page 2/3");
			write(50, 1, "<- Press L/R ->");

			swrite(0, 4, std::string("[") + std::to_string(maxB) + std::string("]"));

			swrite(0, 14, std::string("X: ") + std::to_string(vpad.direction.y.x));
			swrite(0, 15, std::string("Y: ") + std::to_string(vpad.direction.y.y));
			swrite(0, 16, std::string("Z: ") + std::to_string(vpad.direction.y.z));

			swrite(16, 14, std::string("[X]: ") + std::to_string(maxDirB.x));
			swrite(16, 15, std::string("[Y]: ") + std::to_string(maxDirB.y));
			swrite(16, 16, std::string("[Z]: ") + std::to_string(maxDirB.z));

			write(33, tIndexB + 14, ">");

			swrite(34, 14, std::string("Toggle X [") + b_tomsg(xB) + std::string("]"));
			swrite(34, 15, std::string("Toggle Y [") + b_tomsg(yB) + std::string("]"));
			swrite(34, 16, std::string("Toggle Z [") + b_tomsg(zB) + std::string("]"));

			drawV3DGraphic(160, 200, dirB, maxB, xB, yB, zB);
			break;
		case 2:
				write(62, 2, "(Z)");
				write(57, 0, "Page 3/3");
				write(55, 1, "<- Press L");

				swrite(0, 4, std::string("[") + std::to_string(maxC) + std::string("]"));

				swrite(0, 14, std::string("X: ") + std::to_string(vpad.direction.z.x));
				swrite(0, 15, std::string("Y: ") + std::to_string(vpad.direction.z.y));
				swrite(0, 16, std::string("Z: ") + std::to_string(vpad.direction.z.z));

				swrite(16, 14, std::string("[X]: ") + std::to_string(maxDirC.x));
				swrite(16, 15, std::string("[Y]: ") + std::to_string(maxDirC.y));
				swrite(16, 16, std::string("[Z]: ") + std::to_string(maxDirC.z));

				write(33, tIndexC + 14, ">");

				swrite(34, 14, std::string("Toggle X [") + b_tomsg(xC) + std::string("]"));
				swrite(34, 15, std::string("Toggle Y [") + b_tomsg(yC) + std::string("]"));
				swrite(34, 16, std::string("Toggle Z [") + b_tomsg(zC) + std::string("]"));

				drawV3DGraphic(160, 200, dirC, maxC, xC, yC, zC);
				break;
		}

		switch (vpad.trigger) {
		case VPAD_BUTTON_A: {
			switch (dirMenu) {
			case 0:
				switch (tIndexA) {
				case 0:
					if (!xA || (xA && (yA || zA)))
						xA = !xA;
					break;
				case 1:
					if (!yA || (yA && (xA || zA)))
						yA = !yA;
					break;
				case 2:
					if (!zA || (zA && (xA || yA)))
						zA = !zA;
					break;
				}
				break;
			case 1:
				switch (tIndexB) {
				case 0:
					if (!xB || (xB && (yB || zB)))
						xB = !xB;
					break;
				case 1:
					if (!yB || (yB && (xB || zB)))
						yB = !yB;
					break;
				case 2:
					if (!zB || (zB && (xB || yB)))
						zB = !zB;
					break;
				}
				break;
			case 2:
				switch (tIndexC) {
				case 0:
					if (!xC || (xC && (yC || zC)))
						xC = !xC;
					break;
				case 1:
					if (!yC || (yC && (xC || zC)))
						yC = !yC;
					break;
				case 2:
					if (!zC || (zC && (xC || yC)))
						zC = !zC;
					break;
				}
				break;
			}
			break;
		}
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			switch (dirMenu) {
			case 0:
				if (tIndexA > 0)
					tIndexA--;
				else
					tIndexA = 2;
				break;
			case 1:
				if (tIndexB > 0)
					tIndexB--;
				else
					tIndexB = 2;
				break;
			case 2:
				if (tIndexC > 0)
					tIndexC--;
				else
					tIndexC = 2;
				break;
			}
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			switch (dirMenu) {
			case 0:
				if (tIndexA < 2)
					tIndexA++;
				else
					tIndexA = 0;
				break;
			case 1:
				if (tIndexB < 2)
					tIndexB++;
				else
					tIndexB = 0;
				break;
			case 2:
				if (tIndexC < 2)
					tIndexC++;
				else
					tIndexC = 0;
				break;
			}
			break;
		case VPAD_BUTTON_Y:
			dirA.clear();
			dirB.clear();
			dirC.clear();

			maxA = 1;
			maxB = 1;
			maxC = 1;

			maxDirA = { 0, 0, 0 };
			maxDirB = { 0, 0, 0 };
			maxDirC = { 0, 0, 0 };
			break;
		case VPAD_BUTTON_L:
			if (dirMenu > 0)
				dirMenu--;
			break;
		case VPAD_BUTTON_R:
			if (dirMenu < 2)
				dirMenu++;
			break;
		}

		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

bool menuAcc() { //Accelerometer
	bool accMenu = true;

	std::vector<VPADVec3D> acc;
	std::vector<VPADVec2D> vert;

	float max = 1;
	float maxV = 1;

	VPADVec3D maxAcc{ 0, 0, 0 };
	VPADVec2D maxVert{ 0, 0 };
	float maxMag = 0; //Max magnitude
	float maxVar = 0; //Max variation

	bool xA = true; //Active axis
	bool yA = true;
	bool zA = true;

	bool xB = true; //Active axis
	bool yB = true;

	uint8_t tIndex = 0; //Toggle index
	uint8_t tVertIndex = 0; //Toggle index

	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		///ACC

		if (acc.size() >= 851)
			acc.erase(acc.begin());

		acc.push_back(vpad.accelorometer.acc);

		//ABSOLUTE MAX
		if (std::abs(vpad.accelorometer.acc.x) > max)
			max = std::abs(vpad.accelorometer.acc.x);
		if (std::abs(vpad.accelorometer.acc.y) > max)
			max = std::abs(vpad.accelorometer.acc.y);
		if (std::abs(vpad.accelorometer.acc.z) > max)
			max = std::abs(vpad.accelorometer.acc.z);

		//MAX PER AXIS
		if (std::abs(vpad.accelorometer.acc.x) > std::abs(maxAcc.x))
			maxAcc.x = vpad.accelorometer.acc.x;
		if (std::abs(vpad.accelorometer.acc.y) > std::abs(maxAcc.y))
			maxAcc.y = vpad.accelorometer.acc.y;
		if (std::abs(vpad.accelorometer.acc.z) > std::abs(maxAcc.z))
			maxAcc.z = vpad.accelorometer.acc.z;

		///VERT

		if (vert.size() >= 851)
			vert.erase(vert.begin());

		vert.push_back(vpad.accelorometer.vertical);

		//ABSOLUTE MAX VERTICAL
		if (std::abs(vpad.accelorometer.vertical.x) > maxV)
			maxV = std::abs(vpad.accelorometer.vertical.x);
		if (std::abs(vpad.accelorometer.vertical.y) > maxV)
			maxV = std::abs(vpad.accelorometer.vertical.y);

		//MAX PER AXIS VERTICAL
		if (std::abs(vpad.accelorometer.vertical.x) > std::abs(maxVert.x))
			maxVert.x = vpad.accelorometer.vertical.x;
		if (std::abs(vpad.accelorometer.vertical.y) > std::abs(maxVert.y))
			maxVert.y = vpad.accelorometer.vertical.y;

		///MAG & VAR

		if (std::abs(vpad.accelorometer.magnitude) > std::abs(maxMag))
			maxMag = vpad.accelorometer.magnitude;
		if (std::abs(vpad.accelorometer.variation) > std::abs(maxVar))
			maxVar = vpad.accelorometer.variation;

		swrite(26, 0, std::string("Magnitude: ") + std::to_string(vpad.accelorometer.magnitude));
		swrite(25, 1, std::string("[Magnitude]: ") + std::to_string(maxMag));
		swrite(26, 2, std::string("Variation: ") + std::to_string(vpad.accelorometer.variation));
		swrite(25, 3, std::string("[Variation]: ") + std::to_string(maxVar));

		write(28, 4, "m/s2");
		write(49, 4, "Press Y to clear");

		if (accMenu) {
			write(57, 0, "Page 1/2");
			write(55, 1, "Press R ->");
			write(51, 2, "(Acceleration)");

			swrite(0, 4, std::string("[") + std::to_string(max) + std::string("]"));

			swrite(0, 14, std::string("X: ") + std::to_string(vpad.accelorometer.acc.x));
			swrite(0, 15, std::string("Y: ") + std::to_string(vpad.accelorometer.acc.y));
			swrite(0, 16, std::string("Z: ") + std::to_string(vpad.accelorometer.acc.z));

			swrite(16, 14, std::string("[X]: ") + std::to_string(maxAcc.x));
			swrite(16, 15, std::string("[Y]: ") + std::to_string(maxAcc.y));
			swrite(16, 16, std::string("[Z]: ") + std::to_string(maxAcc.z));

			write(33, tIndex + 14, ">");

			swrite(34, 14, std::string("Toggle X [") + b_tomsg(xA) + std::string("]"));
			swrite(34, 15, std::string("Toggle Y [") + b_tomsg(yA) + std::string("]"));
			swrite(34, 16, std::string("Toggle Z [") + b_tomsg(zA) + std::string("]"));

			drawV3DGraphic(160, 200, acc, max, xA, yA, zA);
		}
		else {
			write(57, 0, "Page 2/2");
			write(55, 1, "<- Press L");
			write(55, 2, "(Vertical)");

			swrite(0, 4, std::string("[") + std::to_string(maxV) + std::string("]"));

			swrite(0, 14, std::string("X: ") + std::to_string(vpad.accelorometer.vertical.x));
			swrite(0, 15, std::string("Y: ") + std::to_string(vpad.accelorometer.vertical.y));

			swrite(16, 14, std::string("[X]: ") + std::to_string(maxVert.x));
			swrite(16, 15, std::string("[Y]: ") + std::to_string(maxVert.y));

			write(33, tVertIndex + 14, ">");

			swrite(34, 14, std::string("Toggle X [") + b_tomsg(xB) + std::string("]"));
			swrite(34, 15, std::string("Toggle Y [") + b_tomsg(yB) + std::string("]"));

			drawV2DGraphic(160, 200, vert, maxV, xB, yB);
		}

		switch (vpad.trigger) {
		case VPAD_BUTTON_A: {
			if (accMenu) {
				switch (tIndex) {
				case 0:
					if (!xA || (xA && (yA || zA)))
						xA = !xA;
					break;
				case 1:
					if (!yA || (yA && (xA || zA)))
						yA = !yA;
					break;
				case 2:
					if (!zA || (zA && (xA || yA)))
						zA = !zA;
					break;
				}
			}
			else {
				switch (tVertIndex) {
				case 0:
					if (!xB || (xB && yB))
						xB = !xB;
					break;
				case 1:
					if (!yB || (yB && xB))
						yB = !yB;
					break;
				}
			}
			break;
		}
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			if (accMenu) {
				if (tIndex > 0)
					tIndex--;
				else
					tIndex = 2;
			}
			else {
				if (tVertIndex > 0)
					tVertIndex--;
				else
					tVertIndex = 1;
			}
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			if (accMenu) {
				if (tIndex < 2)
					tIndex++;
				else
					tIndex = 0;
			}
			else {
				if (tVertIndex < 1)
					tVertIndex++;
				else
					tVertIndex = 0;
			}
			break;
		case VPAD_BUTTON_Y:
			acc.clear();
			vert.clear();
			max = 1;
			maxV = 1;
			maxAcc = { 0, 0, 0 };
			maxVert = { 0, 0 };
			break;
		case VPAD_BUTTON_L:
			if (!accMenu)
				accMenu = true;
			break;
		case VPAD_BUTTON_R:
			if (accMenu)
				accMenu = false;
			break;
		}

		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

bool menuGyroAngMag(uint32_t aType) { //Gyro, Angle and Magnetometer
	std::vector<VPADVec3D> axis;

	float max = 1;

	VPADVec3D maxAxis { 0, 0, 0 };

	bool xA = true; //Active axis
	bool yA = true;
	bool zA = true;

	uint8_t tIndex = 0; //Toggle index
	
	if (aType == 1)
		VPADSetGyroAngle(VPAD_CHAN_0, 0, 0, 0); //Set angle to 0

	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		if (axis.size() >= 851)
			axis.erase(axis.begin());

		VPADVec3D tAxis; //Temp axis
		switch (aType) {
		case 0:
			tAxis = vpad.gyro;
			write(28, 4, "m/s2");
			break;
		case 1:
			tAxis = vpad.angle;
			write(28, 4, "rps");
			break;
		case 2:
			tAxis = vpad.mag;
			write(28, 4, "uT");
			break;
		}

		axis.push_back(tAxis);
		//ABSOLUTE MAX
		if (std::abs(tAxis.x) > max)
			max = std::abs(tAxis.x);
		if (std::abs(tAxis.y) > max)
			max = std::abs(tAxis.y);
		if (std::abs(tAxis.z) > max)
			max = std::abs(tAxis.z);

		//MAX PER AXIS
		if (std::abs(tAxis.x) > std::abs(maxAxis.x))
			maxAxis.x = tAxis.x;
		if (std::abs(tAxis.y) > std::abs(maxAxis.y))
			maxAxis.y = tAxis.y;
		if (std::abs(tAxis.z) > std::abs(maxAxis.z))
			maxAxis.z = tAxis.z;

		if (aType != 2) {
			swrite(0, 14, std::string("X: ") + std::to_string(tAxis.x));
			swrite(0, 15, std::string("Y: ") + std::to_string(tAxis.y));
			swrite(0, 16, std::string("Z: ") + std::to_string(tAxis.z));

			swrite(16, 14, std::string("[X]: ") + std::to_string(maxAxis.x));
			swrite(16, 15, std::string("[Y]: ") + std::to_string(maxAxis.y));
			swrite(16, 16, std::string("[Z]: ") + std::to_string(maxAxis.z));
		}
		else {
			swrite(0, 14, std::string("X: ") + std::to_string((int)tAxis.x));
			swrite(0, 15, std::string("Y: ") + std::to_string((int)tAxis.y));
			swrite(0, 16, std::string("Z: ") + std::to_string((int)tAxis.z));

			swrite(16, 14, std::string("[X]: ") + std::to_string((int)maxAxis.x));
			swrite(16, 15, std::string("[Y]: ") + std::to_string((int)maxAxis.y));
			swrite(16, 16, std::string("[Z]: ") + std::to_string((int)maxAxis.z));
		}

		swrite(0, 4, std::string("[") + std::to_string(max) + std::string("]"));

		write(49, 4, "Press Y to clear");
			
		drawV3DGraphic(160, 200, axis, max, xA, yA, zA);

		switch (vpad.trigger) {
		case VPAD_BUTTON_A: {
			switch (tIndex) {
			case 0:
				if (!xA || (xA && (yA || zA)))
					xA = !xA;
				break;
			case 1:
				if (!yA || (yA && (xA || zA)))
					yA = !yA;
				break;
			case 2:
				if (!zA || (zA && (xA || yA)))
					zA = !zA;
				break;
			}
		}
			break;
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			if (tIndex > 0)
				tIndex--;
			else
				tIndex = 2;
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			if (tIndex < 2)
				tIndex++;
			else
				tIndex = 0;
			break;
		case VPAD_BUTTON_Y:
			axis.clear();
			max = 1;
			maxAxis = { 0, 0, 0 };
			if (aType == 1)
				VPADSetGyroAngle(VPAD_CHAN_0, 0, 0, 0);
			break;
		}
		write(33, tIndex + 14, ">");

		swrite(34, 14, std::string("Toggle X [") + b_tomsg(xA) + std::string("]"));
		swrite(34, 15, std::string("Toggle Y [") + b_tomsg(yA) + std::string("]"));
		swrite(34, 16, std::string("Toggle Z [") + b_tomsg(zA) + std::string("]"));

		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

uint8_t screenIndex = 0;
static const char* screenOptions[]{
	"LCD Mode",
	"Screen color pixels test",
	"Screen rainbow color pixels test",
	"Touchscreen test",
	"Touchscreen test with smooting level 1",
	"Touchscreen test with smooting level 2",
};
uint8_t subMenuRainbowSetColor(uint8_t rR, uint8_t gR, uint8_t bR) {
	
	if (!WHBProcIsRunning() || !startCleanRefresh()) {
		OSScreenClearBufferEx(SCREEN_TV, 0x0000000);
		OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);
			
		endRefresh();
		return 1;
	}
	
	uint32_t RGBRainbowResult;

	RGBRainbowResult = rR * 0x1000000;
	RGBRainbowResult += gR * 0x10000;
	RGBRainbowResult += bR * 0x100;
	OSScreenClearBufferEx(SCREEN_TV, RGBRainbowResult);
	OSScreenClearBufferEx(SCREEN_DRC, RGBRainbowResult);
	
	swrite(0, 0, std::string("R: ") + std::to_string(rR));
	swrite(0, 1, std::string("G: ") + std::to_string(gR));
	swrite(0, 2, std::string("B: ") + std::to_string(bR));
					
	swrite(8, 0, std::string("[") + hex_tostring(rR, 2) + std::string("]"));
	swrite(8, 1, std::string("[") + hex_tostring(gR, 2) + std::string("]"));
	swrite(8, 2, std::string("[") + hex_tostring(bR, 2) + std::string("]"));
					
	if (checkReturn())
		return 2;

	endRefresh();
	return 0;
}

bool subMenuRainbow() {
	uint8_t rR = 255;
	uint8_t gR = 0;
	uint8_t bR = 0;
	
	uint8_t* tempByte = 0;

	bool negative = false;
	while (true)
	{
		for (uint8_t t = 0; t < 2; t++) { //Times
			for (uint8_t c = 0; c <= 2; c++) { //Colors (RGB)
				switch(c) {
					case 0:
						tempByte = &gR;
						break;
					case 1:
						tempByte = &rR;
						break;
					case 2:
						tempByte = &bR;
						break;
				}
				if (!negative) {
					while (*tempByte < 255) {
						switch(subMenuRainbowSetColor(rR, gR, bR)) {
							case 1:
								return false;
							case 2:
								return true;
						}
						(*tempByte) += 5;
					}
					negative = true;
				}
				else {
					while (*tempByte > 0) {
						switch(subMenuRainbowSetColor(rR, gR, bR)) {
							case 1:
								return false;
							case 2:
								return true;
						}
						*(tempByte) -= 5;
					}
					negative = false;
				}
			}
			
		}
	}
}
bool subMenuLcd() {
	int lcdIndex = 0;
	while (WHBProcIsRunning())
	{
		if (!startRefresh())
			continue;

		swrite(0, 3, std::string("--->") + screenOptions[0]);

		VPADLcdMode vpadlm;
		VPADGetLcdMode(VPAD_CHAN_0, &vpadlm);

		swrite(0, 5, "Current LCD Mode: " + hex_tostring(vpadlm, 2));

		write(0, 7, "Set LCD Mode to:");
		write(1, 8, "ON (Default)");
		write(1, 9, "OFF (Only works as input pad)");
		write(1, 10, "Standby (Wakes up touching any button)");
		write(0, lcdIndex + 8, ">");

		switch (vpad.trigger) {
		case VPAD_BUTTON_A:
			switch (lcdIndex) {
			case 0:
				VPADSetLcdMode(VPAD_CHAN_0, VPAD_LCD_ON);
				break;
			case 1:
				VPADSetLcdMode(VPAD_CHAN_0, VPAD_LCD_OFF);
				break;
			case 2:
				VPADSetLcdMode(VPAD_CHAN_0, VPAD_LCD_STANDBY);
				break;
			}
			break;
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			if (lcdIndex > 0)
				lcdIndex--;
			else
				lcdIndex = 2;
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			if (lcdIndex < 2)
				lcdIndex++;
			else
				lcdIndex = 0;
			break;
		}
		if (checkReturn())
			return true;

		endRefresh();
	}
	return false;
}

bool menuScreen() {
	screenIndex = 0;
	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		for (int i = 0; i < 6; i++) {
			if (i == screenIndex)
				write(0, i + 4, ">");
			write(1, i + 4, screenOptions[i]);
		}

		switch (vpad.trigger) {
		case VPAD_BUTTON_A: {
			endRefresh();
			switch (screenIndex) {
			case 0:
				if (!subMenuLcd())
					return false;
				goto loopMenuScreen;
			case 1: {
				uint8_t pixelsIndex = 0;
				
				uint8_t r = 128;
				uint8_t g = 128;
				uint8_t b = 128;
				
				uint8_t *RGBTemp;
				uint32_t RGBResult = 0;
				uint32_t RGBAntiResult = 0;
				
				while (WHBProcIsRunning()) {
					if (!startRefresh())
						continue;
					
					swrite(0, 3, std::string("--->") + screenOptions[1]);
					
					write(0, 5, "Input the RGB color to set on screen");
					write(0, 7, "Move cursor between the numbers using UP and DOWN on DPAD");
					write(0, 9, "Change number fast using L Stick LEFT and RIGHT");
					write(0, 10, "Change one-by-one number using LEFT and RIGHT on DPAD");
					
					swrite(1, 12, std::string("R: ") + std::to_string(r));
					swrite(1, 13, std::string("G: ") + std::to_string(g));
					swrite(1, 14, std::string("B: ") + std::to_string(b));
					
					swrite(9, 12, std::string("[") + hex_tostring(r, 2) + std::string("]"));
					swrite(9, 13, std::string("[") + hex_tostring(g, 2) + std::string("]"));
					swrite(9, 14, std::string("[") + hex_tostring(b, 2) + std::string("]"));
					
					write(0, 12 + pixelsIndex, ">");
					
					RGBResult = r * 0x1000000;
					RGBResult += g * 0x10000;
					RGBResult += b * 0x100;
					
					RGBAntiResult = (255 - r) * 0x1000000;
					RGBAntiResult += (255 - g) * 0x10000;
					RGBAntiResult += (255 - b) * 0x100;
				
					drawFillRect(255, 320, 328, 393, RGBResult); //Square
					drawRect(256, 321, 327, 392, RGBAntiResult); //Square shadow
					
					write(0, 16, "Press A to continue");
					
					if (vpad.trigger != 0) {
						if (vpad.trigger & VPAD_BUTTON_UP && pixelsIndex > 0)
							pixelsIndex--;
						else if (vpad.trigger & VPAD_BUTTON_DOWN && pixelsIndex < 2)
							pixelsIndex++;
						
						switch(pixelsIndex) { //Crash if index is not valid
							case 0:
								RGBTemp = &r;
								break;
							case 1:
								RGBTemp = &g;
								break;
							case 2:
								RGBTemp = &b;
								break;
						}
					}
					
					if ((vpad.trigger & VPAD_BUTTON_RIGHT || vpad.hold & VPAD_STICK_L_EMULATION_RIGHT))
						++(*RGBTemp);
					else if ((vpad.trigger & VPAD_BUTTON_LEFT || vpad.hold & VPAD_STICK_L_EMULATION_LEFT))
						--(*RGBTemp);
					
					if (vpad.trigger & VPAD_BUTTON_A)
						goto pixelsTest;
				
					if (checkReturn())
						goto loopMenuScreen;

					endRefresh();
				}
				return false;
				
				pixelsTest:
				while (WHBProcIsRunning()) {
					
					if (!startCleanRefresh()) {
						OSScreenClearBufferEx(SCREEN_TV, 0x0000000);
						OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);
						
						endRefresh();
						continue;
					}
					
					OSScreenClearBufferEx(SCREEN_TV, RGBResult);
					OSScreenClearBufferEx(SCREEN_DRC, RGBResult);
					
					swrite(0, 0, std::string("R: ") + std::to_string(r));
					swrite(0, 1, std::string("G: ") + std::to_string(g));
					swrite(0, 2, std::string("B: ") + std::to_string(b));
					
					swrite(8, 0, std::string("[") + hex_tostring(r, 2) + std::string("]"));
					swrite(8, 1, std::string("[") + hex_tostring(g, 2) + std::string("]"));
					swrite(8, 2, std::string("[") + hex_tostring(b, 2) + std::string("]"));
					
					if (checkReturn())
						goto loopMenuScreen;

					endRefresh();
				}
				return false;
			}
			case 2:
				if (!subMenuRainbow())
					return false;
				goto loopMenuScreen;
			case 3:
			case 4:
			case 5: {
				bool oldFrameTouch = false;
				VPADTouchData oldFrameData;
				bool tCursor = true;
				
				std::vector<VPADTouchData> tPos;
				while (WHBProcIsRunning())
				{
					if (!startRefresh())
						continue;

					swrite(0, 3, std::string("--->") + screenOptions[screenIndex]);

					drawRect(0, 0, 853, 479, 0xFFFFFFFF); //Gamepad touch view

					/*if (tPos.size() > 1) {
						for (unsigned int i = 0; i < tPos.size(); i++) {
							drawLine(tPos[i - 1].x, tPos[i - 1].y, tPos[i].x, tPos[i].y, 0x00FF00FF);
						}
					}*/

					VPADTouchData ictd; //Initial touch data;
					VPADTouchData ctd; //Calibrated touch data

					switch (screenIndex) {
					case 3:
						ictd = vpad.tpNormal;
						break;
					case 4:
						ictd = vpad.tpFiltered1;
						break;
					case 5:
						ictd = vpad.tpFiltered2;
						break;
					}

					VPADGetTPCalibratedPoint(VPAD_CHAN_0, &ctd, &ictd);

					ctd.x = 854 * ctd.x / 1280;
					ctd.y = 480 * ctd.y / 720;

					//swrite(0, 11, std::string("tPos size: ") + std::to_string(tPos.size()));

					swrite(0, 5, "X: " + std::to_string(ctd.x) + ", Y: " + std::to_string(ctd.y));
					swrite(0, 6, std::string("Touching: ") + b_tostring(ctd.touched));
					swrite(0, 7, std::string("Validity: ") + std::to_string(ctd.validity));

					write(0, 9, ">Toggle cursor");

					if (vpad.trigger & VPAD_BUTTON_A)
						tCursor = !tCursor;

					if (tPos.size() > 1)
						for (unsigned int i = 0; i < tPos.size(); i++)
							drawPixel(tPos[i].x, tPos[i].y, 0xFFFFFFFF);

					if (ctd.touched & 1) {
						if (!oldFrameTouch)
							tPos.clear();

						if (oldFrameData.x != ctd.x && oldFrameData.y != ctd.y)
							tPos.push_back(ctd);

						if (tCursor) {
							drawLine(ctd.x - 1279, ctd.y, ctd.x + 1279, ctd.y, 0x00FF00FF);
							drawLine(ctd.x, ctd.y - 719, ctd.x, ctd.y + 719, 0x00FF00FF);
						}
					}
					oldFrameTouch = ctd.touched;
					oldFrameData = ctd;

					if (checkReturn())
						goto loopMenuScreen;

					endRefresh();
				}
				return false;
			}
		}
		case VPAD_BUTTON_UP:
		case VPAD_STICK_L_EMULATION_UP:
			if (screenIndex > 0)
				screenIndex--;
			else
				screenIndex = 5;
			break;
		case VPAD_BUTTON_DOWN:
		case VPAD_STICK_L_EMULATION_DOWN:
			if (screenIndex < 5)
				screenIndex++;
			else
				screenIndex = 0;
			break;
		}
		}
		if (checkReturn())
			return true;

		loopMenuScreen:
		endRefresh();
	}
	return false;
}

const char* checkButton(std::string name, VPADButtons button) {
	name += (vpad.trigger & button) ? "trigger" : ((vpad.hold & button) ? "hold" : ((vpad.release & button) ? "release" : ""));
	return name.c_str();
}
bool menuButtons() {
	bool pActive = true; //Joystick points active
	bool LRholding = false;
	while (WHBProcIsRunning()) {
		if (!startRefresh())
			continue;

		write(0, 4, checkButton("Button A: ", VPAD_BUTTON_A));
		write(0, 5, checkButton("Button B: ", VPAD_BUTTON_B));
		write(0, 6, checkButton("Button X: ", VPAD_BUTTON_X));
		write(0, 7, checkButton("Button Y: ", VPAD_BUTTON_Y));

		write(0, 9, checkButton("Button UP: ", VPAD_BUTTON_UP));
		write(0, 10, checkButton("Button RIGHT: ", VPAD_BUTTON_RIGHT));
		write(0, 11, checkButton("Button DOWN: ", VPAD_BUTTON_DOWN));
		write(0, 12, checkButton("Button LEFT: ", VPAD_BUTTON_LEFT));

		write(0, 14, checkButton("Button SYNC: ", VPAD_BUTTON_SYNC));

		write(23, 4, checkButton("Button L: ", VPAD_BUTTON_L));
		write(23, 5, checkButton("Button R: ", VPAD_BUTTON_R));
		write(23, 6, checkButton("Button ZL: ", VPAD_BUTTON_ZL));
		write(23, 7, checkButton("Button ZR: ", VPAD_BUTTON_ZR));

		write(23, 9, checkButton("Stick button L: ", VPAD_BUTTON_STICK_L));
		write(23, 10, checkButton("Stick button R: ", VPAD_BUTTON_STICK_R));
		write(23, 11, checkButton("Button START: ", VPAD_BUTTON_PLUS));
		write(23, 12, checkButton("Button SELECT: ", VPAD_BUTTON_MINUS));

		write(23, 14, checkButton("Button TV: ", VPAD_BUTTON_TV));

		write(0, 16, "Press L and R to de/activate joystick points");
		write(0, 17, "Press START and SELECT to return");

		//75 is radius
		//5 is size of pointer
		write(45, 0, "L stick:");
		int lcx = 853 - 75 - 20;
		int lcy = 75 + 20;
		drawCircle(lcx, lcy, 75, 0xFFFFFFFF);
		drawLine(lcx, lcy, lcx + (vpad.leftStick.x * 75), lcy + (-vpad.leftStick.y * 75), 0xFFFFFFFF);
		if (pActive)
			drawFillCircle(lcx + (vpad.leftStick.x * 75), lcy + (-vpad.leftStick.y * 75), 5, 0xFFFFFFFF);
		swrite(53, 6, ("X: " + std::to_string(vpad.leftStick.x)));
		swrite(53, 7, ("Y: " + std::to_string(vpad.leftStick.y)));

		write(45, 9, "R stick:");
		int rcx = 853 - 75 - 20;//csx;
		int rcy = 75 + 20 + 215;
		drawCircle(rcx, rcy, 75, 0xFFFFFFFF);
		drawLine(rcx, rcy, rcx + (vpad.rightStick.x * 75), rcy + (-vpad.rightStick.y * 75), 0xFFFFFFFF);
		if (pActive)
			drawFillCircle(rcx + (vpad.rightStick.x * 75), rcy + (-vpad.rightStick.y * 75), 5, 0xFFFFFFFF);
		swrite(53, 15, ("X: " + std::to_string(vpad.rightStick.x)));
		swrite(53, 16, ("Y: " + std::to_string(vpad.rightStick.y)));

		if (vpad.hold & VPAD_BUTTON_L && vpad.hold & VPAD_BUTTON_R) {
			if (!LRholding) {
				pActive = !pActive;
				LRholding = true;
			}
		}
		else
			LRholding = false;
		if (vpad.hold & VPAD_BUTTON_PLUS && vpad.hold & VPAD_BUTTON_MINUS)
			return true;

		endRefresh();
	}
	return false;
}

void displayIndex() {
	for (int i = 0; i < OPTIONS_MAX; i++) {
		if (i == menuIndex)
			write(0, i + 3, ">");
		write(1, i + 3, indexOptions[i]);
	}
}

int main() {
	WHBProcInit();
	OSScreenInit();

	tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
	drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

	tvBuffer = memalign(0x100, tvBufferSize);
	drcBuffer = memalign(0x100, drcBufferSize);

	if (!tvBuffer || !drcBuffer) {
		if (tvBuffer) free(tvBuffer);
		if (drcBuffer) free(drcBuffer);

		OSScreenShutdown();

		return 1;
	}

	OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
	OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);

	OSScreenEnableEx(SCREEN_TV, true);
	OSScreenEnableEx(SCREEN_DRC, true);

	VPADInit();

	while (WHBProcIsRunning()) {
		if (!startRefresh())
			goto refreshBuffs;

		switch (menu) {
		case 0:
			displayIndex();

			switch (vpad.trigger) {
			case VPAD_BUTTON_A:
				menu = menuIndex + 1;
				goto refreshBuffs;
			case VPAD_BUTTON_UP:
			case VPAD_STICK_L_EMULATION_UP:
				if (menuIndex > 0)
					menuIndex--;
				else
					menuIndex = OPTIONS_MAX - 1;
				break;
			case VPAD_BUTTON_DOWN:
			case VPAD_STICK_L_EMULATION_DOWN:
				if (menuIndex < OPTIONS_MAX - 1)
					menuIndex++;
				else
					menuIndex = 0;
				break;
			}
			break;
		case 1: //BUTTONS
			if (!menuButtons())
				goto exit;
			goto endMenu;
		case 2: //SCREEN
			if (!menuScreen())
				goto exit;
			goto endMenu;
		case 3: //GYROSCOPE
			if (!menuGyroAngMag(0))
				goto exit;
			goto endMenu;
		case 4: //ACCELERATION
			if (!menuAcc())
				goto exit;
			goto endMenu;
		case 5: //DIRECTION
			if (!menuDir())
				goto exit;
			goto endMenu;
			break;
		case 6: //ANGLE
			if (!menuGyroAngMag(1))
				goto exit;
			goto endMenu;
		case 7: //MAGNETOMETER
			if (!menuGyroAngMag(2))
				goto exit;
			goto endMenu;
		case 8: //AUDIO
			if (!menuAudio())
				goto exit;
			goto endMenu;
		case 9: //BATTERY
			if (!menuBattery())
				goto exit;
			goto endMenu;
		case 10: //VIBRATION
			if (!menuVibration()) {
				VPADStopMotor(VPAD_CHAN_0);
				goto exit;
			}
			VPADStopMotor(VPAD_CHAN_0);
			goto endMenu;
		case 11: //SENSOR BAR
			if (!menuSensorBar())
				goto exit;
			goto endMenu;
		case 12: //GAMEPAD BASE
			if (!menuDrcBase()) {
				OSDynLoad_Release(base_handle);
				goto exit;
			}
			OSDynLoad_Release(base_handle);
			goto endMenu;
		case 13: //CREDITS
			if (!menuCredits()) 
				goto exit;
			goto endMenu;
		}
		
		write(39, 17, "Press HOME to exit anytime");

		endMenu:
		menu = 0;
		
		refreshBuffs:
		endRefresh();
	}
exit:
	if (tvBuffer) free(tvBuffer);
	if (drcBuffer) free(drcBuffer);

	OSScreenShutdown();
	WHBProcShutdown();

	return 1;
}
