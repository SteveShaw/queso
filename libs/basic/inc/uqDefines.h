/* libs/basic/inc/uqDefines.h
 * 
 * Copyright (C) 2008 The QUESO Team, http://queso.ices.utexas.edu/
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __UQ_DEFINES_H__
#define __UQ_DEFINES_H__

#include <iostream>
#include <limits>
#include <stdlib.h> // For exit()

int uqMyRank();

const int UQ_UNAVAILABLE_RANK         = -1;
const unsigned int UQ_INVALID_DOF_ID  = std::numeric_limits<unsigned int>::max();
const unsigned int UQ_INVALID_NODE_ID = std::numeric_limits<unsigned int>::max();

const int UQ_OK_RC                         =  0;
const int UQ_INCOMPLETE_IMPLEMENTATION_RC  = -1;
const int UQ_INVALID_PARAMETER_SPEC_RC     = -2;
const int UQ_INVALID_OBSERVABLE_SPEC_RC    = -3;
const int UQ_INVALID_QOI_SPEC_RC           = -4;
const int UQ_INVALID_INTERNAL_RESULT_RC    = -5;
const int UQ_INVALID_INTERNAL_STATE_RC     = -6;
const int UQ_FAILED_TO_OPEN_FILE_RC        = -7;
const int UQ_MATRIX_IS_NOT_POS_DEFINITE_RC = -8;
const int UQ_FAILED_READING_FILE_RC        = -9;
const int UQ_INVALID_SPACE_COMPONENT_ID_RC = -10;

#define UQ_RC_MACRO(iRC,givenRank,where,what,retValue) \
  if (iRC) {                                           \
    int macroRank = givenRank;                         \
    if (macroRank == UQ_UNAVAILABLE_RANK) {            \
      macroRank = uqMyRank();                          \
    }                                                  \
    std::cerr << "UQ RC ERROR"                         \
              << ", rank "  << macroRank               \
              << ", in "    << where                   \
              << ": "       << what                    \
              << ", iRC = " << iRC                     \
              << std::endl;                            \
    return retValue;                                   \
  }

#define UQ_TEST_MACRO(test,givenRank,where,what,retValue) \
  if (test) {                                             \
    int macroRank = givenRank;                            \
    if (macroRank == UQ_UNAVAILABLE_RANK) {               \
      macroRank = uqMyRank();                             \
    }                                                     \
    std::cerr << "UQ TEST ERROR"                          \
              << ", rank " << macroRank                   \
              << ", in "   << where                       \
              << ": "      << what                        \
              << std::endl;                               \
    return retValue;                                      \
  }

#define UQ_FATAL_RC_MACRO(iRC,givenRank,where,what) \
  if (iRC) {                                        \
    int macroRank = givenRank;                      \
    if (macroRank == UQ_UNAVAILABLE_RANK) {         \
      macroRank = uqMyRank();                       \
    }                                               \
    std::cerr << "UQ RC FATAL ERROR"                \
              << ", rank "  << macroRank            \
              << ", in "    << where                \
              << ": "       << what                 \
              << ", iRC = " << iRC                  \
              << ". Exiting..."                     \
              << std::endl;                         \
    exit(1);                                        \
  }

#define UQ_FATAL_TEST_MACRO(test,givenRank,where,what) \
  if (test) {                                          \
    int macroRank = givenRank;                         \
    if (macroRank == UQ_UNAVAILABLE_RANK) {            \
      macroRank = uqMyRank();                          \
    }                                                  \
    std::cerr << "UQ TEST FATAL ERROR"                 \
              << ", rank "  << macroRank               \
              << ", in "    << where                   \
              << ": "       << what                    \
              << ". Exiting..."                        \
              << std::endl;                            \
    exit(1);                                           \
  }

#endif // __UQ_DEFINES_H__
