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
#include "../inih/cpp/INIReader.h"
#include "../tinyxml2/tinyxml2.h"

#define SETTINGSFILE "./RTHandlingEditor.ini"
#define LOGFILE "./RTHandlingEditor.log"

Player player;
Ped playerPed;
Vehicle vehicle;
Hash model;
MemoryAccess mem;
VehicleData vehData;
int prevNotification = 0;

int gameVersion = getGameVersion();
int handlingOffset = (gameVersion > 25 ? 0x850 : 0x830);

float disableVal = -1337.0f;

struct vecOffset {
	int X;
	int Y;
	int Z;
};

const struct HandlingOffset {
	DWORD dwHandlingNameHash = 0x0008;
	int fMass = 0x000C;
	int fInitialDragCoeff = 0x0010;
	// 0x0014
	// 0x0018
	// 0x001C
	vecOffset vecCentreOfMass = {
		0x0020,
		0x0024,
		0x0028};
	// 0x002C
	vecOffset vecInertiaMultiplier = {
		0x0030,
		0x0034,
		0x0038};
	// 0x003C
	int fPercentSubmerged = 0x0040;
	int fSubmergedRatio = 0x0044;
	int fDriveBiasFront = 0x0048;
	int fDriveBiasRear = 0x004C;
	uint8_t nInitialDriveGears = 0x0050;
	int fDriveInertia = 0x0054;
	int fClutchChangeRateScaleUpShift = 0x0058;
	int fClutchChangeRateScaleDownShift = 0x005C;
	int fInitialDriveForce = 0x0060;
	int fDriveMaxFlatVel = 0x0064;
	int fInitialDriveMaxFlatVel = 0x0068;
	int fBrakeForce = 0x006C;
	// 0x0070
	int fBrakeBiasFront = 0x0074;
	int fBrakeBiasRear = 0x0078;
	int fHandBrakeForce = 0x007C;
	int fSteeringLock = 0x0080;
	int fSteeringLockRatio = 0x0084;
	int fTractionCurveMax = 0x0088;
	int fTractionCurveMaxRatio = 0x008C;
	int fTractionCurveMin = 0x0090;
	int fTractionCurveRatio = 0x0094;
	int fTractionCurveLateral = 0x0098;
	int fTractionCurveLateralRatio = 0x009C;
	int fTractionSpringDeltaMax = 0x00A0;
	int fTractionSpringDeltaMaxRatio = 0x00A4;
	int fLowSpeedTractionLossMult = 0x00A8;
	int fCamberStiffness = 0x00AC;
	int fTractionBiasFront = 0x00B0;
	int fTractionBiasRear = 0x00B4;
	int fTractionLossMult = 0x00B8;
	int fSuspensionForce = 0x00BC;
	int fSuspensionCompDamp = 0x00C0;
	int fSuspensionReboundDamp = 0x00C4;
	int fSuspensionUpperLimit = 0x00C8;
	int fSuspensionLowerLimit = 0x00CC;
	int fSuspensionRaise = 0x00D0;
	int fSuspensionBiasFront = 0x00D4;
	int fSuspensionBiasRear = 0x00D8;
	int fAntiRollBarForce = 0x00DC;
	int fAntiRollBarBiasFront = 0x00E0;
	int fAntiRollBarBiasRear = 0x00E4;
	int fRollCentreHeightFront = 0x00E8;
	int fRollCentreHeightRear = 0x00EC;
	int fCollisionDamageMult = 0x00F0;
	int fWeaponDamageMult = 0x00F4;
	int fDeformationDamageMult = 0x00F8;
	int fEngineDamageMult = 0x00FC;
	int fPetrolTankVolume = 0x0100;
	int fOilVolume = 0x0104;
	// 0x0108
	int fSeatOffsetDistX = 0x010C;
	int fSeatOffsetDistY = 0x0110;
	int fSeatOffsetDistZ = 0x0114;
	int nMonetaryValue = 0x0118;
	DWORD dwStrModelFlags = 0x011C;
	DWORD dwStrHandlingFlags = 0x0120;
	DWORD dwStrDamageFlags = 0x0124;
	DWORD dwAIHandlingHash = 0x0134;
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

void showNotification(char* message) {
	if (prevNotification)
		UI::_REMOVE_NOTIFICATION(prevNotification);
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(message);
	prevNotification = UI::_DRAW_NOTIFICATION(false, false);
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

void showPhysicsValues(Vector3 velocities, Vector3 accelValsAvg, float xPos, float yPos, float size) {
	/*std::stringstream ssVRawY;
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
	showText(xPos, yPos+0.100f, 0.4f, ssRotVal.str().c_str());*/

	std::stringstream ssGForceL;
	ssGForceL << "G-Force Lat  = " << (vehData.RotationVelocity.z*vehData.Velocity) / 9.81f;
	showText(xPos, yPos, size, ssGForceL.str().c_str());

	std::stringstream ssGForceY;
	ssGForceY << "G-Force Long = " << (accelValsAvg.y) / 9.81f;
	showText(xPos, yPos + (size * 0.025f)/0.40f, size, ssGForceY.str().c_str());

	std::stringstream ssAccelAvgY;
	ssAccelAvgY << "Accel Y (m/s2) = " << accelValsAvg.y;
	showText(xPos, yPos + (size * 0.050f)/0.40f, size, ssAccelAvgY.str().c_str());
}


void setHandling(float fMass,
                 float fInitialDragCoeff,
                 float fPercentSubmerged,
                 float vecCentreOfMassOffsetX,
                 float vecCentreOfMassOffsetY,
                 float vecCentreOfMassOffsetZ,
                 float vecInertiaMultiplierX,
                 float vecInertiaMultiplierY,
                 float vecInertiaMultiplierZ,
                 float fDriveBiasFront,
                 int nInitialDriveGears,
                 float fInitialDriveForce,
                 float fDriveInertia,
                 float fClutchChangeRateScaleUpShift,
                 float fClutchChangeRateScaleDownShift,
                 float fInitialDriveMaxFlatVel,
                 float fBrakeForce,
                 float fBrakeBiasFront,
                 float fHandBrakeForce,
                 float fSteeringLock,
                 float fTractionCurveMax,
                 float fTractionCurveMin,
                 float fTractionCurveLateral,
                 float fTractionSpringDeltaMax,
                 float fLowSpeedTractionLossMult,
                 float fCamberStiffness,
                 float fTractionBiasFront,
                 float fTractionLossMult,
                 float fSuspensionForce,
                 float fSuspensionCompDamp,
                 float fSuspensionReboundDamp,
                 float fSuspensionUpperLimit,
                 float fSuspensionLowerLimit,
                 float fSuspensionRaise,
                 float fSuspensionBiasFront,
                 float fAntiRollBarForce,
                 float fAntiRollBarBiasFront,
                 float fRollCentreHeightFront,
                 float fRollCentreHeightRear,
                 float fCollisionDamageMult,
                 float fWeaponDamageMult,
                 float fDeformationDamageMult,
                 float fEngineDamageMult,
                 float fPetrolTankVolume,
                 float fOilVolume,
                 float fSeatOffsetDistX,
                 float fSeatOffsetDistY,
                 float fSeatOffsetDistZ) {
	if (fMass != disableVal)
		setHandlingValue(vehicle, hOffsets.fMass, fMass);

	if (fInitialDragCoeff != disableVal)
		setHandlingValue(vehicle, hOffsets.fInitialDragCoeff, fInitialDragCoeff / 10000.0f);

	if (fPercentSubmerged != disableVal) {
		setHandlingValue(vehicle, hOffsets.fPercentSubmerged, fPercentSubmerged);
		setHandlingValue(vehicle, hOffsets.fSubmergedRatio, 100.0f / fPercentSubmerged);
	}

	if (vecCentreOfMassOffsetX != disableVal)
		setHandlingValue(vehicle, hOffsets.vecCentreOfMass.X, vecCentreOfMassOffsetX);

	if (vecCentreOfMassOffsetY != disableVal)
		setHandlingValue(vehicle, hOffsets.vecCentreOfMass.Y, vecCentreOfMassOffsetY);

	if (vecCentreOfMassOffsetZ != disableVal)
		setHandlingValue(vehicle, hOffsets.vecCentreOfMass.Z, vecCentreOfMassOffsetZ);

	if (vecInertiaMultiplierX != disableVal)
		setHandlingValue(vehicle, hOffsets.vecInertiaMultiplier.X, vecInertiaMultiplierX);

	if (vecInertiaMultiplierY != disableVal)
		setHandlingValue(vehicle, hOffsets.vecInertiaMultiplier.Y, vecInertiaMultiplierY);

	if (vecInertiaMultiplierZ != disableVal)
		setHandlingValue(vehicle, hOffsets.vecInertiaMultiplier.Z, vecInertiaMultiplierZ);

	if (fDriveBiasFront != disableVal) {
		if (fDriveBiasFront == 1.0f) {
			setHandlingValue(vehicle, hOffsets.fDriveBiasRear, 0.0f);
			setHandlingValue(vehicle, hOffsets.fDriveBiasFront, 1.0f);
		}
		else if (fDriveBiasFront == 0.0f) {
			setHandlingValue(vehicle, hOffsets.fDriveBiasRear, 1.0f);
			setHandlingValue(vehicle, hOffsets.fDriveBiasFront, 0.0f);
		}
		else {
			setHandlingValue(vehicle, hOffsets.fDriveBiasRear, 2.0f*(1.0f - (fDriveBiasFront)));
			setHandlingValue(vehicle, hOffsets.fDriveBiasFront, fDriveBiasFront * 2.0f);
		}
	}

	if (nInitialDriveGears != disableVal)
		setHandlingValue(vehicle, hOffsets.nInitialDriveGears, nInitialDriveGears);

	if (fInitialDriveForce != disableVal)
		setHandlingValue(vehicle, hOffsets.fInitialDriveForce, fInitialDriveForce);

	if (fDriveInertia != disableVal)
		setHandlingValue(vehicle, hOffsets.fDriveInertia, fDriveInertia);

	if (fClutchChangeRateScaleUpShift != disableVal)
		setHandlingValue(vehicle, hOffsets.fClutchChangeRateScaleUpShift, fClutchChangeRateScaleUpShift);

	if (fClutchChangeRateScaleDownShift != disableVal)
		setHandlingValue(vehicle, hOffsets.fClutchChangeRateScaleDownShift, fClutchChangeRateScaleDownShift);

	if (fInitialDriveMaxFlatVel != disableVal) {
		setHandlingValue(vehicle, hOffsets.fInitialDriveMaxFlatVel, fInitialDriveMaxFlatVel / 3.6f);
		setHandlingValue(vehicle, hOffsets.fDriveMaxFlatVel, fInitialDriveMaxFlatVel / 3.0f);
	}

	if (fBrakeForce != disableVal)
		setHandlingValue(vehicle, hOffsets.fBrakeForce, fBrakeForce);

	if (fBrakeBiasFront != disableVal) {
		setHandlingValue(vehicle, hOffsets.fBrakeBiasRear, 2.0f*(1.0f - (fBrakeBiasFront)));
		setHandlingValue(vehicle, hOffsets.fBrakeBiasFront, fBrakeBiasFront * 2.0f);
	}

	if (fHandBrakeForce != disableVal)
		setHandlingValue(vehicle, hOffsets.fHandBrakeForce, fHandBrakeForce);

	if (fSteeringLock != disableVal) {
		setHandlingValue(vehicle, hOffsets.fSteeringLock, fSteeringLock * 0.017453292f);
		setHandlingValue(vehicle, hOffsets.fSteeringLockRatio, 1.0f / (fSteeringLock * 0.017453292f));
	}

	if (fTractionCurveMax != disableVal) {
		setHandlingValue(vehicle, hOffsets.fTractionCurveMax, fTractionCurveMax);
		if (fTractionCurveMax == 0.0f)
			setHandlingValue(vehicle, hOffsets.fTractionCurveMaxRatio, 100000000.000000f);
		else 
			setHandlingValue(vehicle, hOffsets.fTractionCurveMaxRatio, 1.0f/ fTractionCurveMax);
	}

	if (fTractionCurveMin != disableVal) {
		setHandlingValue(vehicle, hOffsets.fTractionCurveMin, fTractionCurveMin);
	}

	{
		float temp_fTractionCurveMax = getHandlingValue<float>(vehicle, hOffsets.fTractionCurveMax);
		float temp_ftractionCurveMin = getHandlingValue<float>(vehicle, hOffsets.fTractionCurveMin);
		if (temp_fTractionCurveMax <= temp_ftractionCurveMin) {
			setHandlingValue(vehicle, hOffsets.fTractionCurveRatio, 100000000.000000f);
		}
		else {
			setHandlingValue(vehicle, hOffsets.fTractionCurveRatio,
				1.0f / (temp_fTractionCurveMax - temp_ftractionCurveMin));
		}
	}

	

	if (fTractionCurveLateral != disableVal) { 
		setHandlingValue(vehicle, hOffsets.fTractionCurveLateral, fTractionCurveLateral * 0.017453292f);
		setHandlingValue(vehicle, hOffsets.fTractionCurveLateralRatio, 1.0f/(fTractionCurveLateral * 0.017453292f));
	}

	if (fTractionSpringDeltaMax != disableVal) {
		setHandlingValue(vehicle, hOffsets.fTractionSpringDeltaMax, fTractionSpringDeltaMax);
		setHandlingValue(vehicle, hOffsets.fTractionSpringDeltaMaxRatio, 1.0f/fTractionSpringDeltaMax);

	}

	if (fLowSpeedTractionLossMult != disableVal)
		setHandlingValue(vehicle, hOffsets.fLowSpeedTractionLossMult, fLowSpeedTractionLossMult);

	if (fCamberStiffness != disableVal)
		setHandlingValue(vehicle, hOffsets.fCamberStiffness, fCamberStiffness);

	if (fTractionBiasFront != disableVal) {
		setHandlingValue(vehicle, hOffsets.fTractionBiasRear, 2.0f*(1.0f - (fTractionBiasFront)));
		setHandlingValue(vehicle, hOffsets.fTractionBiasFront, fTractionBiasFront * 2.0f);
	}

	if (fTractionLossMult != disableVal)
		setHandlingValue(vehicle, hOffsets.fTractionLossMult, fTractionLossMult);

	if (fSuspensionForce != disableVal)
		setHandlingValue(vehicle, hOffsets.fSuspensionForce, fSuspensionForce);

	if (fSuspensionCompDamp != disableVal)
		setHandlingValue(vehicle, hOffsets.fSuspensionCompDamp, fSuspensionCompDamp / 10.0f);

	if (fSuspensionReboundDamp != disableVal)
		setHandlingValue(vehicle, hOffsets.fSuspensionReboundDamp, fSuspensionReboundDamp / 10.0f);

	if (fSuspensionUpperLimit != disableVal)
		setHandlingValue(vehicle, hOffsets.fSuspensionUpperLimit, fSuspensionUpperLimit);

	if (fSuspensionLowerLimit != disableVal)
		setHandlingValue(vehicle, hOffsets.fSuspensionLowerLimit, fSuspensionLowerLimit);

	if (fSuspensionRaise != disableVal)
		setHandlingValue(vehicle, hOffsets.fSuspensionRaise, fSuspensionRaise);

	if (fSuspensionBiasFront != disableVal) {
		setHandlingValue(vehicle, hOffsets.fSuspensionBiasRear, 2.0f*(1.0f - (fSuspensionBiasFront)));
		setHandlingValue(vehicle, hOffsets.fSuspensionBiasFront, fSuspensionBiasFront * 2.0f);
	}

	if (fAntiRollBarForce != disableVal)
		setHandlingValue(vehicle, hOffsets.fAntiRollBarForce, fAntiRollBarForce);

	if (fAntiRollBarBiasFront != disableVal) {
		setHandlingValue(vehicle, hOffsets.fAntiRollBarBiasRear, 2.0f*(1.0f - (fAntiRollBarBiasFront)));
		setHandlingValue(vehicle, hOffsets.fAntiRollBarBiasFront, fAntiRollBarBiasFront * 2.0f);
	}

	if (fRollCentreHeightFront != disableVal)
		setHandlingValue(vehicle, hOffsets.fRollCentreHeightFront, fRollCentreHeightFront);

	if (fRollCentreHeightRear != disableVal)
		setHandlingValue(vehicle, hOffsets.fRollCentreHeightRear, fRollCentreHeightRear);

	if (fCollisionDamageMult != disableVal)
		setHandlingValue(vehicle, hOffsets.fCollisionDamageMult, fCollisionDamageMult);

	if (fWeaponDamageMult != disableVal)
		setHandlingValue(vehicle, hOffsets.fWeaponDamageMult, fWeaponDamageMult);

	if (fDeformationDamageMult != disableVal)
		setHandlingValue(vehicle, hOffsets.fDeformationDamageMult, fDeformationDamageMult);

	if (fEngineDamageMult != disableVal)
		setHandlingValue(vehicle, hOffsets.fEngineDamageMult, fEngineDamageMult);

	if (fPetrolTankVolume != disableVal)
		setHandlingValue(vehicle, hOffsets.fPetrolTankVolume, fPetrolTankVolume);

	if (fOilVolume != disableVal)
		setHandlingValue(vehicle, hOffsets.fOilVolume, fOilVolume);

	if (fSeatOffsetDistX != disableVal)
		setHandlingValue(vehicle, hOffsets.fSeatOffsetDistX, fSeatOffsetDistX);

	if (fSeatOffsetDistY != disableVal)
		setHandlingValue(vehicle, hOffsets.fSeatOffsetDistY, fSeatOffsetDistY);

	if (fSeatOffsetDistZ != disableVal)
		setHandlingValue(vehicle, hOffsets.fSeatOffsetDistZ, fSeatOffsetDistZ);
}


void readMemorytoLog() {
	std::stringstream vehName;
	vehName << "VEHICLE: " << VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
	Logger logger(LOGFILE);
	logger.Write(vehName.str());
	logger.Write("fMass = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fMass)));
	logger.Write("fInitialDragCoeff = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fInitialDragCoeff)*10000.0f)); // * 10000
	logger.Write("fPercentSubmerged = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fPercentSubmerged)));

	logger.Write("vecCentreOfMassOffsetX = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.vecCentreOfMass.X)));
	logger.Write("vecCentreOfMassOffsetY = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.vecCentreOfMass.Y)));
	logger.Write("vecCentreOfMassOffsetZ = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.vecCentreOfMass.Z)));

	logger.Write("vecInertiaMultiplierX = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.vecInertiaMultiplier.X)));
	logger.Write("vecInertiaMultiplierY = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.vecInertiaMultiplier.Y)));
	logger.Write("vecInertiaMultiplierZ = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.vecInertiaMultiplier.Z)));

	logger.Write("fDriveBiasFront = " + std::to_string(1.0f - getHandlingValue<float>(vehicle, hOffsets.fDriveBiasRear)/2.0f)); // 1.0f - (val / 2.0f)

	logger.Write("nInitialDriveGears = " + std::to_string(getHandlingValue<int8_t>(vehicle, hOffsets.nInitialDriveGears))); // int
	logger.Write("fInitialDriveForce = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fInitialDriveForce)));
	logger.Write("fDriveInertia = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fDriveInertia)));
	logger.Write("fClutchChangeRateScaleUpShift = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fClutchChangeRateScaleUpShift)));
	logger.Write("fClutchChangeRateScaleDownShift = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fClutchChangeRateScaleDownShift)));
	logger.Write("fInitialDriveMaxFlatVel = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fInitialDriveMaxFlatVel)*3.6f)); // m/s -> kph

	logger.Write("fBrakeForce = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fBrakeForce)));
	logger.Write("fBrakeBiasFront = " + std::to_string(1.0f - getHandlingValue<float>(vehicle, hOffsets.fBrakeBiasRear) / 2.0f)); // 1.0f - (val / 2.0f)
	logger.Write("fHandBrakeForce = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fHandBrakeForce)));

	logger.Write("fSteeringLock = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSteeringLock)*57.2957795131f)); // rad -> deg
	logger.Write("fTractionCurveMax = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fTractionCurveMax)));
	logger.Write("fTractionCurveMin = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fTractionCurveMin)));
	logger.Write("fTractionCurveLateral = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fTractionCurveLateral)*57.2957795131f)); // rad -> deg
	logger.Write("fTractionSpringDeltaMax = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fTractionSpringDeltaMax)));
	logger.Write("fLowSpeedTractionLossMult = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fLowSpeedTractionLossMult)));
	logger.Write("fCamberStiffness = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fCamberStiffness)));
	logger.Write("fTractionBiasFront = " + std::to_string(1.0f - getHandlingValue<float>(vehicle, hOffsets.fTractionBiasRear) / 2.0f)); // 1.0f - (val / 2.0f)
	logger.Write("fTractionLossMult = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fTractionLossMult)));

	logger.Write("fSuspensionForce = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSuspensionForce)));
	logger.Write("fSuspensionCompDamp = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSuspensionCompDamp)*10.0f)); // * 10
	logger.Write("fSuspensionReboundDamp = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSuspensionReboundDamp)*10.0f)); // * 10
	logger.Write("fSuspensionUpperLimit = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSuspensionUpperLimit)));
	logger.Write("fSuspensionLowerLimit = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSuspensionLowerLimit)));
	logger.Write("fSuspensionRaise = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSuspensionRaise)));
	logger.Write("fSuspensionBiasFront = " + std::to_string(1.0f - getHandlingValue<float>(vehicle, hOffsets.fSuspensionBiasRear) / 2.0f)); // 1.0f - (val / 2.0f)
	logger.Write("fAntiRollBarForce = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fAntiRollBarForce)));
	logger.Write("fAntiRollBarBiasFront = " + std::to_string(1.0f - getHandlingValue<float>(vehicle, hOffsets.fAntiRollBarBiasRear) / 2.0f)); // 1.0f - (val / 2.0f)
	logger.Write("fRollCentreHeightFront = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fRollCentreHeightFront)));
	logger.Write("fRollCentreHeightRear = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fRollCentreHeightRear)));
	logger.Write("fCollisionDamageMult = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fCollisionDamageMult)));
	logger.Write("fWeaponDamageMult = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fWeaponDamageMult)));
	logger.Write("fDeformationDamageMult = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fDeformationDamageMult)));
	logger.Write("fEngineDamageMult = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fEngineDamageMult)));
	logger.Write("fPetrolTankVolume = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fPetrolTankVolume)));
	logger.Write("fOilVolume = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fOilVolume)));
	logger.Write("fSeatOffsetDistX = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSeatOffsetDistX)));
	logger.Write("fSeatOffsetDistY = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSeatOffsetDistY)));
	logger.Write("fSeatOffsetDistZ = " + std::to_string(getHandlingValue<float>(vehicle, hOffsets.fSeatOffsetDistZ)));
	logger.Write("nMonetaryValue = " + std::to_string(getHandlingValue<int>(vehicle, hOffsets.nMonetaryValue)));
	
	// Oh bother
	std::stringstream ssModelFlags;
	ssModelFlags << std::hex << getHandlingValue<DWORD>(vehicle, hOffsets.dwStrModelFlags);
	logger.Write("strModelFlags = " + ssModelFlags.str());

	std::stringstream ssHandlingFlags;
	ssHandlingFlags << std::hex << getHandlingValue<DWORD>(vehicle, hOffsets.dwStrHandlingFlags);
	logger.Write("strHandlingFlags = " + ssHandlingFlags.str());

	std::stringstream ssDamageFlags;
	ssDamageFlags << std::hex << getHandlingValue<DWORD>(vehicle, hOffsets.dwStrDamageFlags);
	logger.Write("strDamageFlags = " + ssDamageFlags.str());
	
	logger.Write("------------------------------");
}

void readXMLFile() {
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile("./RTHandlingEditor.meta");
	int err2 = 0;


	std::stringstream handlingText;
	float fMass = disableVal;
	float fInitialDragCoeff = disableVal;
	float fPercentSubmerged = disableVal;
	float vecCentreOfMassOffsetX = disableVal;
	float vecCentreOfMassOffsetY = disableVal;
	float vecCentreOfMassOffsetZ = disableVal;
	float vecInertiaMultiplierX = disableVal;
	float vecInertiaMultiplierY = disableVal;
	float vecInertiaMultiplierZ = disableVal;
	float fDriveBiasFront = disableVal;
	int   nInitialDriveGears = static_cast<int>(disableVal);
	float fInitialDriveForce = disableVal;
	float fDriveInertia = disableVal;
	float fClutchChangeRateScaleUpShift = disableVal;
	float fClutchChangeRateScaleDownShift = disableVal;
	float fInitialDriveMaxFlatVel = disableVal;
	float fBrakeForce = disableVal;
	float fBrakeBiasFront = disableVal;
	float fHandBrakeForce = disableVal;
	float fSteeringLock = disableVal;
	float fTractionCurveMax = disableVal;
	float fTractionCurveMin = disableVal;
	float fTractionCurveLateral = disableVal;
	float fTractionSpringDeltaMax = disableVal;
	float fLowSpeedTractionLossMult = disableVal;
	float fCamberStiffness = disableVal;
	float fTractionBiasFront = disableVal;
	float fTractionLossMult = disableVal;
	float fSuspensionForce = disableVal;
	float fSuspensionCompDamp = disableVal;
	float fSuspensionReboundDamp = disableVal;
	float fSuspensionUpperLimit = disableVal;
	float fSuspensionLowerLimit = disableVal;
	float fSuspensionRaise = disableVal;
	float fSuspensionBiasFront = disableVal;
	float fAntiRollBarForce = disableVal;
	float fAntiRollBarBiasFront = disableVal;
	float fRollCentreHeightFront = disableVal;
	float fRollCentreHeightRear = disableVal;
	float fCollisionDamageMult = disableVal;
	float fWeaponDamageMult = disableVal;
	float fDeformationDamageMult = disableVal;
	float fEngineDamageMult = disableVal;
	float fPetrolTankVolume = disableVal;
	float fOilVolume = disableVal;
	float fSeatOffsetDistX = disableVal;
	float fSeatOffsetDistY = disableVal;
	float fSeatOffsetDistZ = disableVal;
	int nMonetaryValue = disableVal;
	DWORD strModelFlags = disableVal;	
	DWORD strHandlingFlags = disableVal;
	DWORD strDamageflags = disableVal;
	DWORD dwAIHandlingHash = disableVal;

	if (err != XML_SUCCESS) {
		showNotification("RTHandlingEditor: Can't load RTHandlingEditor.meta");
		return;
	}
	char *xmlGenericFormatError = "RTHandlingEditor: RTHandlingEditor.meta read error";
	XMLNode * pRoot = doc.FirstChild();
	if (!pRoot) {
		showNotification(xmlGenericFormatError);
		return;
	}

	XMLElement * pElement = pRoot->FirstChildElement("handlingName");
	if (pElement) {
		handlingText << "RTHandlingEditor: " << pElement->GetText();
	}
	
	// Error checking not done because I'm lazy
	pRoot->FirstChildElement("fMass")->QueryAttribute("value", &fMass);
	pRoot->FirstChildElement("fInitialDragCoeff")->QueryAttribute("value", &fInitialDragCoeff);
	pRoot->FirstChildElement("fPercentSubmerged")->QueryAttribute("value", &fPercentSubmerged);
	pRoot->FirstChildElement("vecCentreOfMassOffset")->QueryAttribute("x", &vecCentreOfMassOffsetX);
	pRoot->FirstChildElement("vecCentreOfMassOffset")->QueryAttribute("y", &vecCentreOfMassOffsetY);
	pRoot->FirstChildElement("vecCentreOfMassOffset")->QueryAttribute("z", &vecCentreOfMassOffsetZ);
	pRoot->FirstChildElement("vecInertiaMultiplier")->QueryAttribute("x", &vecInertiaMultiplierX);
	pRoot->FirstChildElement("vecInertiaMultiplier")->QueryAttribute("y", &vecInertiaMultiplierY);
	pRoot->FirstChildElement("vecInertiaMultiplier")->QueryAttribute("z", &vecInertiaMultiplierZ);
	pRoot->FirstChildElement("fDriveBiasFront")->QueryAttribute("value", &fDriveBiasFront);
	pRoot->FirstChildElement("nInitialDriveGears")->QueryAttribute("value", &nInitialDriveGears);
	pRoot->FirstChildElement("fInitialDriveForce")->QueryAttribute("value", &fInitialDriveForce);
	pRoot->FirstChildElement("fDriveInertia")->QueryAttribute("value", &fDriveInertia);
	pRoot->FirstChildElement("fClutchChangeRateScaleUpShift")->QueryAttribute("value", &fClutchChangeRateScaleUpShift);
	pRoot->FirstChildElement("fClutchChangeRateScaleDownShift")->QueryAttribute("value", &fClutchChangeRateScaleDownShift);
	pRoot->FirstChildElement("fInitialDriveMaxFlatVel")->QueryAttribute("value", &fInitialDriveMaxFlatVel);
	pRoot->FirstChildElement("fBrakeForce")->QueryAttribute("value", &fBrakeForce);
	pRoot->FirstChildElement("fBrakeBiasFront")->QueryAttribute("value", &fBrakeBiasFront);
	pRoot->FirstChildElement("fHandBrakeForce")->QueryAttribute("value", &fHandBrakeForce);
	pRoot->FirstChildElement("fSteeringLock")->QueryAttribute("value", &fSteeringLock);
	pRoot->FirstChildElement("fTractionCurveMax")->QueryAttribute("value", &fTractionCurveMax);
	pRoot->FirstChildElement("fTractionCurveMin")->QueryAttribute("value", &fTractionCurveMin);
	pRoot->FirstChildElement("fTractionCurveLateral")->QueryAttribute("value", &fTractionCurveLateral);
	pRoot->FirstChildElement("fTractionSpringDeltaMax")->QueryAttribute("value", &fTractionSpringDeltaMax);
	pRoot->FirstChildElement("fLowSpeedTractionLossMult")->QueryAttribute("value", &fLowSpeedTractionLossMult);
	pRoot->FirstChildElement("fCamberStiffnesss")->QueryAttribute("value", &fCamberStiffness);
	pRoot->FirstChildElement("fTractionBiasFront")->QueryAttribute("value", &fTractionBiasFront);
	pRoot->FirstChildElement("fTractionLossMult")->QueryAttribute("value", &fTractionLossMult);
	pRoot->FirstChildElement("fSuspensionForce")->QueryAttribute("value", &fSuspensionForce);
	pRoot->FirstChildElement("fSuspensionCompDamp")->QueryAttribute("value", &fSuspensionCompDamp);
	pRoot->FirstChildElement("fSuspensionReboundDamp")->QueryAttribute("value", &fSuspensionReboundDamp);
	pRoot->FirstChildElement("fSuspensionUpperLimit")->QueryAttribute("value", &fSuspensionUpperLimit);
	pRoot->FirstChildElement("fSuspensionLowerLimit")->QueryAttribute("value", &fSuspensionLowerLimit);
	pRoot->FirstChildElement("fSuspensionRaise")->QueryAttribute("value", &fSuspensionRaise);
	pRoot->FirstChildElement("fSuspensionBiasFront")->QueryAttribute("value", &fSuspensionBiasFront);
	pRoot->FirstChildElement("fAntiRollBarForce")->QueryAttribute("value", &fAntiRollBarForce);
	pRoot->FirstChildElement("fAntiRollBarBiasFront")->QueryAttribute("value", &fAntiRollBarBiasFront);
	pRoot->FirstChildElement("fRollCentreHeightFront")->QueryAttribute("value", &fRollCentreHeightFront);
	pRoot->FirstChildElement("fRollCentreHeightRear")->QueryAttribute("value", &fRollCentreHeightRear);
	pRoot->FirstChildElement("fCollisionDamageMult")->QueryAttribute("value", &fCollisionDamageMult);
	pRoot->FirstChildElement("fWeaponDamageMult")->QueryAttribute("value", &fWeaponDamageMult);
	pRoot->FirstChildElement("fDeformationDamageMult")->QueryAttribute("value", &fDeformationDamageMult);
	pRoot->FirstChildElement("fEngineDamageMult")->QueryAttribute("value", &fEngineDamageMult);
	pRoot->FirstChildElement("fPetrolTankVolume")->QueryAttribute("value", &fPetrolTankVolume);
	pRoot->FirstChildElement("fOilVolume")->QueryAttribute("value", &fOilVolume);
	pRoot->FirstChildElement("fSeatOffsetDistX")->QueryAttribute("value", &fSeatOffsetDistX);
	pRoot->FirstChildElement("fSeatOffsetDistY")->QueryAttribute("value", &fSeatOffsetDistY);
	pRoot->FirstChildElement("fSeatOffsetDistZ")->QueryAttribute("value", &fSeatOffsetDistZ);

	setHandling(
		fMass,
		fInitialDragCoeff,
		fPercentSubmerged,
		vecCentreOfMassOffsetX,
		vecCentreOfMassOffsetY,
		vecCentreOfMassOffsetZ,
		vecInertiaMultiplierX,
		vecInertiaMultiplierY,
		vecInertiaMultiplierZ,
		fDriveBiasFront,
		nInitialDriveGears,
		fInitialDriveForce,
		fDriveInertia,
		fClutchChangeRateScaleUpShift,
		fClutchChangeRateScaleDownShift,
		fInitialDriveMaxFlatVel,
		fBrakeForce,
		fBrakeBiasFront,
		fHandBrakeForce,
		fSteeringLock,
		fTractionCurveMax,
		fTractionCurveMin,
		fTractionCurveLateral,
		fTractionSpringDeltaMax,
		fLowSpeedTractionLossMult,
		fCamberStiffness,
		fTractionBiasFront,
		fTractionLossMult,
		fSuspensionForce,
		fSuspensionCompDamp,
		fSuspensionReboundDamp,
		fSuspensionUpperLimit,
		fSuspensionLowerLimit,
		fSuspensionRaise,
		fSuspensionBiasFront,
		fAntiRollBarForce,
		fAntiRollBarBiasFront,
		fRollCentreHeightFront,
		fRollCentreHeightRear,
		fCollisionDamageMult,
		fWeaponDamageMult,
		fDeformationDamageMult,
		fEngineDamageMult,
		fPetrolTankVolume,
		fOilVolume,
		fSeatOffsetDistX,
		fSeatOffsetDistY,
		fSeatOffsetDistZ);

	showNotification(const_cast<char *>(handlingText.str().c_str()));
}

void readINIFile() {
	INIReader reader(SETTINGSFILE);
	Logger logger(LOGFILE);

	if (reader.ParseError() < 0) {
		logger.Write("Can't load RTHandlingEditor.ini");
		showNotification("RTHandlingEditor: Can't load RTHandlingEditor.ini");
		return;
	}
	
#pragma warning( push )
#pragma warning( disable: 4244 )

	float fMass = reader.GetReal("handling","fMass",disableVal);
	float fInitialDragCoeff = reader.GetReal("handling", "fInitialDragCoeff", disableVal); // * 10000
	float fPercentSubmerged = reader.GetReal("handling", "fPercentSubmerged", disableVal);
	float vecCentreOfMassOffsetX = reader.GetReal("handling", "vecCentreOfMassOffsetX", disableVal);
	float vecCentreOfMassOffsetY = reader.GetReal("handling", "vecCentreOfMassOffsetY", disableVal);
	float vecCentreOfMassOffsetZ = reader.GetReal("handling", "vecCentreOfMassOffsetZ", disableVal);
	float vecInertiaMultiplierX = reader.GetReal("handling", "vecInertiaMultiplierX", disableVal);
	float vecInertiaMultiplierY = reader.GetReal("handling", "vecInertiaMultiplierY", disableVal);
	float vecInertiaMultiplierZ = reader.GetReal("handling", "vecInertiaMultiplierZ", disableVal);
	float fDriveBiasFront = reader.GetReal("handling", "fDriveBiasFront", disableVal); // !!!!!
	int   nInitialDriveGears = (int)reader.GetInteger("handling", "nInitialDriveGears", (long)disableVal);
	float fInitialDriveForce = reader.GetReal("handling", "fInitialDriveForce", disableVal);
	float fDriveInertia = reader.GetReal("handling", "fDriveInertia", disableVal);
	float fClutchChangeRateScaleUpShift = reader.GetReal("handling", "fClutchChangeRateScaleUpShift", disableVal);
	float fClutchChangeRateScaleDownShift = reader.GetReal("handling", "fClutchChangeRateScaleDownShift", disableVal);
	float fInitialDriveMaxFlatVel = reader.GetReal("handling", "fInitialDriveMaxFlatVel", disableVal); // kph -> m/s
	float fBrakeForce = reader.GetReal("handling", "fBrakeForce", disableVal);
	float fBrakeBiasFront = reader.GetReal("handling", "fBrakeBiasFront", disableVal); // !!!!!
	float fHandBrakeForce = reader.GetReal("handling", "fHandBrakeForce", disableVal);
	float fSteeringLock = reader.GetReal("handling", "fSteeringLock", disableVal); // deg -> rad
	float fTractionCurveMax = reader.GetReal("handling", "fTractionCurveMax", disableVal);
	float fTractionCurveMin = reader.GetReal("handling", "fTractionCurveMin", disableVal);
	float fTractionCurveLateral = reader.GetReal("handling", "fTractionCurveLateral", disableVal); // deg -> rad
	float fTractionSpringDeltaMax = reader.GetReal("handling", "fTractionSpringDeltaMax", disableVal);
	float fLowSpeedTractionLossMult = reader.GetReal("handling", "fLowSpeedTractionLossMult", disableVal);
	float fCamberStiffness = reader.GetReal("handling", "fCamberStiffness", disableVal);
	float fTractionBiasFront = reader.GetReal("handling", "fTractionBiasFront", disableVal); // !!!!!
	float fTractionLossMult = reader.GetReal("handling", "fTractionLossMult", disableVal);
	float fSuspensionForce = reader.GetReal("handling", "fSuspensionForce", disableVal);
	float fSuspensionCompDamp = reader.GetReal("handling", "fSuspensionCompDamp", disableVal); // *10
	float fSuspensionReboundDamp = reader.GetReal("handling", "fSuspensionReboundDamp", disableVal); // *10
	float fSuspensionUpperLimit = reader.GetReal("handling", "fSuspensionUpperLimit", disableVal);
	float fSuspensionLowerLimit = reader.GetReal("handling", "fSuspensionLowerLimit", disableVal);
	float fSuspensionRaise = reader.GetReal("handling", "fSuspensionRaise", disableVal);
	float fSuspensionBiasFront = reader.GetReal("handling", "fSuspensionBiasFront", disableVal); // !!!!!
	float fAntiRollBarForce = reader.GetReal("handling", "fAntiRollBarForce", disableVal);
	float fAntiRollBarBiasFront = reader.GetReal("handling", "fAntiRollBarBiasFront", disableVal); // !!!!!
	float fRollCentreHeightFront = reader.GetReal("handling", "fRollCentreHeightFront", disableVal);
	float fRollCentreHeightRear = reader.GetReal("handling", "fRollCentreHeightRear", disableVal);
	float fCollisionDamageMult = reader.GetReal("handling", "fCollisionDamageMult", disableVal);
	float fWeaponDamageMult = reader.GetReal("handling", "fWeaponDamageMult", disableVal);
	float fDeformationDamageMult = reader.GetReal("handling", "fDeformationDamageMult", disableVal);
	float fEngineDamageMult = reader.GetReal("handling", "fEngineDamageMult", disableVal);
	float fPetrolTankVolume = reader.GetReal("handling", "fPetrolTankVolume", disableVal);
	float fOilVolume = reader.GetReal("handling", "fOilVolume", disableVal);
	float fSeatOffsetDistX = reader.GetReal("handling", "fSeatOffsetDistX", disableVal);
	float fSeatOffsetDistY = reader.GetReal("handling", "fSeatOffsetDistY", disableVal);
	float fSeatOffsetDistZ = reader.GetReal("handling", "fSeatOffsetDistZ", disableVal);

#pragma warning ( pop )

	setHandling(
		fMass,
		fInitialDragCoeff,
		fPercentSubmerged,
		vecCentreOfMassOffsetX,
		vecCentreOfMassOffsetY,
		vecCentreOfMassOffsetZ,
		vecInertiaMultiplierX,
		vecInertiaMultiplierY,
		vecInertiaMultiplierZ,
		fDriveBiasFront,
		nInitialDriveGears,
		fInitialDriveForce,
		fDriveInertia,
		fClutchChangeRateScaleUpShift,
		fClutchChangeRateScaleDownShift,
		fInitialDriveMaxFlatVel,
		fBrakeForce,
		fBrakeBiasFront,
		fHandBrakeForce,
		fSteeringLock,
		fTractionCurveMax,
		fTractionCurveMin,
		fTractionCurveLateral,
		fTractionSpringDeltaMax,
		fLowSpeedTractionLossMult,
		fCamberStiffness,
		fTractionBiasFront,
		fTractionLossMult,
		fSuspensionForce,
		fSuspensionCompDamp,
		fSuspensionReboundDamp,
		fSuspensionUpperLimit,
		fSuspensionLowerLimit,
		fSuspensionRaise,
		fSuspensionBiasFront,
		fAntiRollBarForce,
		fAntiRollBarBiasFront,
		fRollCentreHeightFront,
		fRollCentreHeightRear,
		fCollisionDamageMult,
		fWeaponDamageMult,
		fDeformationDamageMult,
		fEngineDamageMult,
		fPetrolTankVolume,
		fOilVolume,
		fSeatOffsetDistX,
		fSeatOffsetDistY,
		fSeatOffsetDistZ);
	showNotification("RTHandlingEditor: Loaded RTHandlingEditor.ini");
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

	GetPrivateProfileStringA("key", "meta", "K", kbKeyBuffer, 24, SETTINGSFILE);
	int metaKey = str2key(kbKeyBuffer);

	float posX = GetPrivateProfileIntA("util", "infox", 1, SETTINGSFILE)  / 100.0f;
	float posY = GetPrivateProfileIntA("util", "infoy", 30, SETTINGSFILE) / 100.0f;
	float size = GetPrivateProfileIntA("util", "infosz", 100, SETTINGSFILE) / 100.0f;
	
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

	showPhysicsValues(velocities, accelValsAvg, posX, posY, size);




		
	/*std::stringstream ss_fBrakeBiasFront;
	ss_fBrakeBiasFront << "fBrakeBiasFront = " << get_fBrakeBiasFront(vehicle);
	showText(0.2, 0.2, 1.0, ss_fBrakeBiasFront.str().c_str());

	std::stringstream ss_fTractionBiasFront;
	ss_fTractionBiasFront << "fTractionBiasFront = " << get_fTractionBiasFront(vehicle);
	showText(0.2, 0.3, 1.0, ss_fTractionBiasFront.str().c_str());*/
	//logger.Write(" = " + std::to_string());
	if (IsKeyJustUp(logKey)) {
		readMemorytoLog();
		showNotification("RTHandlingEditor: Saved to RTHandlingEditor.log");
	}

	if (IsKeyJustUp(metaKey)) {
		readXMLFile();
	}

	if (IsKeyJustUp(readKey)) {
		readINIFile();

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
