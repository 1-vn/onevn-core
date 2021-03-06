/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { action } from 'typesafe-actions'

// Constants
import { types } from '../constants/welcome_types'

export const importNowRequested = () => action(types.IMPORT_NOW_REQUESTED)

export const goToTabRequested = (url: string, target: string) => action(types.GO_TO_TAB_REQUESTED, {
  url,
  target
})

export const closeTabRequested = () => action(types.CLOSE_TAB_REQUESTED)
