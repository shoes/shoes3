/* emeus-version-macros.h
 *
 * Copyright 2016  Endless
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "shoes/layout/emeus-version.h"

/**
 * SECTION: emeus-version
 * @Title: Version macros
 * @Short_Description: Check the version at compile time
 *
 * Emeus allows checking the version of the API at compile time; this can be
 * used to write code that can be used when built against a specific version
 * of the library.
 */

/**
 * EMEUS_MAJOR_VERSION:
 *
 * The major version of Emeus, or 1 in `1.2.3`
 *
 * Since: 1.0
 */

/**
 * EMEUS_MINOR_VERSION:
 *
 * The minor version of Emeus, or 2 in `1.2.3`
 *
 * Since: 1.0
 */

/**
 * EMEUS_MICRO_VERSION:
 *
 * The micro version of Emeus, or 3 in `1.2.3`
 *
 * Since: 1.0
 */

/**
 * EMEUS_CHECK_VERSION:
 * @major: the major version, or 1 in `1.2.3`
 * @minor: the minor version, or 2 in `1.2.3`
 * @micro: the micro version, or 3 in `1.2.3`
 *
 * Compile time check for a specific version of Emeus.
 *
 * This macro evaluates to %TRUE if the version of Emeus is greater than
 * or equal to @major.@minor.@micro.
 *
 * Since: 1.0
 */
#define EMEUS_CHECK_VERSION(major,minor,micro) \
  (((major) > EMEUS_MAJOR_VERSION) || \
   ((major) == EMEUS_MAJOR_VERSION && (minor) > EMEUS_MINOR_VERSION) || \
   ((major) == EMEUS_MAJOR_VERSION && (minor) == EMEUS_MINOR_VERSION && (micro) >= EMEUS_MICRO_VERSION))

#ifndef _EMEUS_PUBLIC
# define _EMEUS_PUBLIC extern
#endif

#ifdef EMEUS_DISABLE_DEPRECATION_WARNINGS
# define EMEUS_DEPRECATED _EMEUS_PUBLIC
# define EMEUS_DEPRECATED_FOR(f) _EMEUS_PUBLIC
# define EMEUS_UNAVAILABLE(maj,min) _EMEUS_PUBLIC
#else
# define EMEUS_DEPRECATED G_DEPRECATED _EMEUS_PUBLIC
# define EMEUS_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f) _EMEUS_PUBLIC
# define EMEUS_UNAVAILABLE(maj,min) G_UNAVAILABLE(maj,min) _EMEUS_PUBLIC
#endif

/**
 * EMEUS_VERSION_1_0:
 *
 * A pre-processor symbol that expands to the 1.0 version of Emeus.
 *
 * You should use this macro with %EMEUS_VERSION_MIN_REQUIRED and
 * %EMEUS_VERSION_MAX_ALLOWED.
 *
 * Since: 1.0
 */
#define EMEUS_VERSION_1_0       (G_ENCODE_VERSION (1, 0))

#if EMEUS_MINOR_VERSION >= 90
# define EMEUS_VERSION_CUR_STABLE       (G_ENCODE_VERSION (EMEUS_MAJOR_VERSION + 1, 0))
#else
# if (EMEUS_MINOR_VERSION % 2)
#  define EMEUS_VERSION_CUR_STABLE      (G_ENCODE_VERSION (EMEUS_MAJOR_VERSION, EMEUS_MINOR_VERSION + 1))
# else
#  define EMEUS_VERSION_CUR_STABLE      (G_ENCODE_VERSION (EMEUS_MAJOR_VERSION, EMEUS_MINOR_VERSION))
# endif
#endif

#if EMEUS_MINOR_VERSION >= 90
# define EMEUS_VERSION_PREV_STABLE      (G_ENCODE_VERSION (EMEUS_MAJOR_VERSION + 1, 0))
#else
# if (EMEUS_MINOR_VERSION % 2)
#  define EMEUS_VERSION_PREV_STABLE     (G_ENCODE_VERSION (EMEUS_MAJOR_VERSION, EMEUS_MINOR_VERSION - 1))
# else
#  define EMEUS_VERSION_PREV_STABLE     (G_ENCODE_VERSION (EMEUS_MAJOR_VERSION, EMEUS_MINOR_VERSION - 2))
# endif
#endif

#ifndef EMEUS_VERSION_MIN_REQUIRED
# define EMEUS_VERSION_MIN_REQUIRED     (EMEUS_VERSION_CUR_STABLE)
#endif

#ifndef EMEUS_VERSION_MAX_ALLOWED
# if EMEUS_VERSION_MIN_REQUIRED > EMEUS_VERSION_PREV_STABLE
#  define EMEUS_VERSION_MAX_ALLOWED     EMEUS_VERSION_MIN_REQUIRED
# else
#  define EMEUS_VERSION_MAX_ALLOWED     EMEUS_VERSION_CUR_STABLE
# endif
#endif

/**
 * EMEUS_VERSION_MIN_REQUIRED:
 *
 * Defines the lower bound of the API of Emeus that can be used.
 *
 * This macro can only be defined prior to including `emeus.h`.
 *
 * The allowed values for this macro are one of the encoded Emeus version symbols,
 * like %EMEUS_VERSION_1_0.
 *
 * If this pre-processor symbol is defined, Emeus will only emit compiler warnings
 * when attempting to use functions that have been deprecated in newer versions
 * of the library; for instance, if %EMEUS_VERSION_MIN_REQUIRED is defined to be
 * %EMEUS_VERSION_1_4, you'll be able to use API that has been deprecated in version
 * 1.0 and 1.2 without any warning; any API deprecated in 1.4 and above will emit a
 * compiler warning.
 *
 * See also: %EMEUS_VERSION_MAX_ALLOWED
 *
 * Since: 1.0
 */

/**
 * EMEUS_VERSION_MAX_ALLOWED:
 *
 * Defines the upper bound of the API of Emeus that can be used.
 *
 * This macro can only be defined prior to including `emeus.h`.
 *
 * The allowed values for this macro are one of the encoded Emeus version symbols,
 * like %EMEUS_VERSION_1_0.
 *
 * If this pre-processor symbol is defined, Emeus will emit a compiler warning when
 * attempting to use functions that have been introduced in newer versions of the
 * library; for instance, if %EMEUS_VERSION_MAX_ALLOWED is defined to be
 * %EMEUS_VERSION_1_2, you'll be able to use API that has been introduced in version
 * 1.0 and 1.2 without any warning; any API introduced in 1.4 and above will emit
 * a compiler warning.
 *
 * See also: %EMEUS_VERSION_MIN_REQUIRED
 *
 * Since: 1.0
 */

/* Sanity checks */
#if EMEUS_VERSION_MAX_ALLOWED < EMEUS_VERSION_MIN_REQUIRED
# error "EMEUS_VERSION_MAX_ALLOWED must be greater than EMEUS_VERSION_MIN_REQUIRED"
#endif
#if EMEUS_VERSION_MIN_REQUIRED < EMEUS_VERSION_1_0
# error "EMEUS_VERSION_MIN_REQUIRED must be greater than EMEUS_VERSION_1_0"
#endif

#if EMEUS_VERSION_MIN_REQUIRED >= EMEUS_VERSION_1_0
# define EMEUS_DEPRECATED_IN_1_0              EMEUS_DEPRECATED
# define EMEUS_DEPRECATED_IN_1_0_FOR(f)       EMEUS_DEPRECATED_FOR(f)
#else
# define EMEUS_DEPRECATED_IN_1_0              _EMEUS_PUBLIC
# define EMEUS_DEPRECATED_IN_1_0_FOR(f)       _EMEUS_PUBLIC
#endif

#if EMEUS_VERSION_MAX_ALLOWED < EMEUS_VERSION_1_0
# define EMEUS_AVAILABLE_IN_1_0               EMEUS_UNAVAILABLE(1, 0)
#else
# define EMEUS_AVAILABLE_IN_1_0               _EMEUS_PUBLIC
#endif
