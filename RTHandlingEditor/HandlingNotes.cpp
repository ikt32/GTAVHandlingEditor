#include "HandlingNotes.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"
#include "Constants.h"

#include <curl/curl.h>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <filesystem>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "GDI32.LIB")
#pragma comment(lib, "ADVAPI32.LIB")
#pragma comment(lib, "CRYPT32.LIB")
#pragma comment(lib, "USER32.LIB")
#pragma comment(lib, "Wldap32.Lib")
#pragma comment(lib, "Normaliz.lib")

using json = nlohmann::json;

namespace {
    struct SVersion {
        int Major;
        int Minor;

        bool operator>(const SVersion& other) {
            return Major > other.Major ||
                Major == other.Major && Minor > other.Minor;
        }
    };

    struct SAllNotes {
        SVersion Version;
        std::map<std::string, Notes::SHandlingParamInfo> Notes;
    };

    SAllNotes g_AllNotes;
}

namespace Notes {
    SAllNotes ReadJson(const std::string& jsonContent);
    std::string GetText(const std::string& fileName);
    std::string GetRemoteText(const std::string& url);
    std::size_t WriteCallback(char* data, size_t size, size_t nmemb, std::string* writerData);
}

std::string Notes::GetVersion() {
    return fmt::format("{}.{}", g_AllNotes.Version.Major, g_AllNotes.Version.Minor);
}

const std::map<std::string, Notes::SHandlingParamInfo>& Notes::GetHandlingNotes() {
    return g_AllNotes.Notes;
}

SAllNotes Notes::ReadJson(const std::string& jsonContent) {
    SAllNotes allNotes{};
    allNotes.Version = { 0, 0 };

    try {
        json notesDoc = json::parse(jsonContent);

        auto version = notesDoc.value("version", "0.0");
        logger.Write(INFO, fmt::format("[Notes] Version '{}'", version));

        int major, minor;

        // Parse version.
        int argsScanned = sscanf_s(version.c_str(), "%d.%d", &major, &minor);
        if (argsScanned != 2) {
            logger.Write(ERROR, fmt::format("[Notes] File version unknown: '{}'", version));
            return {};
        }
        allNotes.Version = { major, minor };

        auto notes = notesDoc.at("notes");
        for (const auto& noteItem : notes.items()) {
            Notes::SHandlingParamInfo paramInfo{
                .Details = noteItem.value()["description"]
            };
            for (const auto& extra : noteItem.value()["extra"]) {
                paramInfo.ExtraLines.push_back(extra.get<std::string>());
            }

            allNotes.Notes.emplace(noteItem.key(), paramInfo);
        }

        logger.Write(INFO, "[Notes] Finished reading notes info");
    }
    catch (std::exception& ex) {
        logger.Write(ERROR, fmt::format("[Notes] Failed to parse notes file. Error: '{}'", ex.what()));
        logger.Write(ERROR, fmt::format("[Notes] Raw json content: {}", jsonContent));
    }
    return allNotes;
}

std::string Notes::GetText(const std::string& fileName) {
    std::ifstream notesFile(fileName);
    if (!notesFile.is_open()) {
        logger.Write(ERROR, fmt::format("[Notes] Failed to open file '{}'", fileName));
        return {};
    }

    std::ostringstream oss;
    oss << notesFile.rdbuf();
    return oss.str();
}

std::size_t Notes::WriteCallback(char* data, size_t size, size_t nmemb, std::string* writerData) {
    if (writerData == NULL)
        return 0;
    writerData->append(data, size * nmemb);
    return size * nmemb;
}

std::string Notes::GetRemoteText(const std::string& url) {
    CURL* curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        long httpCode(0);
        std::string httpData;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpData);

        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if (httpCode == 200) {
            logger.Write(INFO, fmt::format("[Notes] Successfully got json file"));
            return httpData;
        }
        else {
            logger.Write(ERROR, fmt::format("[Notes] Failed to get json file, status code: {}", httpCode));
        }
    }

    return {};
}

void Notes::Load() {
    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string notesFilePath = absoluteModPath + "\\notes.json";

    const std::string remotePath = "https://raw.githubusercontent.com/E66666666/GTAVHandlingInfo/master/notes.json";

    // TODO: Update enable/disable in a ScriptSettings or something.
    bool enableUpdate = true;

    auto localNotes = ReadJson(GetText(notesFilePath));

    if (enableUpdate) {
        auto remoteText = GetRemoteText(remotePath);
        auto remoteNotes = ReadJson(remoteText);

        if (remoteNotes.Version > localNotes.Version) {
            logger.Write(INFO, fmt::format("[Notes] Remote '{}.{}' was newer than local '{}.{}', using remote",
                remoteNotes.Version.Major, remoteNotes.Version.Minor, localNotes.Version.Major, localNotes.Version.Minor));

            bool localExists = std::filesystem::exists(std::filesystem::path{ notesFilePath });
            int renameResult = rename(notesFilePath.c_str(), fmt::format("{}.{}.{}.bak", notesFilePath, localNotes.Version.Major, localNotes.Version.Minor).c_str());
            if (!localExists || renameResult == 0) {
                std::ofstream out(notesFilePath, std::ios::out | std::ios::binary);
                out << remoteText;
                out.close();
                logger.Write(INFO, "[Notes] Downloaded new notes file");
            }
            else {
                logger.Write(ERROR, "[Notes] Failed to rename old notes file, not replacing it. Error: %d", renameResult);
            }

            g_AllNotes = remoteNotes;
        }
        else {
            logger.Write(INFO, fmt::format("[Notes] Remote '{}.{}' was older than or same as local '{}.{}', ignoring remote",
                remoteNotes.Version.Major, remoteNotes.Version.Minor, localNotes.Version.Major, localNotes.Version.Minor));

            g_AllNotes = localNotes;
        }
    }
    else {
        logger.Write(INFO, fmt::format("[Notes] Using local version"));

        g_AllNotes = localNotes;
    }
}
