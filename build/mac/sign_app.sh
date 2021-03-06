#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt "4" ]]; then
  echo "usage: $0 <source_app> <dest_app> <packaging_dir> <mac_signing_keychain> <mac_signing_identifier> <mac_provisioning_profile> <extra_flags>"
  exit 1
fi

SOURCE="${1}"
DEST="${2}"
PKG_DIR="${3}"
MAC_SIGNING_KEYCHAIN="${4}"
MAC_SIGNING_IDENTIFIER="${5}"
MAC_PROVISIONING_PROFILE="${6}"
MAC_ENTITLEMENTS_PLIST="${7}"

function check_exit() {
    return=$?;
    if [[ $return -eq 0 ]]; then
  echo "[INFO] $0 succeded"
    else
  echo "[ERROR] $0 failed"
    fi

    exit $return
}

trap check_exit EXIT

if [[ -d "$DEST" ]]; then
  rm -rf "$DEST"
fi

tmpdir=$(dirname "$DEST")
mkdir -p "$tmpdir"

cp -a "$SOURCE" "$DEST"

if [[ ${#} -eq 8 ]]; then
EXTRA_FLAGS="${8}"
"${PKG_DIR}/sign_versioned_dir.sh" "$DEST" "$MAC_SIGNING_KEYCHAIN" "$MAC_SIGNING_IDENTIFIER" "$EXTRA_FLAGS"
"${PKG_DIR}/sign_app.sh" "$DEST" "$MAC_SIGNING_KEYCHAIN" "$MAC_SIGNING_IDENTIFIER" "$MAC_PROVISIONING_PROFILE" "${MAC_ENTITLEMENTS_PLIST}" "$EXTRA_FLAGS"
else
"${PKG_DIR}/sign_versioned_dir.sh" "$DEST" "$MAC_SIGNING_KEYCHAIN" "$MAC_SIGNING_IDENTIFIER"
"${PKG_DIR}/sign_app.sh" "$DEST" "$MAC_SIGNING_KEYCHAIN" "$MAC_SIGNING_IDENTIFIER" "$MAC_PROVISIONING_PROFILE" "${MAC_ENTITLEMENTS_PLIST}"
fi
