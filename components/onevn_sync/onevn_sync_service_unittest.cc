/* Copyright 2016 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/files/scoped_temp_dir.h"
#include "base/strings/utf_string_conversions.h"
#include "onevn/components/onevn_sync/client/bookmark_change_processor.h"
#include "onevn/components/onevn_sync/client/onevn_sync_client_impl.h"
#include "onevn/components/onevn_sync/client/client_ext_impl_data.h"
#include "onevn/components/onevn_sync/onevn_sync_service_impl.h"
#include "onevn/components/onevn_sync/onevn_sync_service_factory.h"
#include "onevn/components/onevn_sync/onevn_sync_service_observer.h"
#include "onevn/components/onevn_sync/jslib_const.h"
#include "onevn/components/onevn_sync/jslib_messages.h"
#include "onevn/components/onevn_sync/settings.h"
#include "onevn/components/onevn_sync/sync_devices.h"
#include "onevn/components/onevn_sync/test_util.h"
#include "onevn/components/onevn_sync/values_conv.h"
#include "chrome/browser/bookmarks/bookmark_model_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/bookmarks/test/test_bookmark_client.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "services/network/test/test_network_connection_tracker.h"
#include "net/base/network_interfaces.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- onevn_unit_tests --filter=OnevnSyncServiceTest.*

// OnevnSyncClient::methods
// Name                     | Covered
//------------------------------------
// SetSyncToBrowserHandler  |
// GetSyncToBrowserHandler  |
// SendGotInitData          | OnGetInitData
// SendFetchSyncRecords     |
// SendFetchSyncDevices     |
// SendResolveSyncRecords   |
// SendSyncRecords          |
// SendDeleteSyncUser       |
// SendDeleteSyncCategory   |
// SendGetBookmarksBaseOrder|
// NeedSyncWords            |
// NeedBytesFromSyncWords   |
// OnExtensionInitialized   |

// OnevnSyncService::methods
// Name                      | Covered
//-------------------------------------
// OnSetupSyncHaveCode       | +
// OnSetupSyncNewToSync      | +
// OnDeleteDevice            | +
// OnResetSync               | +
// GetSettingsAndDevices     | +
// GetSyncWords              | +
// GetSeed                   | +
// OnSetSyncEnabled          | +
// OnSetSyncBookmarks        | +
// OnSetSyncBrowsingHistory  | +
// OnSetSyncSavedSiteSettings| +
// AddObserver               | +, SetUp
// RemoveObserver            | +, Teardown
// GetSyncClient             | +, SetUp

// OnevnSyncService  SyncMessageHandler overrides
// Name                      | Covered
//-------------------------------------
// BackgroundSyncStarted     | +, OnevnSyncServiceTest.BookmarkAddedImpl
// BackgroundSyncStopped     | +
// OnSyncDebug               | +
// OnSyncSetupError          | Need UI handler
// OnGetInitData             | +
// OnSaveInitData            | OnevnSyncServiceTest.GetSeed
// OnSyncReady               | +
// OnGetExistingObjects      | +
// OnResolvedSyncRecords     | OnevnSyncServiceTest.BookmarkAddedImpl
// OnDeletedSyncUser         | N/A
// OnDeleteSyncSiteSettings  | N/A
// OnSaveBookmarksBaseOrder  | +
// OnSyncWordsPrepared       | OnevnSyncServiceTest.GetSyncWords
// OnResolvedHistorySites    | N/A
// OnResolvedPreferences     | OnevnSyncServiceTest.OnDeleteDevice,
//                           | OnevnSyncServiceTest.OnResetSync
// OnSyncPrefsChanged        | +

using onevn_sync::OnevnSyncService;
using onevn_sync::OnevnSyncServiceImpl;
using onevn_sync::OnevnSyncServiceObserver;
using onevn_sync::jslib::SyncRecord;
using onevn_sync::MockOnevnSyncClient;
using onevn_sync::RecordsList;
using onevn_sync::SimpleDeviceRecord;
using network::TestNetworkConnectionTracker;
using network::mojom::ConnectionType;
using testing::_;
using testing::AtLeast;

class MockOnevnSyncServiceObserver : public OnevnSyncServiceObserver {
 public:
  MockOnevnSyncServiceObserver() {}

  MOCK_METHOD2(OnSyncSetupError, void(OnevnSyncService*, const std::string&));
  MOCK_METHOD1(OnSyncStateChanged, void(OnevnSyncService*));
  MOCK_METHOD2(OnHaveSyncWords, void(OnevnSyncService*, const std::string&));
};

class OnevnSyncServiceTest : public testing::Test {
 public:
  OnevnSyncServiceTest() {}
  ~OnevnSyncServiceTest() override {}

 protected:
  void SetUp() override {
    EXPECT_TRUE(temp_dir_.CreateUniqueTempDir());
    // register the factory

    profile_ = onevn_sync::CreateOnevnSyncProfile(temp_dir_.GetPath());
    EXPECT_TRUE(profile_.get() != NULL);

    // TODO(bridiver) - this is temporary until some changes are made to
    // to bookmark_change_processor to allow `set_for_testing` like
    // OnevnSyncClient
    BookmarkModelFactory::GetInstance()->SetTestingFactory(
       profile(),
       base::BindRepeating(&onevn_sync::BuildFakeBookmarkModelForTests));

    onevn_sync::OnevnSyncClientImpl::set_for_testing(
        new MockOnevnSyncClient());

    sync_service_ = static_cast<OnevnSyncServiceImpl*>(
        onevn_sync::OnevnSyncServiceFactory::GetInstance()->GetForProfile(
            profile()));

    sync_client_ =
        static_cast<MockOnevnSyncClient*>(sync_service_->GetSyncClient());

    observer_.reset(new MockOnevnSyncServiceObserver);
    sync_service_->AddObserver(observer_.get());
    EXPECT_TRUE(sync_service_ != NULL);

    // TestNetworkConnectionTracker::CreateInstance has been called in
    // TestingBrowserProcess
    TestNetworkConnectionTracker* tracker =
      TestNetworkConnectionTracker::GetInstance();
    tracker->SetConnectionType(ConnectionType::CONNECTION_UNKNOWN);
  }

  void TearDown() override {
    sync_service_->RemoveObserver(observer_.get());
    // this will also trigger a shutdown of the onevn sync service
    profile_.reset();
  }

  void BookmarkAddedImpl();

  Profile* profile() { return profile_.get(); }
  OnevnSyncServiceImpl* sync_service() { return sync_service_; }
  MockOnevnSyncClient* sync_client() { return sync_client_; }
  MockOnevnSyncServiceObserver* observer() { return observer_.get(); }

 private:
  // Need this as a very first member to run tests in UI thread
  // When this is set, class should not install any other MessageLoops, like
  // base::test::ScopedTaskEnvironment
  content::TestBrowserThreadBundle thread_bundle_;

  std::unique_ptr<Profile> profile_;
  OnevnSyncServiceImpl* sync_service_;
  MockOnevnSyncClient* sync_client_;
  std::unique_ptr<MockOnevnSyncServiceObserver> observer_;

  base::ScopedTempDir temp_dir_;
};

TEST_F(OnevnSyncServiceTest, SetSyncEnabled) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncEnabled));
  sync_service()->OnSetSyncEnabled(true);
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncEnabled));
  EXPECT_FALSE(sync_service()->IsSyncInitialized());
  EXPECT_FALSE(sync_service()->IsSyncConfigured());
}

TEST_F(OnevnSyncServiceTest, SetSyncDisabled) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged).Times(1);
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  sync_service()->OnSetSyncEnabled(true);
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncEnabled));

  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged).Times(1);
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  sync_service()->OnSetSyncEnabled(false);
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncEnabled));
  EXPECT_FALSE(sync_service()->IsSyncInitialized());
  EXPECT_FALSE(sync_service()->IsSyncConfigured());
}

TEST_F(OnevnSyncServiceTest, IsSyncConfiguredOnNewProfile) {
  EXPECT_FALSE(sync_service()->IsSyncConfigured());
}

TEST_F(OnevnSyncServiceTest, IsSyncInitializedOnNewProfile) {
  EXPECT_FALSE(sync_service()->IsSyncInitialized());
}

void OnevnSyncServiceTest::BookmarkAddedImpl() {
  // OnevnSyncService: real
  // OnevnSyncClient: mock
  // Invoke OnevnSyncService::BookmarkAdded
  // Expect OnevnSyncClient::SendSyncRecords invoked
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged).Times(1);
  EXPECT_CALL(*observer(),
      OnSyncStateChanged(sync_service())).Times(AtLeast(1));
  sync_service()->OnSetupSyncNewToSync("UnitTestBookmarkAdded");
  sync_service()->BackgroundSyncStarted(true/*startup*/);

  auto* bookmark_model = BookmarkModelFactory::GetForBrowserContext(profile());
  bookmarks::AddIfNotBookmarked(bookmark_model,
                                 GURL("https://a.com"),
                                 base::ASCIIToUTF16("A.com - title"));
  // Force service send bookmarks and fire the mock
  EXPECT_CALL(*sync_client(), SendSyncRecords(_, _)).Times(1);
  sync_service()->OnResolvedSyncRecords(onevn_sync::jslib_const::kBookmarks,
    std::make_unique<RecordsList>());
}

TEST_F(OnevnSyncServiceTest, BookmarkAdded) {
  BookmarkAddedImpl();
}

TEST_F(OnevnSyncServiceTest, BookmarkDeleted) {
  BookmarkAddedImpl();
  auto* bookmark_model = BookmarkModelFactory::GetForBrowserContext(profile());

  // And just now can actually test delete
  std::vector<const bookmarks::BookmarkNode*> nodes;
  bookmarks::GetMostRecentlyAddedEntries(bookmark_model, 1, &nodes);
  ASSERT_EQ(nodes.size(), 1u);
  ASSERT_NE(nodes.at(0), nullptr);
  // TODO(alexeyb): preciece with mock expect filter
  EXPECT_CALL(*sync_client(), SendSyncRecords(_, _)).Times(1);
  bookmark_model->Remove(nodes.at(0));
  // record->action = jslib::SyncRecord::Action::A_DELETE;
  // <= BookmarkNodeToSyncBookmark <= BookmarkChangeProcessor::SendUnsynced
  // <= OnevnSyncServiceImpl::OnResolvedSyncRecords
  sync_service()->OnResolvedSyncRecords(onevn_sync::jslib_const::kBookmarks,
    std::make_unique<RecordsList>());
}

TEST_F(OnevnSyncServiceTest, OnSetupSyncHaveCode) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  // Expecting sync state changed twice: for enabled state and for device name
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(2);
  sync_service()->OnSetupSyncHaveCode("word1 word2 word3", "test_device");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));
}

TEST_F(OnevnSyncServiceTest, OnSetupSyncHaveCode_EmptyDeviceName) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  // Expecting sync state changed twice: for enabled state and for device name
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(2);
  sync_service()->OnSetupSyncHaveCode("word1 word2 word3", "");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));
  EXPECT_EQ(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncDeviceName), net::GetHostName());
}

TEST_F(OnevnSyncServiceTest, OnSetupSyncNewToSync) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  // Expecting sync state changed twice: for enabled state and for device name
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(2);
  sync_service()->OnSetupSyncNewToSync("test_device");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));
}

TEST_F(OnevnSyncServiceTest, OnSetupSyncNewToSync_EmptyDeviceName) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  // Expecting sync state changed twice: for enabled state and for device name
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(2);
  sync_service()->OnSetupSyncNewToSync("");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));
  EXPECT_EQ(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncDeviceName), net::GetHostName());
}

TEST_F(OnevnSyncServiceTest, GetSettingsAndDevices) {
  // The test absorbs OnSetupSyncNewToSync test
  auto callback1 = base::BindRepeating(
      [](std::unique_ptr<onevn_sync::Settings> settings,
         std::unique_ptr<onevn_sync::SyncDevices> devices) {
            EXPECT_TRUE(settings->this_device_name_.empty());
            EXPECT_TRUE(settings->this_device_id_.empty());
            EXPECT_FALSE(settings->sync_configured_);
            EXPECT_FALSE(settings->sync_this_device_);
            EXPECT_FALSE(settings->sync_bookmarks_);
            EXPECT_FALSE(settings->sync_settings_);
            EXPECT_FALSE(settings->sync_history_);
            EXPECT_EQ(devices->size(), 0u);
        });

  sync_service()->GetSettingsAndDevices(callback1);
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  // Expecting sync state changed twice: for enabled state and for device name
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(2);
  sync_service()->OnSetupSyncNewToSync("test_device");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));
  auto callback2 = base::BindRepeating(
      [](std::unique_ptr<onevn_sync::Settings> settings,
         std::unique_ptr<onevn_sync::SyncDevices> devices) {
          // Other fields may be switched later
          EXPECT_EQ(settings->this_device_name_, "test_device");
          EXPECT_TRUE(settings->sync_this_device_);
      });
  sync_service()->GetSettingsAndDevices(callback2);
}

TEST_F(OnevnSyncServiceTest, GetSyncWords) {
  EXPECT_CALL(*sync_client(), NeedSyncWords);
  sync_service()->GetSyncWords();
  // The call should go to OnevnSyncClient => OnevnSyncEventRouter =>
  // background.js onNeedSyncWords => api::OnevnSyncSyncWordsPreparedFunction =>
  // OnevnSyncServiceImpl::OnSyncWordsPrepared
  // but as we have a mock instead of OnevnSyncClient, emulate the response
  const std::string words = "word1 word2 word3";
  EXPECT_CALL(*observer(), OnHaveSyncWords(sync_service(), words)).Times(1);
  sync_service()->OnSyncWordsPrepared(words);
}

TEST_F(OnevnSyncServiceTest, SyncSetupError) {
  EXPECT_CALL(*observer(), OnSyncSetupError(sync_service(), _)).Times(1);
  sync_service()->OnSetupSyncHaveCode("", "");
}

TEST_F(OnevnSyncServiceTest, GetSeed) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged);
  EXPECT_CALL(*observer(),
      OnSyncStateChanged(sync_service())).Times(AtLeast(2));
  sync_service()->OnSetupSyncNewToSync("test_device");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));

  // Service gets seed from client via OnevnSyncServiceImpl::OnSaveInitData
  const auto binary_seed = onevn_sync::Uint8Array(16, 77);

  EXPECT_TRUE(sync_service()->sync_prefs_->GetPrevSeed().empty());
  sync_service()->OnSaveInitData(binary_seed, {0});
  std::string expected_seed = onevn_sync::StrFromUint8Array(binary_seed);
  EXPECT_EQ(sync_service()->GetSeed(), expected_seed);
  EXPECT_TRUE(sync_service()->sync_prefs_->GetPrevSeed().empty());
}

bool DevicesContains(onevn_sync::SyncDevices* devices, const std::string& id,
    const std::string& name) {
  DCHECK(devices);
  for (const auto& device : devices->devices_) {
    if (device.device_id_ == id && device.name_ == name) {
      return true;
    }
  }
  return false;
}

MATCHER_P2(ContainsDeviceRecord, action, name,
    "contains device sync record with params") {
  for (const auto& record : arg) {
    if (record->has_device()) {
      const auto& device = record->GetDevice();
      if (record->action == action &&
          device.name == name) {
        return true;
      }
    }
  }
  return false;
}

TEST_F(OnevnSyncServiceTest, OnDeleteDevice) {
  RecordsList records;
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "1", "device1"));
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "2", "device2"));
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "3", "device3"));
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  sync_service()->OnResolvedPreferences(records);

  sync_service()->sync_prefs_->SetThisDeviceId("1");
  auto devices = sync_service()->sync_prefs_->GetSyncDevices();

  EXPECT_TRUE(DevicesContains(devices.get(), "1", "device1"));
  EXPECT_TRUE(DevicesContains(devices.get(), "2", "device2"));
  EXPECT_TRUE(DevicesContains(devices.get(), "3", "device3"));

  EXPECT_CALL(*sync_client(), SendSyncRecords("PREFERENCES",
      ContainsDeviceRecord(SyncRecord::Action::A_DELETE, "device3"))).Times(1);
  sync_service()->OnDeleteDevice("3");

  RecordsList resolved_records;
  auto resolved_record = SyncRecord::Clone(*records.at(2));
  resolved_record->action = SyncRecord::Action::A_DELETE;
  resolved_records.push_back(std::move(resolved_record));

  // Emulate we have twice the same delete device record to ensure we
  // don't have failed DCHECK in debug build
  auto resolved_record2 = SyncRecord::Clone(*records.at(2));
  resolved_record2->action = SyncRecord::Action::A_DELETE;
  resolved_records.push_back(std::move(resolved_record2));

  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  sync_service()->OnResolvedPreferences(resolved_records);

  auto devices_final = sync_service()->sync_prefs_->GetSyncDevices();
  EXPECT_TRUE(DevicesContains(devices_final.get(), "1", "device1"));
  EXPECT_TRUE(DevicesContains(devices_final.get(), "2", "device2"));
  EXPECT_FALSE(DevicesContains(devices_final.get(), "3", "device3"));
}

TEST_F(OnevnSyncServiceTest, OnDeleteDeviceWhenOneDevice) {
  sync_service()->sync_prefs_->SetThisDeviceId("1");
  RecordsList records;
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "1", "device1"));
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "2", "device2"));
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  sync_service()->OnResolvedPreferences(records);

  auto devices = sync_service()->sync_prefs_->GetSyncDevices();

  EXPECT_TRUE(DevicesContains(devices.get(), "1", "device1"));
  EXPECT_TRUE(DevicesContains(devices.get(), "2", "device2"));

  EXPECT_CALL(*sync_client(), SendSyncRecords).Times(1);
  sync_service()->OnDeleteDevice("2");

  RecordsList resolved_records;
  auto resolved_record = SyncRecord::Clone(*records.at(1));
  resolved_record->action = SyncRecord::Action::A_DELETE;
  resolved_records.push_back(std::move(resolved_record));
  // Expecting to be called one time to set the new devices list
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);

  EXPECT_CALL(*sync_client(), SendSyncRecords).Times(1);

  sync_service()->OnResolvedPreferences(resolved_records);

  auto devices_semi_final = sync_service()->sync_prefs_->GetSyncDevices();
  EXPECT_FALSE(DevicesContains(devices_semi_final.get(), "2", "device2"));
  EXPECT_TRUE(DevicesContains(devices_semi_final.get(), "1", "device1"));

  // Emulate sending DELETE for this device
  RecordsList resolved_records2;
  auto resolved_record2 = SyncRecord::Clone(*records.at(0));
  resolved_record2->action = SyncRecord::Action::A_DELETE;
  resolved_records2.push_back(std::move(resolved_record2));
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(3);

  sync_service()->OnResolvedPreferences(resolved_records2);

  auto devices_final = sync_service()->sync_prefs_->GetSyncDevices();
  EXPECT_FALSE(DevicesContains(devices_final.get(), "1", "device1"));
  EXPECT_FALSE(DevicesContains(devices_final.get(), "2", "device2"));
  EXPECT_FALSE(sync_service()->IsSyncConfigured());
}

TEST_F(OnevnSyncServiceTest, OnDeleteDeviceWhenSelfDeleted) {
  sync_service()->sync_prefs_->SetThisDeviceId("1");
  RecordsList records;
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "1", "device1"));
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "2", "device2"));
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(1);
  sync_service()->OnResolvedPreferences(records);

  auto devices = sync_service()->sync_prefs_->GetSyncDevices();

  EXPECT_TRUE(DevicesContains(devices.get(), "1", "device1"));
  EXPECT_TRUE(DevicesContains(devices.get(), "2", "device2"));

  EXPECT_CALL(*sync_client(), SendSyncRecords("PREFERENCES",
      ContainsDeviceRecord(SyncRecord::Action::A_DELETE, "device1"))).Times(1);
  sync_service()->OnDeleteDevice("1");

  RecordsList resolved_records;
  auto resolved_record = SyncRecord::Clone(*records.at(0));
  resolved_record->action = SyncRecord::Action::A_DELETE;
  resolved_records.push_back(std::move(resolved_record));
  // If you have to modify .Times(3) to another value, double re-check
  EXPECT_CALL(*observer(), OnSyncStateChanged(sync_service())).Times(3);
  sync_service()->OnResolvedPreferences(resolved_records);

  auto devices_final = sync_service()->sync_prefs_->GetSyncDevices();
  EXPECT_FALSE(DevicesContains(devices_final.get(), "1", "device1"));
  EXPECT_FALSE(DevicesContains(devices_final.get(), "2", "device2"));

  EXPECT_FALSE(sync_service()->IsSyncConfigured());
}

TEST_F(OnevnSyncServiceTest, OnResetSync) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged).Times(AtLeast(1));
  EXPECT_CALL(*observer(),
      OnSyncStateChanged(sync_service())).Times(AtLeast(3));
  sync_service()->OnSetupSyncNewToSync("this_device");
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncEnabled));
  sync_service()->sync_prefs_->SetThisDeviceId("0");

  RecordsList records;
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "0", "this_device"));
  records.push_back(SimpleDeviceRecord(
      SyncRecord::Action::A_CREATE,
      "1", "device1"));

  sync_service()->OnResolvedPreferences(records);

  auto devices = sync_service()->sync_prefs_->GetSyncDevices();

  EXPECT_TRUE(DevicesContains(devices.get(), "0", "this_device"));
  EXPECT_TRUE(DevicesContains(devices.get(), "1", "device1"));

  sync_service()->OnResetSync();
  RecordsList resolved_records;
  auto resolved_record = SyncRecord::Clone(*records.at(0));
  resolved_record->action = SyncRecord::Action::A_DELETE;
  resolved_records.push_back(std::move(resolved_record));
  sync_service()->OnResolvedPreferences(resolved_records);

  auto devices_final = sync_service()->sync_prefs_->GetSyncDevices();
  EXPECT_FALSE(DevicesContains(devices_final.get(), "0", "this_device"));
  EXPECT_FALSE(DevicesContains(devices_final.get(), "1", "device1"));

  EXPECT_TRUE(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncDeviceId).empty());
  EXPECT_TRUE(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncSeed).empty());
  EXPECT_TRUE(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncDeviceName).empty());
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncEnabled));
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncBookmarksEnabled));
  EXPECT_TRUE(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncBookmarksBaseOrder).empty());
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncSiteSettingsEnabled));
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncHistoryEnabled));
  EXPECT_TRUE(profile()->GetPrefs()->GetTime(
      onevn_sync::prefs::kSyncLatestRecordTime).is_null());
  EXPECT_TRUE(profile()->GetPrefs()->GetTime(
      onevn_sync::prefs::kSyncLastFetchTime).is_null());
  EXPECT_TRUE(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncDeviceList).empty());
  EXPECT_EQ(profile()->GetPrefs()->GetString(
      onevn_sync::prefs::kSyncApiVersion), "0");

  EXPECT_FALSE(sync_service()->IsSyncInitialized());
  EXPECT_FALSE(sync_service()->IsSyncConfigured());
}

TEST_F(OnevnSyncServiceTest, OnSetSyncBookmarks) {
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncBookmarksEnabled));
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(1);
  sync_service()->OnSetSyncBookmarks(true);
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncBookmarksEnabled));
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(1);
  sync_service()->OnSetSyncBookmarks(false);
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncBookmarksEnabled));
}

TEST_F(OnevnSyncServiceTest, OnSetSyncBrowsingHistory) {
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncHistoryEnabled));
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(1);
  sync_service()->OnSetSyncBrowsingHistory(true);
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncHistoryEnabled));
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(1);
  sync_service()->OnSetSyncBrowsingHistory(false);
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncHistoryEnabled));
}

TEST_F(OnevnSyncServiceTest, OnSetSyncSavedSiteSettings) {
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncSiteSettingsEnabled));
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(1);
  sync_service()->OnSetSyncSavedSiteSettings(true);
  EXPECT_TRUE(profile()->GetPrefs()->GetBoolean(
       onevn_sync::prefs::kSyncSiteSettingsEnabled));
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(1);
  sync_service()->OnSetSyncSavedSiteSettings(false);
  EXPECT_FALSE(profile()->GetPrefs()->GetBoolean(
      onevn_sync::prefs::kSyncSiteSettingsEnabled));
}

TEST_F(OnevnSyncServiceTest, OnGetInitData) {
  EXPECT_CALL(*sync_client(), SendGotInitData).Times(1);
  sync_service()->OnGetInitData("v1.4.2");
}

TEST_F(OnevnSyncServiceTest, OnSaveBookmarksBaseOrder) {
  sync_service()->OnSaveBookmarksBaseOrder("1.1.");
  EXPECT_EQ(profile()->GetPrefs()->GetString(
       onevn_sync::prefs::kSyncBookmarksBaseOrder), "1.1.");
}

TEST_F(OnevnSyncServiceTest, OnSyncPrefsChanged) {
  EXPECT_CALL(*sync_client(), OnSyncEnabledChanged).Times(1);
  EXPECT_CALL(*observer(), OnSyncStateChanged);
  sync_service()->OnSyncPrefsChanged(onevn_sync::prefs::kSyncEnabled);
}

TEST_F(OnevnSyncServiceTest, OnSyncReadyAlreadyWithSync) {
  EXPECT_FALSE(sync_service()->IsSyncInitialized());
  profile()->GetPrefs()->SetString(
                           onevn_sync::prefs::kSyncBookmarksBaseOrder, "1.1.");
  // OnSyncPrefsChanged => OnSyncStateChanged for
  // kSyncSiteSettingsEnabled (1)  and kSyncDeviceList (2)
  EXPECT_CALL(*observer(), OnSyncStateChanged).Times(2);
  profile()->GetPrefs()->SetBoolean(
                            onevn_sync::prefs::kSyncSiteSettingsEnabled, true);
  profile()->GetPrefs()->SetTime(
                     onevn_sync::prefs::kSyncLastFetchTime, base::Time::Now());
  const char* devices_json = R"(
    {
       "devices":[
          {
             "device_id":"0",
             "last_active":1552993896717.0,
             "name":"Device1",
             "object_id":"186, 247, 230, 75, 57, 111, 76, 166, 51, 142, 217, 221, 219, 237, 229, 235"
          },
          {
             "device_id":"1",
             "last_active":1552993909257.0,
             "name":"Device2",
             "object_id":"36, 138, 200, 221, 191, 81, 214, 65, 134, 48, 55, 119, 162, 93, 33, 226"
          }
       ]
    }
  )";

  profile()->GetPrefs()->SetString(
                    onevn_sync::prefs::kSyncDeviceList, devices_json);
  EXPECT_CALL(*sync_client(), SendFetchSyncRecords).Times(1);
  EXPECT_CALL(*sync_client(), SendFetchSyncDevices).Times(1);
  sync_service()->OnSyncReady();
  EXPECT_TRUE(sync_service()->IsSyncInitialized());
}

TEST_F(OnevnSyncServiceTest, OnSyncReadyNewToSync) {
  EXPECT_CALL(*observer(), OnSyncStateChanged);
  profile()->GetPrefs()->SetBoolean(
                            onevn_sync::prefs::kSyncSiteSettingsEnabled, true);
  EXPECT_CALL(*sync_client(), SendGetBookmarksBaseOrder).Times(1);
  sync_service()->OnSyncReady();
}

TEST_F(OnevnSyncServiceTest, OnGetExistingObjects) {
  EXPECT_CALL(*sync_client(), SendResolveSyncRecords).Times(1);

  auto records = std::make_unique<RecordsList>();
  sync_service()->OnGetExistingObjects(onevn_sync::jslib_const::kBookmarks,
      std::move(records),
      base::Time(),
      false);
}

TEST_F(OnevnSyncServiceTest, BackgroundSyncStarted) {
  sync_service()->BackgroundSyncStarted(false);
  EXPECT_TRUE(sync_service()->timer_->IsRunning());
}

TEST_F(OnevnSyncServiceTest, BackgroundSyncStopped) {
  sync_service()->BackgroundSyncStopped(false);
  EXPECT_FALSE(sync_service()->timer_->IsRunning());
}
