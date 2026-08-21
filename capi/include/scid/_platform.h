/**
 * @file _platform.h
 * @brief Platform-specific linkage specifications and export macro definitions.
 */

#ifndef SCID_PLATFORM_H
#define SCID_PLATFORM_H

#include <stddef.h>

#ifndef SCID_API
#if defined(_WIN32) && defined(SCID_SHARED)
#if defined(SCID_BUILDING_LIBRARY)
#define SCID_API __declspec(dllexport)
#else
#define SCID_API __declspec(dllimport)
#endif
#else
#define SCID_API
#endif
#endif

#endif
