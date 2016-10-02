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
	
	int fMass = 0xC; // VERIFIED
	
	// *10000.0f!
	int fInitialDragCoeff = 0x10;  // VERIFIED
	
	vecOffset vecCentreOfMass = { // VERIFIED
		0x20, 0x24, 0x28
	};
	vecOffset vecInertiaMultiplier = { // VERIFIED
		0x30, 0x34, 0x38
	};
	int fPercentSubmerged = 0x40; // VERIFIED

	// 2*meta
	//int fDriveBiasFront = 0x48; // No? 0x48?
	
	// 2*(1.0-meta)
	int fDriveBiasFront = 0x4C;
	
	uint8_t nInitialDriveGears = 0x50; // VERIFIED
	int fDriveInertia = 0x54; // VERIFIED
	int fClutchChangeRateScaleUpShift = 0x58; // VERIFIED
	int fClutchChangeRateScaleDownShift = 0x5C; // VERIFIED
	int fInitialDriveForce = 0x60; // VERIFIED
	
   //int fInitialDriveMaxFlatVel = 0x64; // meta/3 // doesn't seem to read changes
	int fInitialDriveMaxFlatVel = 0x68; // meta/3.6 (kph -> m/s)
	
	// NORMAL/VERIFIED
	int fBrakeForce = 0x6C;
	
	// 2*meta
	//int fBrakeBiasFront = 0x74;

	// 2*(1.0-meta)
	int fBrakeBiasFront = 0x78;


	int fHandBrakeForce = 0x7C; // VERIFIED

	int fSteeringLock = 0x80; // (meta*pi)/180 (degrees -> radians), for inner wheel
	
	int fTractionCurveMax = 0x88; // VERIFIED
	//int m_fAcceleration = 0x8C; // ???

	int fTractionCurveMin = 0x90; // VERIFIED

	//int m_fGrip = 0x9C; // lateral grip?

	int fTractionSpringDeltaMax = 0xA0; // VERIFIED
	int fLowSpeedTractionLostMult = 0xA8; // VERIFIED
	int fCamberStiffness = 0xAC; // VERIFIED

	// meta*2
	// int fTractionBiasFront = 0xB0

	// 1.0f - (value / 2.0f)
	int fTractionBiasFront = 0xB4;

	int fTractionLossMult = 0xB8;	// VERIFIED
	int fSuspensionForce = 0xBC; // VERIFIED
	int fSuspensionCompDamp = 0xC0; // VERIFIED
	int fSuspensionReboundDamp = 0xC4; // VERIFIED
	int fSuspensionUpperLimit = 0xC8; // VERIFIED
	int fSuspensionLowerLimit = 0xCC; // VERIFIED
	int fSuspensionRaise = 0xD0; // VERIFIED

	// meta*2
	//int fSuspensionBiasFront = 0xD4; // VERIFIED

	// 1.0-(meta/2)
	int fSuspensionBiasFront = 0xD8; // VERIFIED

	int fAntiRollBarForce = 0xDC; // VERIFIED
	int fRollCentreHeightFront = 0xE8; // VERIFIED
	int fRollCentreHeightRear = 0xEC; // VERIFIED

	int fCollisionDamageMult = 0xF0; // VERIFIED
	int fWeaponDamageMult = 0xF4; // VERIFIED
	int fDeformationDamageMult = 0xF8; // VERIFIED
	int fEngineDamageMult = 0xFC; // VERIFIED
	
	int fPetrolTankVolume = 0x100; // VERIFIED
	int fOilVolume = 0x104; // VERIFIED
	
	int fSeatOffsetDistX = 0x10C; // VERIFIED
	int fSeatOffsetDistY = 0x110; // VERIFIED
	int fSeatOffsetDistZ = 0x114; // VERIFIED

	int nMonetaryValue = 0x118; // VERIFIED
	int dwStrModelFlags = 0x11C; // VERIFIED
	int dwStrHandlingFlags = 0x120; // VERIFIED
	int dwStrDamageFlags = 0x124; // VERIFIED
	//int dwAIHandlingHash = 0x134; // VERIFIED ???
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

// 2.0f * value
float getHandlingValueHalf(Vehicle veh, int valueOffset) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	float value = *reinterpret_cast<float *>(handlingPtr + valueOffset);
	return 2*value;
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

uint8_t getHandlingValueInt(Vehicle veh, int valueOffset) {
	const uint64_t address = mem.GetAddressOfEntity(veh);
	uint64_t handlingPtr = *reinterpret_cast<uint64_t *>(address + handlingOffset);
	return *reinterpret_cast<uint8_t *>(handlingPtr + valueOffset);
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

void showPhysicsValues(Vector3 velocities, Vector3 accelValsAvg, float xPos, float yPos) {
	std::stringstream ssVRawY;
	ssVRawY << "velY = " << velocities.y;
	showText(xPos, yPos, 0.4f, ssVRawY.str().c_str());

	std::stringstream ssVRawX;
	ssVRawX << "velX = " << velocities.x;
	showText(xPos, yPos+0.025f, 0.4f, ssVRawX.str().c_str());

	std::stringstream ssAccelAvgY;
	ssAccelAvgY << "avgAY = " << accelValsAvg.y;
	showText(xPos, yPos+0.050f, 0.4f, ssAccelAvgY.str().c_str());

	std::stringstream ssAccelAvgX;
	ssAccelAvgX << "avgAX = " << accelValsAvg.x;
	showText(xPos, yPos+0.075f, 0.4f, ssAccelAvgX.str().c_str());

	std::stringstream ssRotVal;
	ssRotVal << "rotZ = " << vehData.RotationVelocity.z;
	showText(xPos, yPos+0.100f, 0.4f, ssRotVal.str().c_str());

	std::stringstream ssGForceL;
	ssGForceL << "G-Force X = " << (vehData.RotationVelocity.z*vehData.Velocity) / 9.81f;
	showText(xPos, yPos+0.125f, 0.4f, ssGForceL.str().c_str());

	std::stringstream ssGForceY;
	ssGForceY << "G-Force Y = " << (accelValsAvg.y) / 9.81f;
	showText(xPos, yPos+0.150f, 0.4f, ssGForceY.str().c_str());
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

	float posX = GetPrivateProfileIntA("util", "infox", 1, SETTINGSFILE)/100.0f;
	float posY = GetPrivateProfileIntA("util", "infoy", 30, SETTINGSFILE)/100.0f;
	
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

	showPhysicsValues(velocities, accelValsAvg, posX, posY);




		
	/*std::stringstream ss_fBrakeBiasFront;
	ss_fBrakeBiasFront << "fBrakeBiasFront = " << get_fBrakeBiasFront(vehicle);
	showText(0.2, 0.2, 1.0, ss_fBrakeBiasFront.str().c_str());

	std::stringstream ss_fTractionBiasFront;
	ss_fTractionBiasFront << "fTractionBiasFront = " << get_fTractionBiasFront(vehicle);
	showText(0.2, 0.3, 1.0, ss_fTractionBiasFront.str().c_str());*/
	//logger.Write(" = " + std::to_string());
	if (IsKeyJustUp(logKey)) {
		std::stringstream vehName;
		vehName << "VEHICLE: " << VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
		Logger logger(LOGFILE);
		logger.Write(vehName.str());
		logger.Write("fMass = " + std::to_string(getHandlingValue(vehicle, hOffsets.fMass)));
		logger.Write("fInitialDragCoeff = " + std::to_string(getHandlingValueMult10000(vehicle, hOffsets.fInitialDragCoeff)));
		logger.Write("fPercentSubmerged = " + std::to_string(getHandlingValue(vehicle, hOffsets.fPercentSubmerged)));

		logger.Write("vecCentreOfMassOffsetX = " + std::to_string(getHandlingValue(vehicle, hOffsets.vecCentreOfMass.X)));
		logger.Write("vecCentreOfMassOffsetY = " + std::to_string(getHandlingValue(vehicle, hOffsets.vecCentreOfMass.Y)));
		logger.Write("vecCentreOfMassOffsetZ = " + std::to_string(getHandlingValue(vehicle, hOffsets.vecCentreOfMass.Z)));

		logger.Write("vecInertiaMultiplierX = " + std::to_string(getHandlingValue(vehicle, hOffsets.vecInertiaMultiplier.X)));
		logger.Write("vecInertiaMultiplierY = " + std::to_string(getHandlingValue(vehicle, hOffsets.vecInertiaMultiplier.Y)));
		logger.Write("vecInertiaMultiplierZ = " + std::to_string(getHandlingValue(vehicle, hOffsets.vecInertiaMultiplier.Z)));

		logger.Write("fDriveBiasFront = " + std::to_string(getHandlingValueInvHalf(vehicle, hOffsets.fDriveBiasFront)));

		logger.Write("nInitialDriveGears = " + std::to_string(getHandlingValueInt(vehicle, hOffsets.nInitialDriveGears)));
		logger.Write("fInitialDriveForce = " + std::to_string(getHandlingValue(vehicle, hOffsets.fInitialDriveForce)));
		logger.Write("fDriveInertia = " + std::to_string(getHandlingValue(vehicle, hOffsets.fDriveInertia)));
		logger.Write("fClutchChangeRateScaleUpShift = " + std::to_string(getHandlingValue(vehicle, hOffsets.fClutchChangeRateScaleUpShift)));
		logger.Write("fClutchChangeRateScaleDownShift = " + std::to_string(getHandlingValue(vehicle, hOffsets.fClutchChangeRateScaleDownShift)));
		logger.Write("fInitialDriveMaxFlatVel = " + std::to_string(getHandlingValue(vehicle, hOffsets.fInitialDriveMaxFlatVel)*3.6f)); // m/s -> kph
		
		logger.Write("fBrakeForce = " + std::to_string(getHandlingValue(vehicle, hOffsets.fBrakeForce)));
		logger.Write("fBrakeBiasFront = " + std::to_string(getHandlingValueInvHalf(vehicle, hOffsets.fBrakeBiasFront)));
		logger.Write("fHandBrakeForce = " + std::to_string(getHandlingValue(vehicle, hOffsets.fHandBrakeForce)));
		
		logger.Write("fSteeringLock = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSteeringLock)*(180.0f / 3.14159265359f))); // rad -> deg
		logger.Write("fTractionCurveMax = " + std::to_string(getHandlingValue(vehicle, hOffsets.fTractionCurveMax)));
		logger.Write("fTractionCurveMin = " + std::to_string(getHandlingValue(vehicle, hOffsets.fTractionCurveMin)));
		logger.Write("fTractionCurveLateral = "); // MISSING!!!!!
		logger.Write("fTractionSpringDeltaMax = " + std::to_string(getHandlingValue(vehicle, hOffsets.fTractionSpringDeltaMax)));
		logger.Write("fLowSpeedTractionLossMult = "); // MISSING!!!!!
		logger.Write("fCamberStiffness = " + std::to_string(getHandlingValue(vehicle, hOffsets.fCamberStiffness)));
		logger.Write("fTractionBiasFront = " + std::to_string(getHandlingValueInvHalf(vehicle, hOffsets.fTractionBiasFront)));
		logger.Write("fTractionLossMult = " + std::to_string(getHandlingValue(vehicle, hOffsets.fTractionLossMult)));
		
		logger.Write("fSuspensionForce = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSuspensionForce)));
		logger.Write("fSuspensionCompDamp = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSuspensionCompDamp)*10.0f));
		logger.Write("fSuspensionReboundDamp = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSuspensionReboundDamp)*10.0f));
		logger.Write("fSuspensionUpperLimit = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSuspensionUpperLimit)));
		logger.Write("fSuspensionLowerLimit = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSuspensionLowerLimit)));
		logger.Write("fSuspensionRaise = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSuspensionRaise)));
		logger.Write("fSuspensionBiasFront = " + std::to_string(getHandlingValueInvHalf(vehicle, hOffsets.fSuspensionBiasFront)));
		logger.Write("fAntiRollBarForce = " + std::to_string(getHandlingValue(vehicle, hOffsets.fAntiRollBarForce)));
		logger.Write("fAntiRollBarBiasFront = "); // MISSING!!!!!
		logger.Write("fRollCentreHeightFront = " + std::to_string(getHandlingValue(vehicle, hOffsets.fRollCentreHeightFront)));
		logger.Write("fRollCentreHeightRear = " + std::to_string(getHandlingValue(vehicle, hOffsets.fRollCentreHeightRear)));
		logger.Write("fCollisionDamageMult = " + std::to_string(getHandlingValue(vehicle, hOffsets.fCollisionDamageMult)));
		logger.Write("fWeaponDamageMult = " + std::to_string(getHandlingValue(vehicle, hOffsets.fWeaponDamageMult)));
		logger.Write("fDeformationDamageMult = " + std::to_string(getHandlingValue(vehicle, hOffsets.fDeformationDamageMult)));
		logger.Write("fEngineDamageMult = " + std::to_string(getHandlingValue(vehicle, hOffsets.fEngineDamageMult)));
		logger.Write("fPetrolTankVolume = " + std::to_string(getHandlingValue(vehicle, hOffsets.fPetrolTankVolume)));
		logger.Write("fOilVolume = " + std::to_string(getHandlingValue(vehicle, hOffsets.fOilVolume)));
		logger.Write("fSeatOffsetDistX = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSeatOffsetDistX)));
		logger.Write("fSeatOffsetDistY = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSeatOffsetDistY)));
		logger.Write("fSeatOffsetDistZ = " + std::to_string(getHandlingValue(vehicle, hOffsets.fSeatOffsetDistZ)));
		logger.Write("------------------------------");
	}

	if (IsKeyJustUp(readKey)) {

		int raw_fBrakeBiasFront 	= GetPrivateProfileIntA("handling", "fBrakeBiasFront", -1337, SETTINGSFILE);
		int raw_fTractionBiasFront	= GetPrivateProfileIntA("handling", "fTractionBiasFront", -1337, SETTINGSFILE);
		int raw_fDriveInertia		= GetPrivateProfileIntA("handling", "fDriveInertia", -1337, SETTINGSFILE);
		int raw_fBrakeForce			= GetPrivateProfileIntA("handling", "fBrakeForce", -1337, SETTINGSFILE);
		int raw_fInitialDragCoeff	= GetPrivateProfileIntA("handling", "fInitialDragCoeff", -1337, SETTINGSFILE);

		int raw_vecCentreOfMassOffsetX = GetPrivateProfileIntA("handling", "vecCentreOfMassOffsetX", -1337, SETTINGSFILE);
		int raw_vecCentreOfMassOffsetY = GetPrivateProfileIntA("handling", "vecCentreOfMassOffsetY", -1337, SETTINGSFILE);
		int raw_vecCentreOfMassOffsetZ = GetPrivateProfileIntA("handling", "vecCentreOfMassOffsetZ", -1337, SETTINGSFILE);


		if (raw_fBrakeBiasFront != -1337) {
			setHandlingValueInvHalf(vehicle, hOffsets.fBrakeBiasFront, raw_fBrakeBiasFront / 1000000.0f);
		}
		
		if (raw_fTractionBiasFront != -1337) {
			setHandlingValueInvHalf(vehicle, hOffsets.fTractionBiasFront, raw_fTractionBiasFront / 1000000.0f);
		}
		
		if (raw_fDriveInertia != -1337) {
			setHandlingValue(vehicle, hOffsets.fDriveInertia, raw_fDriveInertia / 1000000.0f);
		}
		
		if (raw_fBrakeForce != -1337) {
			setHandlingValue(vehicle, hOffsets.fBrakeForce, raw_fBrakeForce / 1000000.0f);
		}

		if (raw_fInitialDragCoeff != -1337) {
			setHandlingValueMult10000(vehicle, hOffsets.fInitialDragCoeff, raw_fInitialDragCoeff / 1000000.0f);
		}

		if (raw_vecCentreOfMassOffsetX != -1337) {
			setHandlingValue(vehicle, hOffsets.vecCentreOfMass.X, raw_vecCentreOfMassOffsetX / 1000000.0f);
		}

		if (raw_vecCentreOfMassOffsetY != -1337) {
			setHandlingValue(vehicle, hOffsets.vecCentreOfMass.Y, raw_vecCentreOfMassOffsetY / 1000000.0f);
		}

		if (raw_vecCentreOfMassOffsetZ != -1337) {
			setHandlingValue(vehicle, hOffsets.vecCentreOfMass.Z, raw_vecCentreOfMassOffsetZ / 1000000.0f);
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
