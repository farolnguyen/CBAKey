# Fcitx5 Adapter

This directory contains the integration bridge used by Phase 2.

## Current Components
- `bridge.cpp`: thin lifecycle bridge from key events to `farolkey::core::Engine`.
- `createBridgeFromConfigFile(...)`: loads runtime config and creates adapter bridge.
- `tests/fcitx5_bridge_smoke_test.cpp`: validates preedit/commit and EN/VN toggle flow.
- `farolkey_fcitx5_engine.cpp`: real Fcitx5 addon entrypoint (`FCITX_ADDON_FACTORY`).

## Boundary Rules
- Keep adapter thin: only translate framework events to `farolkey::core::KeyEvent`.
- Do not implement Telex/VNI rules in adapter code.
- Preedit/commit state must be forwarded directly from core output.

## Build Notes
- Requires pkg-config modules: `Fcitx5Core`, `Fcitx5Utils`, `Fcitx5Config`, `Fcitx5Module`.
- Build target: `farolkey_fcitx5plugin` (module output `build/farolkey.so`).
- Config file path used by plugin: `~/.config/farolkey/farolkey.conf`.
