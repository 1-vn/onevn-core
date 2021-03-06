#!/usr/bin/env python3
import argparse
import logging
import re
import sys

from argparse import RawTextHelpFormatter

from lib.changelog import *


def main():
    """
    Download the onevn-browser/CHANGELOG.md file, parse it, and
    return either markdown or html for a particular Onevn tag.

    Example:
    python script/changelog.py -t refs/tags/v0.61.51 \
        -u https://raw.githubusercontent.com/onevn/onevn-browser/master/CHANGELOG.md -o markdown

    ## [0.61.51](https://github.com/1-vn/onevn-browser/releases/tag/v0.61.51)

    - Added new setting that allows Onevn Rewards icon in the URL to be hidden if Rewards \
        is inactive. ([#2975](https://github.com/1-vn/onevn-browser/issues/2975))

    """

    args = parse_args()

    if args.debug:
        logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)

    changelog_url = args.url

    tag = args.tag

    if not re.match(r'^refs/tags/', tag) and not re.match(r'^v', tag):
        logging.error(" Tag prefix must contain {} or {}".format("\"refs/tags/\"", "\"v\""))
        exit(1)

    match = re.match(r'^refs/tags/(.*)$', tag)
    if match:
        tag = match.group(1)

    match = re.match(r'^v(.*)$', tag)
    if match:
        version = match.group(1)

    logging.debug("CHANGELOG_URL: {}".format(changelog_url))
    logging.debug("TAG: {}".format(tag))
    logging.debug("VERSION: {}".format(version))

    changelog_txt = download_from_url(args, logging, changelog_url)

    if args.output == 'markdown':
        print(render_markdown(changelog_txt, version, logging))
    elif args.output == 'html':
        print(render_html(changelog_txt, version, logging))


def parse_args():
    desc = "Parse Onevn Browser changelog and return changes for a tag"

    parser = argparse.ArgumentParser(
        description=desc, formatter_class=RawTextHelpFormatter)
    parser.add_argument('-d', '--debug', action='store_true',
                        help='Print debug statements')
    parser.add_argument('-o', '--output', help='Output format: markdown or html', required=True)
    parser.add_argument('-t', '--tag',
                        help='Onevn version tag (allowed format: "v0.60.45" or "refs/tags/v0.60.45")', required=True)
    parser.add_argument('-u', '--url', help='URL for Onevn Browser raw markdown file (required)', required=True)
    return parser.parse_args()


if __name__ == '__main__':
    sys.exit(main())
