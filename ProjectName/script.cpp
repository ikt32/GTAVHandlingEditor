#include "script.h"
#include "NativeMemory.hpp"
#include <sstream>
#include "keyboard.h"
#include <iomanip>
#include <fstream>

#define SETTINGSFILE "./RTHandlingEditor.ini"
#define LOGFILE "./RTHandlingEditor.log"

Player player;
Ped playerPed;
Vehicle vehicle;
Hash model;
MemoryAccess mem;

class Logger {
public:
	explicit Logger(char* fileName);
	void Clear() const;
	void Write(const std::string& text) const;

private:
	char* file;
};

Logger::Logger(char* fileName) {
	file = fileName;
}

void Logger::Clear() const {
	std::ofstream logFile;
	logFile.open(file, std::ofstream::out | std::ofstream::trunc);
	logFile.close();
}

void Logger::Write(const std::string& text) const {
	std::ofstream logFile(file, std::ios_base::out | std::ios_base::app);
	SYSTEMTIME currTimeLog;
	GetLocalTime(&currTimeLog);
	logFile << "[" <<
		std::setw(2) << std::setfill('0') << currTimeLog.wHour << ":" <<
		std::setw(2) << std::setfill('0') << currTimeLog.wMinute << ":" <<
		std::setw(2) << std::setfill('0') << currTimeLog.wSecond << "." <<
		std::setw(3) << std::setfill('0') << currTimeLog.wMilliseconds << "] " <<
		text << "\n";
}


void showText(float x, float y, float scale, const char* text) {
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(scale, scale);
	UI::SET_TEXT_COLOUR(255, 255, 255, 255);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(const_cast<char *>(text));
	UI::_DRAW_TEXT(x, y);
}

float get_fBrakeBiasFront(Vehicle veh) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	int handlingOffset = 0x830;
	int valueOffset = 0x78;
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	float value = *reinterpret_cast<float *>(handlingPtr+valueOffset);
	return 1.0f-(value/2.0f);
}

float get_fTractionBiasFront(Vehicle veh) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	int handlingOffset = 0x830;
	int valueOffset = 0xB4;
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	float value = *reinterpret_cast<float *>(handlingPtr + valueOffset);
	return 1.0f - (value / 2.0f);
}

void set_fBrakeBiasFront(Vehicle veh, float val) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	int handlingOffset = 0x830;
	int valueOffset = 0x78;
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);

	*reinterpret_cast<float *>(handlingPtr + valueOffset) = val;
}

void set_fTractionBiasFront(Vehicle veh, float val) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	int handlingOffset = 0x830;
	int valueOffset = 0xB4;
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	*reinterpret_cast<float *>(handlingPtr + valueOffset) = val;
}

void update()
{
	char kbKeyBuffer[24];
	GetPrivateProfileStringA("key", "read", "O", kbKeyBuffer, 24, SETTINGSFILE);
	int readKey = str2key(kbKeyBuffer);

	GetPrivateProfileStringA("key", "log", "L", kbKeyBuffer, 24, SETTINGSFILE);
	int logKey = str2key(kbKeyBuffer);
	
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player))
		return;

	// check for player ped death and player arrest
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
		return;

	vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
	model = ENTITY::GET_ENTITY_MODEL(vehicle);

	if (!ENTITY::DOES_ENTITY_EXIST(vehicle))
		return;
	
	/*std::stringstream ss_fBrakeBiasFront;
	ss_fBrakeBiasFront << "fBrakeBiasFront = " << get_fBrakeBiasFront(vehicle);
	showText(0.2, 0.2, 1.0, ss_fBrakeBiasFront.str().c_str());

	std::stringstream ss_fTractionBiasFront;
	ss_fTractionBiasFront << "fTractionBiasFront = " << get_fTractionBiasFront(vehicle);
	showText(0.2, 0.3, 1.0, ss_fTractionBiasFront.str().c_str());*/

	if (IsKeyJustUp(logKey)) {
		std::stringstream vehName;
		vehName << "VEHICLE: " << VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
		Logger logger(LOGFILE);
		logger.Write(vehName.str());
		logger.Write("fBrakeBiasFront = " + std::to_string(get_fBrakeBiasFront(vehicle)));
		logger.Write("fTractionBiasFront = " + std::to_string(get_fTractionBiasFront(vehicle)));
		logger.Write("------------------------------");
	}

	if (IsKeyJustUp(readKey)) {

		float fBrakeBiasFront = 2.0f*(1.0f-(GetPrivateProfileIntA("handling", "fBrakeBiasFront", -1000000, SETTINGSFILE) / 1000000.0f));
		float fTractionBiasFront = 2.0f*(1.0f-(GetPrivateProfileIntA("handling", "fTractionBiasFront", -1000000, SETTINGSFILE) / 1000000.0f));

		set_fBrakeBiasFront(vehicle, fBrakeBiasFront);
		set_fTractionBiasFront(vehicle, fTractionBiasFront);
	}

}

void main()
{
	while (true)
	{
		update();
		WAIT(0);
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}
