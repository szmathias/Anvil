//
// Created by zack on 9/27/25.
//

#ifndef ANVIL_VERSION_H
#define ANVIL_VERSION_H

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// Note: ANV_VERSION_(MAJOR, MINOR, PATCH, and STRING) macros come from CMake
#define ANV_VERSION_BUILD_DATE __DATE__
#define ANV_VERSION_BUILD_TIME __TIME__

// Enable conditional compilation based on version
#define ANV_VERSION_AT_LEAST(maj, min) \
    (ANV_VERSION_MAJOR > (maj) || \
    (ANV_VERSION_MAJOR == (maj) && ANV_VERSION_MINOR >= (min)))

/**
 * Struct containing version information about the Anvil library.
 */
typedef struct ANVVersionInfo
{
    int major;
    int minor;
    int patch;
    const char* string;
    const char* build_date;
    const char* build_time;
} ANVVersionInfo;

/**
 * Get version information about the Anvil library from
 * when it was built.
 *
 * @return Version information struct
 */
ANV_API ANVVersionInfo anv_version_get(void);

/**
 * Check if the current Anvil library version is compatible
 * with the required version.
 *
 * @param required_major Required major version
 * @param required_minor Required minor version
 * @return If the current version is compatible with the required version
 */
ANV_API bool anv_version_compatible(int required_major, int required_minor);

/**
 * Get a string representation of the Anvil library version.
 *
 * @return Version string as "major.minor.patch"
 */
ANV_API const char* anv_version_string(void);

/**
 * Get the build date of the Anvil library.
 *
 * @return Build date string in "MMM DD YYYY" format
 */
ANV_API const char* anv_version_build_date(void);

/**
 * Get the build time of the Anvil library.
 *
 * @return Build time string in "HH:MM:SS" format
 */
ANV_API const char* anv_version_build_time(void);

#ifdef __cplusplus
}
#endif

#endif //ANVIL_VERSION_H
