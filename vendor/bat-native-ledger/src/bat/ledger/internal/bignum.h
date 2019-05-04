/* Copyright (c) 2019 The OneVN Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVNLEDGER_BAT_BIGNUM_H_
#define ONEVNLEDGER_BAT_BIGNUM_H_

#include <string>

extern "C" {
#include "relic.h"  // NOLINT
}

namespace onevnledger_bat_bignum {
  void prepareBigNum(bn_t& big_num, const std::string& probi);  // NOLINT

  std::string bigNumToString(bn_t& number);  // NOLINT

  std::string sum(const std::string& a_string, const std::string& b_string);

  std::string sub(const std::string& a_string, const std::string& b_string);

  std::string mul(const std::string& a_string, const std::string& b_string);

}  // namespace onevnledger_bat_bignum

#endif  // ONEVNLEDGER_BAT_BIGNUM_H_
