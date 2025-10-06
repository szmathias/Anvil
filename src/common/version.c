//
// Created by zack on 9/27/25.
//

#include "version.h"

ANV_API ANVVersionInfo anv_version_get(void)
{
    return (ANVVersionInfo)
    {
        .major = ANV_VERSION_MAJOR,
        .minor = ANV_VERSION_MINOR,
        .patch = ANV_VERSION_PATCH,
        .string = ANV_VERSION_STRING,
        .build_date = ANV_VERSION_BUILD_DATE,
        .build_time = ANV_VERSION_BUILD_TIME
    };
}

ANV_API bool anv_version_compatible(const int required_major, const int required_minor)
{
    return (required_major < ANV_VERSION_MAJOR) ||
           (required_major == ANV_VERSION_MAJOR && required_minor <= ANV_VERSION_MINOR);
}

ANV_API const char* anv_version_string(void)
{
    return ANV_VERSION_STRING;
}

ANV_API const char* anv_version_build_date(void)
{
    return ANV_VERSION_BUILD_DATE;
}

ANV_API const char* anv_version_build_time(void)
{
    return ANV_VERSION_BUILD_TIME;
}