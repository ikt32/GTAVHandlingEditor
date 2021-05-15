#include "HandlingFlags.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"
#include "Constants.h"

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fstream>

using json = nlohmann::json;

namespace {
    std::vector<Flags::SFlag> g_ModelFlags;
    std::vector<Flags::SFlag> g_HandlingFlags;
    std::vector<Flags::SFlag> g_AdvancedFlags;
}

const std::vector<Flags::SFlag>& Flags::GetModelFlags() {
    return g_ModelFlags;
}

const std::vector<Flags::SFlag>& Flags::GetHandlingFlags() {
    return g_HandlingFlags;
}

const std::vector<Flags::SFlag>& Flags::GetAdvancedFlags() {
    return g_AdvancedFlags;
}

void LoadJson(const std::string& fileName) {
    std::ifstream flagsFile(fileName);
    if (!flagsFile.is_open()) {
        logger.Write(ERROR, fmt::format("[Flags] Failed to open file '{}'", fileName));
        return;
    }

    json flagsDoc;

    try {
        flagsFile >> flagsDoc;

        auto version = flagsDoc.at("version");
        logger.Write(INFO, fmt::format("[Flags] Version '{}'", version.get<std::string>()));

        auto flags = flagsDoc.at("flags");
        auto modelFlags = flags.at("strModelFlags");
        auto handlingFlags = flags.at("strHandlingFlags");
        auto advancedFlags = flags.at("strAdvancedFlags");

        if (modelFlags.size() == 32) {
            g_ModelFlags.reserve(32);
            for (const auto& docFlag : modelFlags) {
                std::string name = docFlag.at("name").get<std::string>();
                std::string description = docFlag.at("description").get<std::string>();
                g_ModelFlags.emplace_back(Flags::SFlag{ name, description });
            }
        }
        else {
            logger.Write(ERROR, "[Flags] Expected 32 items in strModelFlags");
        }

        if (handlingFlags.size() == 32) {
            g_HandlingFlags.reserve(32);
            for (const auto& docFlag : handlingFlags) {
                std::string name = docFlag.at("name").get<std::string>();
                std::string description = docFlag.at("description").get<std::string>();
                g_HandlingFlags.emplace_back(Flags::SFlag{ name, description });
            }
        }
        else {
            logger.Write(ERROR, "[Flags] Expected 32 items in strHandlingFlags");
        }

        if (advancedFlags.size() == 32) {
            g_AdvancedFlags.reserve(32);
            for (const auto& docFlag : advancedFlags) {
                std::string name = docFlag.at("name").get<std::string>();
                std::string description = docFlag.at("description").get<std::string>();
                g_AdvancedFlags.emplace_back(Flags::SFlag{ name, description });
            }
        }
        else {
            logger.Write(ERROR, "[Flags] Expected 32 items in strAdvancedFlags");
        }

        logger.Write(INFO, "[Flags] Finished reading flag info");
    }
    catch (std::exception& ex) {
        logger.Write(ERROR, fmt::format("[Flags] Failed to parse flags file. Error: '{}'", ex.what()));
        return;
    }
}

void Flags::Load() {
    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string flagsFilePath = absoluteModPath + "\\flags.json";

    LoadJson(flagsFilePath);
}
