#include "HandlingFlags.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"
#include "Constants.h"

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

struct SVersion {
    int Major;
    int Minor;

    bool operator>(const SVersion& other) {
        return Major > other.Major ||
            Major == other.Major && Minor > other.Minor;
    }
};

struct SAllFlags {
    SVersion Version;
    std::vector<Flags::SFlag> ModelFlags;
    std::vector<Flags::SFlag> HandlingFlags;
    std::vector<Flags::SFlag> AdvancedFlags;
};

namespace {
    SAllFlags g_AllFlags;
}

const std::vector<Flags::SFlag>& Flags::GetModelFlags() {
    return g_AllFlags.ModelFlags;
}

const std::vector<Flags::SFlag>& Flags::GetHandlingFlags() {
    return g_AllFlags.HandlingFlags;
}

const std::vector<Flags::SFlag>& Flags::GetAdvancedFlags() {
    return g_AllFlags.AdvancedFlags;
}

SAllFlags ReadJson(const std::string& jsonContent) {
    SAllFlags allFlags{};
    allFlags.Version = { 0, 0 };

    try {
        json flagsDoc = json::parse(jsonContent);

        auto version = flagsDoc.at("version").get<std::string>();
        logger.Write(INFO, fmt::format("[Flags] Version '{}'", version));

        int major, minor;

        // Parse version.
        int argsScanned = sscanf_s(version.c_str(), "%d.%d", &major, &minor);
        if (argsScanned != 2) {
            logger.Write(ERROR, fmt::format("[Flags] File version unknown: '{}'", version));
            return {};
        }
        allFlags.Version = { major, minor };

        auto flags = flagsDoc.at("flags");
        auto modelFlags = flags.at("strModelFlags");
        auto handlingFlags = flags.at("strHandlingFlags");
        auto advancedFlags = flags.at("strAdvancedFlags");

        if (modelFlags.size() == 32) {
            allFlags.ModelFlags.reserve(32);
            for (const auto& docFlag : modelFlags) {
                std::string name = docFlag.at("name").get<std::string>();
                std::string description = docFlag.at("description").get<std::string>();
                allFlags.ModelFlags.emplace_back(Flags::SFlag{ name, description });
            }
        }
        else {
            logger.Write(ERROR, "[Flags] Expected 32 items in strModelFlags");
        }

        if (handlingFlags.size() == 32) {
            allFlags.HandlingFlags.reserve(32);
            for (const auto& docFlag : handlingFlags) {
                std::string name = docFlag.at("name").get<std::string>();
                std::string description = docFlag.at("description").get<std::string>();
                allFlags.HandlingFlags.emplace_back(Flags::SFlag{ name, description });
            }
        }
        else {
            logger.Write(ERROR, "[Flags] Expected 32 items in strHandlingFlags");
        }

        if (advancedFlags.size() == 32) {
            allFlags.AdvancedFlags.reserve(32);
            for (const auto& docFlag : advancedFlags) {
                std::string name = docFlag.at("name").get<std::string>();
                std::string description = docFlag.at("description").get<std::string>();
                allFlags.AdvancedFlags.emplace_back(Flags::SFlag{ name, description });
            }
        }
        else {
            logger.Write(ERROR, "[Flags] Expected 32 items in strAdvancedFlags");
        }

        logger.Write(INFO, "[Flags] Finished reading flag info");
    }
    catch (std::exception& ex) {
        logger.Write(ERROR, fmt::format("[Flags] Failed to parse flags file. Error: '{}'", ex.what()));
    }
    return allFlags;
}

std::string GetText(const std::string& fileName) {
    std::ifstream flagsFile(fileName);
    if (!flagsFile.is_open()) {
        logger.Write(ERROR, fmt::format("[Flags] Failed to open file '{}'", fileName));
        return {};
    }

    std::ostringstream oss;
    oss << flagsFile.rdbuf();
    return oss.str();
}

std::string GetRemoteText(const std::string& url) {

    return {};
}

void Flags::Load() {
    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string flagsFilePath = absoluteModPath + "\\flags.json";

    const std::string remotePath = "https://raw.githubusercontent.com/E66666666/GTAVHandlingInfo/master/flags.json";

    // TODO: Update enable/disable in a ScriptSettings or something.
    bool enableUpdate = true;

    auto localFlags = ReadJson(GetText(flagsFilePath));

    if (enableUpdate) {
        auto remoteText = GetRemoteText(remotePath);
        auto remoteFlags = ReadJson(remoteText);

        if (remoteFlags.Version > localFlags.Version) {
            logger.Write(INFO, fmt::format("[Flags] Remote '{}.{}' was newer than local '{}.{}', using remote",
                remoteFlags.Version.Major, remoteFlags.Version.Minor, localFlags.Version.Major, localFlags.Version.Minor));

            //UpdateLocalFile(flagsFilePath);
            g_AllFlags = remoteFlags;
        }
        else {
            logger.Write(INFO, fmt::format("[Flags] Remote '{}.{}' was older than or same as local '{}.{}', ignoring remote",
                remoteFlags.Version.Major, remoteFlags.Version.Minor, localFlags.Version.Major, localFlags.Version.Minor));

            g_AllFlags = localFlags;
        }
    }
    else {
        logger.Write(INFO, fmt::format("[Flags] Using local version"));

        g_AllFlags = localFlags;
    }
}
