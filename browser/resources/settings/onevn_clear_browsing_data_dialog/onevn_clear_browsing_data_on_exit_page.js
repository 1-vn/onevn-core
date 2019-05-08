/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

(function() {
'use strict';

Polymer({
  is: 'settings-onevn-clear-browsing-data-on-exit-page',

  properties: {
    prefs: {
      type: Object,
      notify: true,
    },

    counters: {
      type: Object,
      // Will be filled as results are reported.
      value: function() {
        return {};
      }
    },

    isModified: {
      type: Boolean,
      value: false,
    },

    isSupervised_: {
      type: Boolean,
      value: function() {
        return loadTimeData.getBoolean('isSupervised');
      },
    },

  },

  listeners: {'settings-boolean-control-change': 'updateModified_'},

  setCounter: function (counter, text) {
    this.set('counters.' + counter, text);
  },

  getChangedSettings: function () {
    let changed = [];
    const boxes = this.$.checkboxes.querySelectorAll('settings-checkbox');
    boxes.forEach((checkbox) => {
      if (checkbox.checked != this.get(checkbox.pref.key, this.prefs).value) {
        changed.push({key:checkbox.pref.key, value:checkbox.checked});
      }
    });
    return changed;
  },

  siteSettingsLabel_: function (siteSettings, contentSettings) {
    return loadTimeData.getBoolean('enableSiteSettings') ? siteSettings :
                                                           contentSettings;
  },

  updateModified_: function (checkbox) {
    let modified = false;
    const boxes = this.$.checkboxes.querySelectorAll('settings-checkbox');
    for (let checkbox of boxes) {
      if (checkbox.checked != this.get(checkbox.pref.key, this.prefs).value) {
        modified = true;
        break;
      }
    }

    if (this.isModified !== modified) {
      this.isModified = modified;
      this.fire('clear-data-on-exit-page-change');
    }
  },
});
})();