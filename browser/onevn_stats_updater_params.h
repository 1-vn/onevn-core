/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_ONEVN_STATS_UPDATER_PARAMS_H_
#define ONEVN_BROWSER_ONEVN_STATS_UPDATER_PARAMS_H_

#include <string>

#include "base/macros.h"

class OnevnStatsUpdaterTest;
class PrefService;

namespace base {
class Time;
}

namespace onevn {

class OnevnStatsUpdaterParams {
 public:
  explicit OnevnStatsUpdaterParams(PrefService* pref_service);
  OnevnStatsUpdaterParams(PrefService* pref_service,
                          const std::string& ymd,
                          int woy,
                          int month);
  ~OnevnStatsUpdaterParams();

  std::string GetDailyParam() const;
  std::string GetWeeklyParam() const;
  std::string GetMonthlyParam() const;
  std::string GetFirstCheckMadeParam() const;
  std::string GetWeekOfInstallationParam() const;
  std::string GetReferralCodeParam() const;

  void SavePrefs();

 private:
  friend class ::OnevnStatsUpdaterTest;
  PrefService* pref_service_;
  std::string ymd_;
  int woy_;
  int month_;
  std::string last_check_ymd_;
  int last_check_woy_;
  int last_check_month_;
  bool first_check_made_;
  std::string week_of_installation_;
  std::string referral_promo_code_;
  static base::Time g_current_time;

  void LoadPrefs();

  std::string BooleanToString(bool bool_value) const;

  std::string GetDateAsYMD(const base::Time& time) const;
  std::string GetCurrentDateAsYMD() const;
  std::string GetLastMondayAsYMD() const;
  int GetCurrentMonth() const;
  int GetCurrentISOWeekNumber() const;
  base::Time GetCurrentTimeNow() const;

  static void SetCurrentTimeForTest(const base::Time& current_time);

  DISALLOW_COPY_AND_ASSIGN(OnevnStatsUpdaterParams);
};

}  // namespace onevn

#endif  // ONEVN_BROWSER_ONEVN_STATS_UPDATER_PARAMS_H_
