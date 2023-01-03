#pragma once
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VERSION_MAJOR 3
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_LABEL "beta1"

#ifdef VERSION_LABEL
#define VERSION_LABEL_TXT "-" VERSION_LABEL
#else
#define VERSION_LABEL_TXT ""
#endif

namespace Constants {
    static const char* const DisplayVersion =
        "v" STR(VERSION_MAJOR) "."  STR(VERSION_MINOR) "." STR(VERSION_PATCH) VERSION_LABEL_TXT;

    static const char* const iktDir = "ikt";
    static const char* const ModDir = "HandlingEditor";
    static const char* const NotificationPrefix = "~b~Handling Editor~w~";
}
