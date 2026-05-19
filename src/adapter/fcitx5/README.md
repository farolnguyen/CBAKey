# Fcitx5 Adapter

This directory contains the integration bridge used by Phase 2.

## Current Components
- `bridge.cpp`: thin lifecycle bridge from key events to `cbakey::core::Engine`.
- `createBridgeFromConfigFile(...)`: loads runtime config and creates adapter bridge.
- `tests/fcitx5_bridge_smoke_test.cpp`: validates preedit/commit and EN/VN toggle flow.
- `cbakey_fcitx5_engine.cpp`: real Fcitx5 addon entrypoint (`FCITX_ADDON_FACTORY`).

## Boundary Rules
- Keep adapter thin: only translate framework events to `cbakey::core::KeyEvent`.
- Do not implement Telex/VNI rules in adapter code.
- Preedit/commit state must be forwarded directly from core output.

## Build Notes
- Requires pkg-config modules: `Fcitx5Core`, `Fcitx5Utils`, `Fcitx5Config`, `Fcitx5Module`.
- Build target: `cbakey_fcitx5plugin` (module output `build/cbakey.so`).
- Config file path used by plugin: `~/.config/cbakey/cbakey.conf`.
