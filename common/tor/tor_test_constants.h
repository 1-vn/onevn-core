/* Copyright (c) 2019 The OneVN Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_COMMON_TOR_TOR_TEST_CONSTANTS_H_
#define ONEVN_COMMON_TOR_TOR_TEST_CONSTANTS_H_

#include "base/files/file_path.h"

namespace tor {

extern const char kTestTorProxy[];
extern const char kTestTorPacString[];
extern const base::FilePath::CharType kTestTorPath[];
extern const base::FilePath::CharType kTestBrokenTorPath[];

}  // namespace tor

#endif  // ONEVN_COMMON_TOR_TOR_TEST_CONSTANTS_H_
