/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_LEDGER_DATA_STORE_ADAPTER_H_
#define BAT_LEDGER_DATA_STORE_ADAPTER_H_

#include <string>

#include "bat/ledger/ledger_callback_handler.h"
#include "bat/ledger/publisher_info.h"

namespace ledger {

class LedgerClient;

// TODO(nejczdovc) we should be providing result back as well
using PublisherInfoListCallback =
    std::function<void(const PublisherInfoList&, uint32_t /* next_record */)>;
using SaveContributionInfoCallback = std::function<void(Result)>;

class DataStoreAdapter {
 public:
  enum Type {
    SQL,
  };

  virtual ~DataStoreAdapter() = default;

  static DataStoreAdapter* CreateInstance(Type type);

  virtual void Initialize(LedgerClient* client) = 0;

  virtual void SaveContributionInfo(
      const std::string& probi,
      const int month,
      const int year,
      const uint32_t date,
      const std::string& publisher_key,
      const ledger::REWARDS_CATEGORY category,
      SaveContributionInfoCallback callback) = 0;

  virtual void GetRecurringTips(PublisherInfoListCallback callback) = 0;
  virtual void GetOneTimeTips(ledger::ACTIVITY_MONTH month,
                              int32_t year,
                              ledger::PublisherInfoListCallback callback) = 0;

  // // TODO(bridiver) - add callback
  // virtual bool GetTips(ledger::PublisherInfoList* list,
  //                      ledger::ACTIVITY_MONTH month,
  //                      int year) = 0;

  // virtual void SavePublisherInfo(std::unique_ptr<PublisherInfo> publisher_info,
  //                               PublisherInfoCallback callback) = 0;

  // virtual void LoadPublisherInfo(const std::string& publisher_key,
  //                                PublisherInfoCallback callback) = 0;

  // virtual void LoadPanelPublisherInfo(ActivityInfoFilter filter,
  //                                     PublisherInfoCallback callback) = 0;

  // virtual void RestorePublishers(ledger::OnRestoreCallback callback) = 0;

  // virtual void GetExcludedPublishersNumberDB(
  //     ledger::GetExcludedPublishersNumberDBCallback callback) = 0;

  // virtual void GetActivityInfoList(
  //     uint32_t start,
  //     uint32_t limit,
  //     ledger::ActivityInfoFilter filter,
  //     ledger::PublisherInfoListCallback callback) = 0;

  // virtual void SaveActivityInfo(std::unique_ptr<PublisherInfo> publisher_info,
  //                               PublisherInfoCallback callback) = 0;

  // virtual void LoadActivityInfo(ActivityInfoFilter filter,
  //                               PublisherInfoCallback callback) = 0;

  // // TODO(bridiver) - add callback
  // virtual void SaveMediaPublisherInfo(const std::string& media_key,
  //                                     const std::string& publisher_id) = 0;

  // virtual void LoadMediaPublisherInfo(const std::string& media_key,
  //                                     PublisherInfoCallback callback) = 0;

  // // TODO(bridiver) - add callback
  // virtual void SaveRecurringDonation(const std::string& publisher_key,
  //                                    const int amount);

  // virtual void GetRecurringDonations(
  //     ledger::PublisherInfoListCallback callback) = 0;

  // virtual void RemoveRecurringDonation(
  //     const std::string& publisher_key,
  //     ledger::RecurringRemoveCallback callback) = 0;

  // // TODO(bridiver) - add callback
  // virtual void SavePendingContribution(
  //     const ledger::PendingContributionList& list) = 0;

  // // virtual void GetPendingContributionsTotal(
  // //     const GetPendingContributionsTotalCallback& callback) override;

  // // TODO(bridiver) - add callback
  // virtual bool DeleteActivityInfo(const std::string& publisher_key,
  //                                 uint64_t reconcile_stamp) = 0;
};

}  // namespace ledger

#endif  // BAT_LEDGER_DATA_STORE_ADAPTER_H_