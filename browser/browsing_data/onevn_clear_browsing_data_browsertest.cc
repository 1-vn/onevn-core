/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/files/scoped_temp_dir.h"
#include "base/macros.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/task/post_task.h"
#include "base/test/scoped_feature_list.h"
#include "onevn/browser/browsing_data/onevn_clear_browsing_data.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/browsing_data/chrome_browsing_data_remover_delegate.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_window.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_list_observer.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/search_test_utils.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/browsing_data/core/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_features.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"

using content::OnevnClearBrowsingData;
using content::WebContents;

namespace {

class BrowserRemovedObserver : public BrowserListObserver {
 public:
  BrowserRemovedObserver() : removed_browser_(nullptr) {
    BrowserList::AddObserver(this);
    original_browsers_count_ = BrowserList::GetInstance()->size();
  }

  ~BrowserRemovedObserver() override { BrowserList::RemoveObserver(this); }

  Browser* CheckReturn() const {
    EXPECT_EQ(original_browsers_count_ - 1, chrome::GetTotalBrowserCount());
    return removed_browser_;
  }

  // Wait for a new browser to be removed.
  Browser* WaitForBrowserRemoval() {
    if (removed_browser_)
      return CheckReturn();
    run_loop_.Run();
    EXPECT_NE(removed_browser_, nullptr);
    return CheckReturn();
  }

 private:
  // BrowserListObserver
  void OnBrowserRemoved(Browser* browser) override {
    removed_browser_ = browser;
    run_loop_.Quit();
  }

  size_t original_browsers_count_;
  Browser* removed_browser_;
  base::RunLoop run_loop_;

  DISALLOW_COPY_AND_ASSIGN(BrowserRemovedObserver);
};

}  // namespace

class OnevnClearDataOnExitTest
    : public InProcessBrowserTest,
      public OnevnClearBrowsingData::OnExitTestingCallback {
 public:
  OnevnClearDataOnExitTest() = default;
  void SetUpOnMainThread() override {
    OnevnClearBrowsingData::SetOnExitTestingCallback(this);
  }

  void TearDownOnMainThread() override {
    // Borrowed from browser_browsertest.cc.
    // Cycle the MessageLoop: one for each browser.
    for (unsigned int i = 0; i < browsers_count_; ++i)
      content::RunAllPendingInMessageLoop();

    // Run the application event loop to completion, which will cycle the
    // native MessagePump on all platforms.
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::RunLoop::QuitCurrentWhenIdleClosureDeprecated());

    base::RunLoop().Run();

    // Take care of any remaining message loop work.
    content::RunAllPendingInMessageLoop();

    // At this point, quit should be for real now.
    ASSERT_EQ(0u, chrome::GetTotalBrowserCount());
  }

  void TearDownInProcessBrowserTestFixture() override {
    // Verify expected number of calls to remove browsing data.
    EXPECT_EQ(remove_data_call_count_, expected_remove_data_call_count_);
    OnevnClearBrowsingData::SetOnExitTestingCallback(nullptr);
  }

  int remove_data_call_count() { return remove_data_call_count_; }

  void SetExepectedRemoveDataCallCount(int count) {
    expected_remove_data_call_count_ = count;
  }

  void SetExpectedRemoveDataRemovalMasks(int remove_mask, int origin_mask) {
    expected_remove_mask_ = remove_mask;
    expected_origin_mask_ = origin_mask;
  }

  void SetClearAll(PrefService* prefService) {
    prefService->SetBoolean(browsing_data::prefs::kDeleteBrowsingHistoryOnExit,
                            true);
    prefService->SetBoolean(browsing_data::prefs::kDeleteDownloadHistoryOnExit,
                            true);
    prefService->SetBoolean(browsing_data::prefs::kDeleteCacheOnExit, true);
    prefService->SetBoolean(browsing_data::prefs::kDeleteCookiesOnExit, true);
    prefService->SetBoolean(browsing_data::prefs::kDeletePasswordsOnExit, true);
    prefService->SetBoolean(browsing_data::prefs::kDeleteFormDataOnExit, true);
    prefService->SetBoolean(browsing_data::prefs::kDeleteHostedAppsDataOnExit,
                            true);
    prefService->SetBoolean(browsing_data::prefs::kDeleteSiteSettingsOnExit,
                            true);
  }

  int GetRemoveMaskAll() {
    return ChromeBrowsingDataRemoverDelegate::DATA_TYPE_HISTORY |
           content::BrowsingDataRemover::DATA_TYPE_DOWNLOADS |
           content::BrowsingDataRemover::DATA_TYPE_CACHE |
           ChromeBrowsingDataRemoverDelegate::DATA_TYPE_SITE_DATA |
           ChromeBrowsingDataRemoverDelegate::DATA_TYPE_PASSWORDS |
           ChromeBrowsingDataRemoverDelegate::DATA_TYPE_FORM_DATA |
           ChromeBrowsingDataRemoverDelegate::DATA_TYPE_CONTENT_SETTINGS;
  }

  int GetOriginMaskAll() {
    return content::BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB |
           content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB;
  }

  // OnevnClearBrowsingData::OnExitTestingCallback implementation.
  void BeforeClearOnExitRemoveData(content::BrowsingDataRemover* remover,
                                   int remove_mask,
                                   int origin_mask) override {
    remove_data_call_count_++;

    if (expected_remove_mask_ != -1)
      EXPECT_EQ(expected_remove_mask_, remove_mask);
    if (expected_origin_mask_ != -1)
      EXPECT_EQ(expected_origin_mask_, origin_mask);
  }

 protected:
  unsigned int browsers_count_ = 1u;
  int remove_data_call_count_ = 0;
  int expected_remove_data_call_count_ = 0;
  int expected_remove_mask_ = -1;
  int expected_origin_mask_ = -1;

  DISALLOW_COPY_AND_ASSIGN(OnevnClearDataOnExitTest);
};

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTest, NoPrefsSet) {
  // No set preferences to clear data.
  SetExepectedRemoveDataCallCount(0);
  // Tell the application to quit.
  chrome::ExecuteCommand(browser(), IDC_EXIT);
}

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTest, VerifyRemovalMasks) {
  // Set all clear data on exit preferences and corresponding expected remove
  // mask and origin flags.
  SetClearAll(browser()->profile()->GetPrefs());

  // Given those preferences the following removal mask is expected.
  SetExpectedRemoveDataRemovalMasks(GetRemoveMaskAll(), GetOriginMaskAll());

  // Expect a call to clear data.
  SetExepectedRemoveDataCallCount(1);

  // Tell the application to quit.
  chrome::ExecuteCommand(browser(), IDC_EXIT);
}

class OnevnClearDataOnExitTwoBrowsersTest : public OnevnClearDataOnExitTest {
 public:
  OnevnClearDataOnExitTwoBrowsersTest() : OnevnClearDataOnExitTest() {
    browsers_count_ = 2u;
  }

 protected:
  // Open a new browser window with the provided |profile|.
  Browser* NewBrowserWindow(Profile* profile) {
    DCHECK(profile);
    ui_test_utils::BrowserAddedObserver browser_added_observer;
    chrome::NewEmptyWindow(profile);
    Browser* browser = browser_added_observer.WaitForSingleNewBrowser();
    DCHECK(browser);
    content::WaitForLoadStopWithoutSuccessCheck(
        browser->tab_strip_model()->GetActiveWebContents());
    return browser;
  }

  // Open a new browser window with a guest session.
  Browser* NewGuestBrowserWindow() {
    ui_test_utils::BrowserAddedObserver browser_added_observer;
    profiles::SwitchToGuestProfile(ProfileManager::CreateCallback());
    Browser* browser = browser_added_observer.WaitForSingleNewBrowser();
    DCHECK(browser);
    // When a guest |browser| closes a BrowsingDataRemover will be created and
    // executed. It needs a loaded TemplateUrlService or else it hangs on to a
    // CallbackList::Subscription forever.
    Profile* guest = g_browser_process->profile_manager()->GetProfileByPath(
        ProfileManager::GetGuestProfilePath());
    DCHECK(guest);
    search_test_utils::WaitForTemplateURLServiceToLoad(
        TemplateURLServiceFactory::GetForProfile(guest));
    // Navigate to about:blank.
    ui_test_utils::NavigateToURL(browser, GURL(url::kAboutBlankURL));
    return browser;
  }

  // Open a new browser window with a new profile.
  Browser* NewProfileBrowserWindow() {
    base::FilePath path;
    base::PathService::Get(chrome::DIR_USER_DATA, &path);
    path = path.AppendASCII("Profile 2");
    base::ScopedAllowBlockingForTesting allow_blocking;
    // Clean up profile directory when the test is done.
    ignore_result(profile2_dir_.Set(path));
    ProfileManager* profile_manager = g_browser_process->profile_manager();
    size_t starting_number_of_profiles = profile_manager->GetNumberOfProfiles();
    if (!base::PathExists(path) && !base::CreateDirectory(path))
      NOTREACHED() << "Could not create directory at " << path.MaybeAsASCII();
    Profile* profile = profile_manager->GetProfile(path);
    DCHECK(profile);
    EXPECT_EQ(starting_number_of_profiles + 1,
              profile_manager->GetNumberOfProfiles());

    return NewBrowserWindow(profile);
  }

  // Close the provided |browser| window and wait until done.
  void CloseBrowserWindow(Browser* browser) {
    BrowserRemovedObserver bro;
    chrome::ExecuteCommand(browser, IDC_CLOSE_WINDOW);
    EXPECT_EQ(bro.WaitForBrowserRemoval(), browser);
  }

  // Enable deletion of browsing history on exit.
  void SetDeleteBrowsingHistoryOnExit(Profile* profile) {
    profile->GetPrefs()->SetBoolean(
        browsing_data::prefs::kDeleteBrowsingHistoryOnExit, true);
  }

  void SetDeleteBrowsingHistoryOnExit() {
    SetDeleteBrowsingHistoryOnExit(browser()->profile());
  }

 private:
  base::ScopedTempDir profile2_dir_;

  DISALLOW_COPY_AND_ASSIGN(OnevnClearDataOnExitTwoBrowsersTest);
};

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTwoBrowsersTest, SameProfile) {
  // Delete browsing history on exit.
  SetDeleteBrowsingHistoryOnExit();
  // Same profile, so expect a single call.
  SetExepectedRemoveDataCallCount(1);

  // Open a second browser window.
  Browser* second_window = NewBrowserWindow(browser()->profile());
  // Close second browser window
  CloseBrowserWindow(second_window);
  EXPECT_EQ(0, remove_data_call_count());

  // Tell the application to quit.
  chrome::ExecuteCommand(browser(), IDC_EXIT);
}

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTwoBrowsersTest, OneOTR) {
  // Delete browsing history on exit.
  SetDeleteBrowsingHistoryOnExit();
  // OTR sessions don't count, so expect a single call.
  SetExepectedRemoveDataCallCount(1);

  // Open a second browser window with OTR profile.
  Browser* second_window =
      NewBrowserWindow(browser()->profile()->GetOffTheRecordProfile());
  // Close second browser window
  CloseBrowserWindow(second_window);
  EXPECT_EQ(0, remove_data_call_count());

  // Tell the application to quit.
  chrome::ExecuteCommand(browser(), IDC_EXIT);
}

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTwoBrowsersTest, OneOTRExitsLast) {
  // Delete browsing history on exit.
  SetDeleteBrowsingHistoryOnExit();
  // OTR sessions don't count, so expect a single call.
  SetExepectedRemoveDataCallCount(1);

  // Open a second browser window with OTR profile.
  Browser* second_window =
      NewBrowserWindow(browser()->profile()->GetOffTheRecordProfile());

  // Close regular profile window.
  CloseBrowserWindow(browser());
  EXPECT_EQ(0, remove_data_call_count());

  // Tell the application to quit.
  chrome::ExecuteCommand(second_window, IDC_EXIT);
}

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTwoBrowsersTest, OneGuest) {
  // Delete browsing history on exit.
  SetDeleteBrowsingHistoryOnExit();
  // Guest sessions don't count, so expect a single call.
  SetExepectedRemoveDataCallCount(1);

  // Open a second browser window with Guest session.
  Browser* guest_window = NewGuestBrowserWindow();

  // Close Guest session window: regular profile cleanup shouldn't happen.
  CloseBrowserWindow(guest_window);
  EXPECT_EQ(0, remove_data_call_count());

  // Tell the application to quit.
  chrome::ExecuteCommand(browser(), IDC_EXIT);
}

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTwoBrowsersTest, OneGuestExitsLast) {
  // Delete browsing history on exit.
  SetDeleteBrowsingHistoryOnExit();
  // Guest sessions don't count, so expect a single call.
  SetExepectedRemoveDataCallCount(1);

  // Open a second browser window with Guest session.
  Browser* guest_window = NewGuestBrowserWindow();

  // Close regular profile window.
  CloseBrowserWindow(browser());
  EXPECT_EQ(0, remove_data_call_count());

  // Tell the application to quit.
  chrome::ExecuteCommand(guest_window, IDC_EXIT);
}

IN_PROC_BROWSER_TEST_F(OnevnClearDataOnExitTwoBrowsersTest, TwoProfiles) {
  // Delete browsing history on exit.
  SetDeleteBrowsingHistoryOnExit();

  // Open a second browser window with a different profile.
  Browser* second_profile_window = NewProfileBrowserWindow();
  DCHECK(second_profile_window);
  // Delete browsing history for this profile on exit too.
  Profile* second_profile = second_profile_window->profile();
  SetDeleteBrowsingHistoryOnExit(second_profile);

  // Both profiles have browsing data removal set, so expect two calls.
  SetExepectedRemoveDataCallCount(2);

  // Close second profile window.
  CloseBrowserWindow(second_profile_window);
  EXPECT_EQ(0, remove_data_call_count());

  // Tell the application to quit.
  chrome::ExecuteCommand(browser(), IDC_EXIT);
}
