/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/path_service.h"
#include "base/strings/pattern.h"
#include "base/strings/utf_string_conversions.h"
#include "onevn/common/onevn_paths.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_list_observer.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/common/url_constants.h"
#include "content/public/test/browser_test_utils.h"
#include "net/dns/mock_host_resolver.h"

class OnevnSchemeLoadBrowserTest : public InProcessBrowserTest,
                                   public BrowserListObserver,
                                   public TabStripModelObserver {
 public:
  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();
    host_resolver()->AddRule("*", "127.0.0.1");

    onevn::RegisterPathProvider();
    base::FilePath test_data_dir;
    base::PathService::Get(onevn::DIR_TEST_DATA, &test_data_dir);
    embedded_test_server()->ServeFilesFromDirectory(test_data_dir);

    ASSERT_TRUE(embedded_test_server()->Start());
  }

  // BrowserListObserver overrides:
  void OnBrowserAdded(Browser* browser) override { popup_ = browser; }

  // TabStripModelObserver overrides:
  void OnTabStripModelChanged(
      TabStripModel* tab_strip_model,
      const TabStripModelChange& change,
      const TabStripSelectionChange& selection) override {
    if (change.type() == TabStripModelChange::kInserted) {
      WaitForLoadStop(active_contents());
    }
  }

  content::WebContents* active_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  bool NavigateToURLUntilLoadStop(const std::string& origin,
                                  const std::string& path) {
    ui_test_utils::NavigateToURL(browser(),
                                 embedded_test_server()->GetURL(origin, path));
    return WaitForLoadStop(active_contents());
  }

  // Check loading |url| in private window is redirected to normal
  // window.
  void TestURLIsNotLoadedInPrivateWindow(const std::string& url) {
    Browser* private_browser = CreateIncognitoBrowser(nullptr);
    TabStripModel* private_model = private_browser->tab_strip_model();

    // Check normal & private window have one blank tab.
    EXPECT_EQ("about:blank",
              private_model->GetActiveWebContents()->GetVisibleURL().spec());
    EXPECT_EQ(1, private_model->count());
    EXPECT_EQ("about:blank", active_contents()->GetVisibleURL().spec());
    EXPECT_EQ(1, browser()->tab_strip_model()->count());

    browser()->tab_strip_model()->AddObserver(this);

    // Load url to private window.
    NavigateParams params(
        private_browser, GURL(url), ui::PAGE_TRANSITION_TYPED);
    Navigate(&params);

    browser()->tab_strip_model()->RemoveObserver(this);

    EXPECT_STREQ(url.c_str(),
                 base::UTF16ToUTF8(browser()->location_bar_model()
                      ->GetFormattedFullURL()).c_str());
    // EXPECT_EQ(url, active_contents()->GetVisibleURL());
    EXPECT_EQ(2, browser()->tab_strip_model()->count());
    // Private window stays as initial state.
    EXPECT_EQ("about:blank",
              private_model->GetActiveWebContents()->GetVisibleURL().spec());
    EXPECT_EQ(1, private_browser->tab_strip_model()->count());
  }

  Browser* popup_ = nullptr;
};

// Test whether onevn page is not loaded from different host by window.open().
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest, NotAllowedToLoadTest) {
  EXPECT_TRUE(
      NavigateToURLUntilLoadStop("example.com", "/onevn_scheme_load.html"));
  content::ConsoleObserverDelegate console_delegate(
      active_contents(), "Not allowed to load local resource:*");
  active_contents()->SetDelegate(&console_delegate);

  ASSERT_TRUE(ExecuteScript(
      active_contents(),
      "window.domAutomationController.send(openOnevnSettings())"));
  console_delegate.Wait();

  EXPECT_TRUE(base::MatchPattern(
      console_delegate.message(),
      "Not allowed to load local resource: onevn://settings/"));
}

// Test whether onevn page is not loaded from different host by window.open().
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       NotAllowedToLoadTestByWindowOpenWithNoOpener) {
  BrowserList::GetInstance()->AddObserver(this);

  EXPECT_TRUE(
      NavigateToURLUntilLoadStop("example.com", "/onevn_scheme_load.html"));

  ASSERT_TRUE(ExecuteScript(
      active_contents(),
      "window.domAutomationController.send(openOnevnSettingsWithNoOpener())"));

  auto* popup_tab = popup_->tab_strip_model()->GetActiveWebContents();

  WaitForLoadStop(popup_tab);

  // Loading onevn page should be blocked in new window.
  DCHECK_EQ(popup_tab->GetVisibleURL().spec(), content::kBlockedURL);

  BrowserList::GetInstance()->RemoveObserver(this);
}

// Test whether onevn page is not loaded from different host directly by
// location.replace().
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       NotAllowedToDirectReplaceTest) {
  EXPECT_TRUE(
      NavigateToURLUntilLoadStop("example.com", "/onevn_scheme_load.html"));
  content::ConsoleObserverDelegate console_delegate(
      active_contents(), "Not allowed to load local resource:*");
  active_contents()->SetDelegate(&console_delegate);

  ASSERT_TRUE(ExecuteScript(
      active_contents(),
      "window.domAutomationController.send(replaceToOnevnSettingsDirectly())"));
  console_delegate.Wait();
  EXPECT_TRUE(base::MatchPattern(
      console_delegate.message(),
      "Not allowed to load local resource: onevn://settings/"));
}

// Test whether onevn page is not loaded from different host indirectly by
// location.replace().
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       NotAllowedToIndirectReplaceTest) {
  EXPECT_TRUE(
      NavigateToURLUntilLoadStop("example.com", "/onevn_scheme_load.html"));
  auto* initial_active_tab = active_contents();
  content::ConsoleObserverDelegate console_delegate(
      initial_active_tab, "Not allowed to load local resource:*");

  ASSERT_TRUE(ExecuteScript(initial_active_tab,
                            "window.domAutomationController.send("
                            "replaceToOnevnSettingsIndirectly())"));

  initial_active_tab->SetDelegate(&console_delegate);
  console_delegate.Wait();

  EXPECT_TRUE(base::MatchPattern(
      console_delegate.message(),
      "Not allowed to load local resource: onevn://settings/"));
}

// Test whether onevn page is not loaded from chrome page.
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       NotAllowedToOnevnFromChrome) {
  NavigateToURLBlockUntilNavigationsComplete(active_contents(),
                                             GURL("chrome://version"), 1);

  content::ConsoleObserverDelegate console_delegate(
      active_contents(), "Not allowed to load local resource:*");
  active_contents()->SetDelegate(&console_delegate);

  ASSERT_TRUE(
      ExecuteScript(active_contents(), "window.open(\"onevn://settings\")"));
  console_delegate.Wait();
  EXPECT_TRUE(base::MatchPattern(
      console_delegate.message(),
      "Not allowed to load local resource: onevn://settings/"));
}

// Test whether onevn page is not loaded by click.
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest, NotAllowedToOnevnByClick) {
  EXPECT_TRUE(
      NavigateToURLUntilLoadStop("example.com", "/onevn_scheme_load.html"));
  content::ConsoleObserverDelegate console_delegate(
      active_contents(), "Not allowed to load local resource:*");
  active_contents()->SetDelegate(&console_delegate);

  ASSERT_TRUE(ExecuteScript(
      active_contents(),
      "window.domAutomationController.send(gotoOnevnSettingsByClick())"));
  console_delegate.Wait();
  EXPECT_TRUE(base::MatchPattern(
      console_delegate.message(),
      "Not allowed to load local resource: onevn://settings/"));
}

// Test whether onevn page is not loaded by middle click.
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       NotAllowedToOnevnByMiddleClick) {
  EXPECT_TRUE(
      NavigateToURLUntilLoadStop("example.com", "/onevn_scheme_load.html"));
  content::ConsoleObserverDelegate console_delegate(
      active_contents(), "Not allowed to load local resource:*");
  active_contents()->SetDelegate(&console_delegate);

  ASSERT_TRUE(ExecuteScript(
      active_contents(),
      "window.domAutomationController.send(gotoOnevnSettingsByMiddleClick())"));
  console_delegate.Wait();
  EXPECT_TRUE(base::MatchPattern(
      console_delegate.message(),
      "Not allowed to load local resource: onevn://settings/"));
}

// Check renderer crash happened by observing related notification.
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest, CrashURLTest) {
  content::WindowedNotificationObserver observer(
      content::NOTIFICATION_WEB_CONTENTS_DISCONNECTED,
      content::NotificationService::AllSources());
  browser()->OpenURL(
      content::OpenURLParams(GURL("onevn://crash/"), content::Referrer(),
                             WindowOpenDisposition::CURRENT_TAB,
                             ui::PAGE_TRANSITION_TYPED, false));
  observer.Wait();
}

// Some webuis are not allowed to load in private window.
// Allowed url list are checked by IsURLAllowedInIncognito().
// So, corresponding onevn scheme url should be filtered as chrome scheme.
// Ex, onevn://settings should be loaded only in normal window because
// chrome://settings is not allowed. When tyring to loading onevn://settings in
// private window, it should be loaded in normal window instead of private
// window.
IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       SettingsPageIsNotAllowedInPrivateWindow) {
  TestURLIsNotLoadedInPrivateWindow("onevn://settings");
}

IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       SyncPageIsNotAllowedInPrivateWindow) {
  TestURLIsNotLoadedInPrivateWindow("onevn://sync");
}

IN_PROC_BROWSER_TEST_F(OnevnSchemeLoadBrowserTest,
                       RewardsPageIsNotAllowedInPrivateWindow) {
  TestURLIsNotLoadedInPrivateWindow("onevn://rewards");
}