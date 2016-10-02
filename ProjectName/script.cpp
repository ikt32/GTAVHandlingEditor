/*
 * GTA V Real Time Handling editor
 * This plugin is intended for help with creating handling mods, preventing
 * the need to restart the game to test each change to each parameter. As such
 * this plugin is not intended for normal usage during gameplay.
 * 
 * These offsets are based on game version VER_1_0_791_2_STEAM
 */

#include "script.h"
#include "NativeMemory.hpp"
#include <sstream>
#include "keyboard.h"
#include <iomanip>
#include <fstream>
#include "VehicleData.h"

#define SETTINGSFILE "./RTHandlingEditor.ini"
#define LOGFILE "./RTHandlingEditor.log"

Player player;
Ped playerPed;
Vehicle vehicle;
Hash model;
MemoryAccess mem;
VehicleData vehData;

int handlingOffset = 0x830;

struct vecOffset {
	int X;
	int Y;
	int Z;
};

const struct HandlingOffset {
	
	int fMass = 0xC;
	
	// *10000.0f!
	int fInitialDragCoeff = 0x10; 
	
	vecOffset vecCentreOfMass = {
		0x20, 0x24, 0x28
	};
	vecOffset vecInteriaMultiplier = {
		0x30, 0x34, 0x38
	};
	int fPercentSubmerged = 0x40;
	int fDriveBiasFront = 0x4C;
	int nInitialDriveGears = 0x50;
	int fDriveInertia = 0x54;
	int fClutchChangeRateScaleUpShift = 0x58;
	int fClutchChangeRateScaleDownShift = 0x5C;
	int fInitialDriveForce = 0x60;
	int fInitialDriveMaxFlatVel = 0x64;
	//int fSteeringLock = 0x68; // GUESSING
	
	// NORMAL/VERIFIED
	int fBrakeForce = 0x6C;
	
	// 1.0f - (value / 2.0f)
	int fBrakeBiasFront = 0x78;
	int fHandBrakeForce = 0x7C;
	int fSteeringLock = 0x80; // GUESSING
	int fTractionCurveMax = 0x88;
	//int m_fAcceleration = 0x8C; // ???

	int fTractionCurveMin = 0x90;
	//int fInitialDragCoeff = 0x94; //WRONG?
	int m_fGrip = 0x9C;
	int fTractionSpringDeltaMax = 0xA0;
	int fLowSpeedTractionLostMult = 0xA8;
	int fCamberStiffness = 0xAC; // GUESSING

	// 1.0f - (value / 2.0f)
	int fTractionBiasFront = 0xB4;

	int fTractionLossMult = 0xB8;	// VERIFIED
	int fSuspensionForce = 0xBC; // VERIFIED
	int fSuspensionCompDamp = 0xC0;
	int fSuspensionReboundDamp = 0xC4;
	int fSuspensionUpperLimit = 0xC8;
	int fSuspensionLowerLimit = 0xCC;
	int fAntiRollBarForce = 0xDC;
	
	int fRollCentreHeightFront = 0xE8; // VERIFIED
	int fRollCentreHeightRear = 0xEC; // VERIFIED
	int fCollisionDamageMult = 0xF0; // VERIFIED
	int fWeaponDamageMult = 0xF4; // VERIFIED
	int fDeformationDamageMult = 0xF8; // VERIFIED
	int fEngineDamageMult = 0xFC; // VERIFIED
	int fPetrolTankVolume = 0x100; // VERIFIED
	int fOilVolume = 0x104; // VERIFIED
	int fSeatOffsetDistX = 0x10C;
	int fSeatOffsetDistY = 0x110; // VERIFIED
	int fSeatOffsetDistZ = 0x114;
	int dwMonetaryValue = 0x118; // VERIFIED
	int dwStrModelFlags = 0x11C; // VERIFIED
	int dwStrHandlingFlags = 0x120;
	int dwStrDamageFlags = 0x124;
	int dwAIHandlingHash = 0x134; // VERIFIED
} hOffsets = {};


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

// 1.0f - (value / 2.0f)
float getHandlingValueInvHalf(Vehicle veh, int valueOffset) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	float value = *reinterpret_cast<float *>(handlingPtr + valueOffset);
	return 1.0f - (value / 2.0f);
}

// no change between handling.meta and memory
float getHandlingValue(Vehicle veh, int valueOffset) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	return *reinterpret_cast<float *>(handlingPtr + valueOffset);
	//return 1.0f - (value / 2.0f);
}

// *10000.0f
float getHandlingValueMult10000(Vehicle veh, int valueOffset) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	return *reinterpret_cast<float *>(handlingPtr + valueOffset) * 10000.0f;
	//return 1.0f - (value / 2.0f);
}

/*
 * Setters
 */

void setHandlingValueInvHalf(Vehicle veh, int valueOffset, float val) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	float invHalf = 2.0f*(1.0f - (val));
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	*reinterpret_cast<float *>(handlingPtr + valueOffset) = invHalf;
}

void setHandlingValue(Vehicle veh, int valueOffset, float val) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	*reinterpret_cast<float *>(handlingPtr + valueOffset) = val;
}

void setHandlingValueMult10000(Vehicle veh, int valueOffset, float val) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	*reinterpret_cast<float *>(handlingPtr + valueOffset) = val/10000.0f;
}

/*
 * main loop
 */

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

	vehData.UpdateValues(vehicle);
	Vector3 velocities = ENTITY::GET_ENTITY_SPEED_VECTOR(vehicle, true);
	Vector3 accelVals = vehData.getAccelerationVectors(velocities);
	Vector3 accelValsAvg = vehData.getAccelerationVectorsAverage();

	/*std::stringstream ssAccelRawY;
	ssAccelRawY << "rawAY = " << accelVals.y;
	showText(0.01f, 0.300f, 0.4f, ssAccelRawY.str().c_str());

	std::stringstream ssAccelRawX;
	ssAccelRawX << "rawAX = " << accelVals.x;
	showText(0.01f, 0.325f, 0.4f, ssAccelRawX.str().c_str());*/

	std::stringstream ssVRawY;
	ssVRawY << "velY = " << velocities.y;
	showText(0.01f, 0.300f, 0.4f, ssVRawY.str().c_str());

	std::stringstream ssVRawX;
	ssVRawX << "velX = " << velocities.x;
	showText(0.01f, 0.325f, 0.4f, ssVRawX.str().c_str());

	std::stringstream ssAccelAvgY;
	ssAccelAvgY << "avgAY = " << accelValsAvg.y;
	showText(0.01f, 0.350f, 0.4f, ssAccelAvgY.str().c_str());

	std::stringstream ssAccelAvgX;
	ssAccelAvgX << "avgAX = " << accelValsAvg.x;
	showText(0.01f, 0.375f, 0.4f, ssAccelAvgX.str().c_str());

	std::stringstream ssRotVal;
	ssRotVal << "rotZ = " << vehData.RotationVelocity.z;
	showText(0.01f, 0.400f, 0.4f, ssRotVal.str().c_str());

	std::stringstream ssGForceL;
	ssGForceL << "G-Force X = " << (vehData.RotationVelocity.z*vehData.Velocity) / 9.81f;
	showText(0.01f, 0.425, 0.4f, ssGForceL.str().c_str());

	std::stringstream ssGForceY;
	ssGForceY << "G-Force Y = " << (accelValsAvg.y) / 9.81f;
	showText(0.01f, 0.450, 0.4f, ssGForceY.str().c_str());




		
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
		logger.Write("fBrakeBiasFront = " + std::to_string(getHandlingValueInvHalf(vehicle, hOffsets.fBrakeBiasFront)));
		logger.Write("fTractionBiasFront = " + std::to_string(getHandlingValueInvHalf(vehicle, hOffsets.fTractionBiasFront)));
		logger.Write("fDriveInertia = " + std::to_string(getHandlingValue(vehicle, hOffsets.fDriveInertia)));
		logger.Write("fBrakeForce = " + std::to_string(getHandlingValue(vehicle, hOffsets.fBrakeForce)));
		logger.Write("fInitialDragCoeff = " + std::to_string(getHandlingValueMult10000(vehicle, hOffsets.fInitialDragCoeff)));
		logger.Write("------------------------------");
	}

	if (IsKeyJustUp(readKey)) {

		float raw_fBrakeBiasFront 	= GetPrivateProfileIntA("handling", "fBrakeBiasFront", -1337, SETTINGSFILE) / 1000000.0f;
		float raw_fTractionBiasFront	= GetPrivateProfileIntA("handling", "fTractionBiasFront", -1337, SETTINGSFILE) / 1000000.0f;
		float raw_fDriveInertia		= GetPrivateProfileIntA("handling", "fDriveInertia", -1337, SETTINGSFILE) / 1000000.0f;
		float raw_fBrakeForce			= GetPrivateProfileIntA("handling", "fBrakeForce", -1337, SETTINGSFILE) / 1000000.0f;
		float raw_fInitialDragCoeff	= GetPrivateProfileIntA("handling", "fInitialDragCoeff", -1337, SETTINGSFILE) / 1000000.0f;

		if (raw_fBrakeBiasFront != -1337) {
			setHandlingValueInvHalf(vehicle, hOffsets.fBrakeBiasFront, raw_fBrakeBiasFront);
		}
		
		if (raw_fTractionBiasFront != -1337) {
			setHandlingValueInvHalf(vehicle, hOffsets.fTractionBiasFront, raw_fTractionBiasFront);
		}
		
		if (raw_fDriveInertia != -1337) {
			setHandlingValue(vehicle, hOffsets.fDriveInertia, raw_fDriveInertia);
		}
		
		if (raw_fBrakeForce != -1337) {
			setHandlingValue(vehicle, hOffsets.fBrakeForce, raw_fBrakeForce);
		}

		if (raw_fInitialDragCoeff != -1337) {
			setHandlingValueMult10000(vehicle, hOffsets.fInitialDragCoeff, raw_fInitialDragCoeff);
		}
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
