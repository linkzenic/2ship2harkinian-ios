# 2Ship 2 Harkinian for iOS

An iPhone, iPad, and Apple TV port of 2Ship 2 Harkinian, based on the Linkzenic fork and the
open-source HarbourMasters project.

## Status

This is an early testing release. Current iOS work includes:

- full-resolution Metal rendering on Retina displays
- landscape iPhone and iPad support
- on-screen touch controls with dual analog sticks
- physical-controller support
- mobile-scaled Linkzenic settings menus
- optional iCloud save synchronization
- iOS and tvOS app icons
- Apple TV local-network uploads for game data, saves, mods, and presets
- controller-navigable Apple TV settings

## Game data

This repository and its releases do not provide a Majora's Mask ROM. Users must
supply game data from a legally obtained copy. Do not open issues requesting
copyrighted game files.

The build can optionally embed an existing `2ship.o2r` support archive:

```sh
TWO_SHIP_O2R_PATH=/absolute/path/to/2ship.o2r scripts/build-ios.sh --device
```

## Building

Requirements:

- macOS with Xcode
- CMake 3.26 or newer
- an Apple Silicon Mac for the default simulator configuration

Simulator:

```sh
scripts/build-ios.sh --simulator
```

Unsigned device build:

```sh
scripts/build-ios.sh --device
scripts/package-ios-unsigned.sh
```

Unsigned Apple TV build:

```sh
scripts/build-ios.sh --tvos-device
scripts/package-ios-unsigned.sh \
  "build-tvos-device/mm/Release-appletvos/2Ship 2 Harkinian.app" \
  "dist/2Ship-2-Harkinian-tvOS-unsigned.ipa"
```

Unsigned IPAs do not contain a developer certificate or provisioning profile.
To install one, sign it with your own Apple account and provisioning profile
using your preferred sideloading tool.

For a locally signed device build, provide your own Apple development team:

```sh
DEVELOPMENT_TEAM=YOUR_TEAM_ID scripts/build-ios.sh --device
```

## iCloud saves

iCloud requires an iCloud container belonging to the Apple developer team used
to sign the app. Builders can select their own container:

```sh
TWO_SHIP_ICLOUD_CONTAINER_ID=iCloud.example.your-container \
DEVELOPMENT_TEAM=YOUR_TEAM_ID \
scripts/build-ios.sh --device
```

Apps signed by unrelated developer teams cannot share the same private iCloud
container. Local app storage continues to work without shared iCloud access.

## Privacy and release policy

Public artifacts must remain unsigned. The repository intentionally excludes:

- certificates, private keys, and provisioning profiles
- developer names, email addresses, team identifiers, and device identifiers
- save files, settings, ROMs, and user-installed mods
- local build paths and build directories

## Credits and license

Based on [HarbourMasters/2ship2harkinian](https://github.com/HarbourMasters/2ship2harkinian)
and the community Android port lineage. Upstream copyright notices and licenses
are preserved in this source tree. See [LICENSE](LICENSE) and the licenses in
the vendored component directories.

This project is not affiliated with or endorsed by Nintendo.
