# NSO SNES Controller ZL/ZR Support

## TL;DR

The Nintendo Switch Online SNES controller presents as two separate SDL devices on macOS simultaneously, and neither device exposes ZL/ZR through the SDL GameController API. A raw HID fallback channel was added to read ZL/ZR directly from the Nintendo USB HID device, running alongside the existing SDL joystick path which continues to handle all other buttons. To reliably identify which saved config bindings belong to which device across reconnects, joystick identification was changed from SDL's enumeration index (unstable) to a VID:PID hash (stable). That config key change exposed two latent bugs in the Input Config dialog — a read/write direction error and a spurious Qt signal during initialisation — which were also fixed. The change touches 11 files across the SDL frontend, input config UI, and the KH plugin system.

---

## Background

The Nintendo Switch Online SNES controller adds ZL and ZR to the original SNES button set. These two inputs are present on the physical hardware and expected to work in Kingdom Hearts: Days and Re:Coded through their existing plugin joystick mapping. Prior to this change they were silently undetectable: the SDL GameController API returned zero for both triggers regardless of which SDL device was active.

The fix required understanding why the GC API fails for this specific controller on macOS, implementing a targeted raw HID fallback that activates only when the GC path genuinely has nothing to offer, and threading that fallback through both the input processing path (`inputProcess()`) and the binding detection UI (`JoyMapButton::checkJoystick()`). Along the way, the input config dialog contained two pre-existing bugs that had been masked by an unrelated error in config section addressing — both were fixed as part of making the overall feature work correctly.

**Files changed**: `main.cpp`, `EmuInstance.h`, `EmuInstance.cpp`, `EmuInstanceInput.cpp`, `InputConfig/InputConfigDialog.h`, `InputConfig/InputConfigDialog.cpp`, `InputConfig/MapButton.h`, `plugins/Plugin.h`, `plugins/Plugin.cpp`, `plugins/PluginKingdomHeartsDays.cpp`, `plugins/PluginKingdomHeartsReCoded.cpp`.

---

## Why ZL/ZR Required a Raw HID Fallback

### The dual-device problem

On macOS, SDL 2.x enumerates the NSO SNES controller as two independent devices simultaneously:

- A **HIDAPI device** with Nintendo's USB identifiers (VID `0x057E`, product `0x2017`) — this communicates directly over USB HID via SDL's built-in HIDAPI backend.
- A **CoreHID device** with Apple's identifiers (VID `0x05AC`) — this is presented by macOS's IOKit GameController framework, which wraps the hardware in its own abstraction layer.

This dual enumeration is an intentional SDL behaviour for Nintendo hardware on macOS. Both devices appear as separate joysticks in `SDL_NumJoysticks()`, each with their own SDL device index and GameController handle.

### Why the GameController API returns zero for ZL/ZR

SDL's GameController layer maps raw HID buttons and axes to a standard layout defined in `gamecontrollerdb.txt`. The SNES controller mapping in that database covers A, B, X, Y, the D-pad, and the shoulder buttons L and R — the original SNES layout. ZL and ZR are additions that the NSO SNES controller provides beyond the original hardware. They do not appear in the GC mapping. As a result, `SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)` returns 0 for both SDL devices — the HIDAPI device because the mapping says nothing about triggers, and the CoreHID device for the same reason.

### Why the SDL HIDAPI hints are required

Without `SDL_HINT_JOYSTICK_HIDAPI=1` and `SDL_HINT_JOYSTICK_HIDAPI_NINTENDO_CLASSIC=1` set before `SDL_Init()`, SDL on macOS routes everything through CoreHID and never creates the HIDAPI device entry. The Nintendo VID:PID device used for HID fallback only appears in `SDL_NumJoysticks()` when these hints are active. Both hints are set in `main.cpp` immediately before the SDL init sequence:

```cpp
SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "1");
SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_NINTENDO_CLASSIC, "1");
```

### Why the HID channel is opened alongside the existing SDL joystick handle

The approach adds a separate `SDL_hid_device*` handle used exclusively for ZL/ZR detection, rather than replacing the existing SDL joystick path. The CoreHID device is what SDL maps all the standard buttons through, and those mappings are what existing user config files contain. Switching to a pure HIDAPI joystick would change the device that SDL uses for A/B/X/Y and D-pad, altering the device indices and invalidating stored bindings. The HID channel is additive — the existing joystick path is left completely intact.

---

## Raw HID Channel Implementation

### When to open it

The raw HID channel is opened conditionally inside `openJoystick()`, guarded by a check of the active controller's GameController descriptor:

```cpp
SDL_GameControllerButtonBind trigBind =
    SDL_GameControllerGetBindForAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
if (trigBind.bindType == SDL_CONTROLLER_BINDTYPE_NONE)
{
    hidDevice = SDL_hid_open(0x057E, 0x2017, NULL);
    // drain + non-blocking setup...
}
```

`SDL_GameControllerGetBindForAxis` returns `BINDTYPE_NONE` only when the GameController descriptor genuinely has no trigger mapping — which is exactly the CoreHID case for the NSO SNES controller. When the HIDAPI device is active (which does have trigger mappings from the Nintendo GC database entry), this condition is false and the raw HID channel is never opened. This ensures the HID path is not entered for controllers that don't need it.

### The wakeup report problem

When the HID channel is first opened, the NSO SNES controller sends a single report with the ZL bit set. This is a known Nintendo HID protocol behaviour: the controller signals that it is ready by transmitting a report that is byte-identical to a ZL press. Without consuming this report before switching to normal polling, ZL would appear permanently held from the moment a ROM is loaded.

The drain sequence that follows `SDL_hid_open` handles this:

```cpp
u8 drain[64];
SDL_hid_read_timeout(hidDevice, drain, sizeof(drain), 50);
SDL_hid_set_nonblocking(hidDevice, 1);
while (SDL_hid_read(hidDevice, drain, sizeof(drain)) > 0) {}
```

`SDL_hid_read_timeout` with a 50ms timeout consumes the wakeup report. The wakeup report arrives within a few milliseconds in practice; 50ms provides sufficient margin. The device is then switched to non-blocking mode and any remaining buffered reports are flushed before normal polling begins.

### Non-blocking mode and per-frame polling

`pollHidReport()` is called once per emulation frame inside `inputProcess()` (which already holds `joyMutex`) and, since the in-game settings overlay added controller polling, also from the UI thread via `SettingsView::pollJoystick()`. Because of that second caller it now acquires `joyMutex` itself — the SDL mutex is recursive, so the `inputProcess()` path is unaffected (see *Thread Safety* below). In non-blocking mode, `SDL_hid_read()` returns 0 immediately when no report is pending — the common case during normal gameplay. This adds a negligible cost to each frame. Only report ID `0x3F` (the simple HID report format for this controller) is stored; other report IDs are discarded.

```cpp
void EmuInstance::pollHidReport()
{
    // hidDevice is opened/closed by openJoystick()/closeJoystick() (under joyMutex) and read here.
    // This runs on both EmuThread (inputProcess, already holding joyMutex — the SDL mutex is
    // recursive) and the UI thread (SettingsView::pollJoystick). Without this lock the device could
    // be closed on one thread while SDL's HID run loop reads it on another, freeing memory mid-read
    // and faulting on a thread where NDS::Current is null.
    SDL_LockMutex(joyMutex.get());
    if (hidDevice)
    {
        u8 buf[64];
        int n;
        while ((n = SDL_hid_read(hidDevice, buf, sizeof(buf))) != 0)
        {
            if (n < 0) { SDL_hid_close(hidDevice); hidDevice = nullptr; memset(hidReport, 0, sizeof(hidReport)); break; }
            if (n >= 3 && buf[0] == 0x3F) memcpy(hidReport, buf, n);
        }
    }
    SDL_UnlockMutex(joyMutex.get());
}
```

If `SDL_hid_read` returns an error, the device handle is closed, the report buffer is cleared, and ZL/ZR inputs are treated as released. This prevents stale button state from persisting if the HID device disconnects unexpectedly.

### Report format and byte offsets

The NSO SNES controller's simple HID report uses the following layout, confirmed empirically by capturing raw reports while pressing each button in isolation:

- Byte 0: report ID `0x3F`
- Byte 1 bit 6 (`0x40`): ZL pressed
- Byte 2 bit 7 (`0x80`): ZR pressed

The `hidReport[0] == 0x3F` check in the trigger detection path validates that the buffer contains a recognised report before reading trigger bits.

---

## Trigger Detection in `joystickButtonDown()`

The ZL/ZR fallback is wired into the existing axis type 2 (trigger) branch of `joystickButtonDown()`, which handles all trigger input for all controllers. The GameController API is always tried first:

```cpp
Sint16 trigval = controller
    ? SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)axisnum)
    : axisval;
if (trigval == 0 && hidReport[0] == 0x3F)
{
    bool pressed = (axisnum == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
        ? (hidReport[1] & 0x40) != 0
        : (hidReport[2] & 0x80) != 0;
    if (pressed) trigval = 32767;
}
if (trigval > 16384) return 1;
```

The HID fallback path is reached only when `trigval == 0` and the report header is valid. For any controller whose GC descriptor maps triggers correctly, `trigval` will be non-zero when a trigger is pressed, and the HID block is never entered. When the NSO SNES controller is active via CoreHID, `trigval` is always 0 for triggers (no GC mapping), and ZL/ZR are detected from `hidReport`. The synthesised value of `32767` — SDL's maximum axis value — satisfies the existing `> 16384` threshold check, which is consistent with the analog stick dead-zone threshold used throughout the same function.

The same detection logic is implemented in `JoyMapButton::checkJoystick()` in `MapButton.h`, which handles trigger detection during the binding UI. There, `pollAndGetHidReport()` on `InputConfigDialog` polls the HID device and returns the latest report:

```cpp
const melonDS::u8* hr = parentDialog->pollAndGetHidReport();
if (hr && hr[0] == 0x3F)
{
    if (hr[1] & 0x40) { /* bind ZL */ click(); return; }
    if (hr[2] & 0x80) { /* bind ZR */ click(); return; }
}
```

---

## Thread Safety: The Freeze Bug

### What was happening

After the HID drain was added to `openJoystick()`, opening the Input Config dialog caused the emulation to freeze permanently. The game screen halted mid-frame while the UI remained responsive. The freeze required a force-quit to recover.

The cause was `openJoystick()` being called from the **main thread** via `setJoystick()` and `setJoystickByUniqueId()`. Both functions acquire `joyMutex` for their duration. `openJoystick()` now contains `SDL_hid_read_timeout(50ms)` — a blocking call. On macOS, the SDL2 dylib exposes both `SDL_hid_read_timeout_REAL` and `SDL_hid_read_timeout_DEFAULT` symbols, indicating that the default path may not honour the timeout when called from certain thread contexts. The main thread's stack was blocked inside the drain, holding `joyMutex`. EmuThread's `inputProcess()` runs every ~16ms and acquires `joyMutex` on every frame — it could not proceed until the main thread released the lock. The dialog open sequence triggered two separate drain calls (one from the joystick combobox signal, one from `inputLoadConfig()`), making the block at least 100ms minimum. In practice the timeout was not respected and the block was indefinite.

### Why deadlock was ruled out before finding the real cause

`inputLoadConfig()` acquires `joyMutex`, then calls `setJoystickByUniqueId()`, which also acquires `joyMutex`. This looks like a potential deadlock: the same thread trying to lock a mutex it already holds. The SDL2 Homebrew install resolves this:

```
# /opt/homebrew/opt/sdl2/include/SDL2/SDL_config.h
#define SDL_THREAD_PTHREAD_RECURSIVE_MUTEX 1
```

On macOS, SDL mutexes are backed by `PTHREAD_MUTEX_RECURSIVE`. Re-entering the lock from the same thread increments an internal counter rather than blocking. The freeze is not a deadlock.

### The fix

`setJoystick()` and `setJoystickByUniqueId()` now call `closeJoystick()` instead of `openJoystick()`. `closeJoystick()` contains no I/O — it releases SDL handles and clears state immediately. The re-opening is deferred to EmuThread, which already has the correct re-open logic in `inputProcess()`:

```cpp
if (!joystick && (SDL_NumJoysticks() > 0))
    openJoystick();
```

The 50ms drain happens in EmuThread context, where it delays one emulation frame rather than blocking a mutex the emulation thread needs. This is the correct architectural boundary: blocking HID I/O belongs in the emulation thread, not the UI thread.

### A second hazard: UI-thread HID polling

The freeze fix above kept blocking HID *open* I/O off the main thread. A later change — the in-game settings overlay (`SettingsView`) — introduced a second way for a non-EmuThread to touch `hidDevice`: while the overlay is open it pauses emulation and polls the controller on a UI-thread timer, including a `pollHidReport()` call for ZL/ZR rebinding. That opened a use-after-free window: the overlay's reopen/rebind paths can `SDL_hid_close()` the device on the UI thread while SDL's HID run loop is mid-read on it, freeing memory under the read and faulting on a thread where `NDS::Current` is null (so it isn't even a JIT fault the SIGSEGV handler can interpret).

The fix is to serialise all `hidDevice` access. `pollHidReport()` now takes `joyMutex` for the duration of the read, matching `openJoystick()`/`closeJoystick()`, which already mutate the handle under the same lock; the recursive mutex means the existing `inputProcess()` caller (which already holds it) is unaffected. As a defence-in-depth net the ARMJIT SIGSEGV handler still advances the PC past a faulting instruction on any non-EmuThread, but it now logs that event (rate-limited) instead of swallowing it silently, so a genuinely unrelated worker-thread fault still surfaces.

---

## Device Identity and Config Persistence

### The problem with index-based config keys

The original code used `joystickID` — SDL's 0-based enumeration index — as the TOML section key for joystick bindings (e.g. `Joystick.1`). SDL's enumeration order is not stable: it changes when devices connect or disconnect, and with two devices for one physical controller, the index that the user configured their bindings under might not match the index SDL assigns on the next launch. This made saved bindings unreliable for the NSO SNES controller specifically.

### VID:PID as a stable identifier

`getJoystickUniqueIdById()` computes a stable identifier by combining the device's USB vendor ID and product ID:

```cpp
int EmuInstance::getJoystickUniqueIdById(int id) {
    u16 JoystickVendorID = SDL_JoystickGetDeviceVendor(id);
    u16 JoystickDeviceID = SDL_JoystickGetDeviceProduct(id);
    if (JoystickVendorID == 0 || JoystickDeviceID == 0) return -1;
    return (int)((JoystickVendorID << 16) | JoystickDeviceID);
}
```

`JoystickUniqueID` is now written to the TOML config and used as the section key for bindings (e.g. `Joystick.374964247`). This value is the same every time the same physical device is connected, regardless of enumeration order. `inputLoadConfig()` uses the saved unique ID as the config section key when loading bindings, falling back to the currently active joystick's unique ID only when no saved value is present.

**Known limitation**: Two controllers of identical model (same VID and PID) will share one config section, since the identifier does not incorporate serial number or path. SDL2 does not surface serial numbers reliably for all HID devices. For the target use case — a single NSO SNES controller — this is an acceptable trade-off.

---

## Input Config Dialog: Two Latent Bugs

Switching from index-based to VID:PID-based config section keys meant that joystick bindings now reliably loaded from and saved to the correct section. This exposed two pre-existing bugs in `on_cbxJoystick_currentIndexChanged()` that had been invisible because the wrong section was being read regardless.

### Bug 1: `SetInt` instead of `GetInt` in the loading section

`on_cbxJoystick_currentIndexChanged()` has two halves: it saves the current controller's in-memory mappings to TOML, then loads the newly selected controller's TOML mappings into the in-memory arrays. The save half correctly uses `SetInt` throughout. The load half correctly uses `GetInt` for keypad, hotkey, and general hotkey mappings — but for plugin and touchscreen mappings it used `SetInt`, writing the old controller's values into the new controller's TOML section instead of reading from it. This is a copy-paste error from the save half. The fix is straightforward:

```cpp
// Before (incorrect — writes old values into new section):
joycfg.SetInt(plugin->customKeyMappingNames[i], pluginJoyMap[i]);

// After (correct — reads new section into working arrays):
pluginJoyMap[i] = joycfg.GetInt(plugin->customKeyMappingNames[i]);
```

### Bug 2: Spurious Qt signal during constructor

Qt fires `currentIndexChanged` when `setCurrentIndex()` is called programmatically, including during widget setup. The original guard at the top of the handler:

```cpp
if (ui->cbxJoystick->count() < 2) return;
```

was intended to suppress this spurious call during construction. With the NSO SNES controller present, `SDL_NumJoysticks()` returns 2, so the combobox already has two items when `setCurrentIndex()` is called — `count >= 2`, and the guard does not fire.

At that point in construction, `setupAddonsPage()` has not yet run, so `pluginJoyMap[]` — declared as a plain C++ array member with no in-class initialiser — contains arbitrary stack memory. The save half of the handler was writing this uninitialised data to TOML, corrupting the plugin binding section before the dialog had even displayed.

The fix is to wrap the combobox population with `blockSignals(true/false)`:

```cpp
ui->cbxJoystick->blockSignals(true);
// addItem / setCurrentIndex calls...
ui->cbxJoystick->blockSignals(false);
```

`blockSignals` is Qt's standard mechanism for suppressing signals during programmatic widget setup. After the block is lifted, user-triggered changes fire the handler normally.

---

## Plugin System: Preventing Double-Application of Joystick Defaults

Both KH plugins (`PluginKingdomHeartsDays`, `PluginKingdomHeartsReCoded`) apply a set of default joystick bindings on first run — mapping ZL, ZR, and camera controls to sensible defaults for the NSO SNES controller. Without a guard, reloading a ROM triggers this code path again, overwriting any custom bindings the user has configured.

A `JoystickDefaultsApplied` boolean flag was added to the `Plugin` base class and persisted to the plugin's TOML config section:

```cpp
// Plugin.h
bool JoystickDefaultsApplied = false;

// Plugin.cpp — loaded from config on init
JoystickDefaultsApplied = getBoolConfig(root + ".JoystickDefaultsApplied");
```

Each plugin sets the flag and saves it after the first application:

```cpp
if (!JoystickDefaultsApplied)
    KingdomHeartsHDCollection::applyJoystickMappings(setIntConfig, false);

setBoolConfig(tomlUniqueIdentifier() + ".JoystickDefaultsApplied", true);
```

This follows the existing pattern for first-run configuration flags used throughout the plugin config system.

---

## What Is Unchanged

The following are explicitly unchanged to confirm backward compatibility:

**Button, axis, and hat encoding format**: The `int` packing used throughout `joyMapping[]`, `hkJoyMapping[]`, and related arrays is identical to upstream melonDS. No stored bindings require migration or conversion.

**TOML key names**: All config keys for keyboard, hotkey, addon, and general mappings are unchanged. Existing `.toml` files load correctly — only the joystick section header changes from an index to a VID:PID hash.

**Joystick lifecycle for other controllers**: The HID channel is only opened when `SDL_GameControllerGetBindForAxis` returns `BINDTYPE_NONE` for the left trigger. Any controller whose GameController descriptor includes trigger mappings — which is the majority of modern gamepads — never enters the HID path. `openJoystick()` and `closeJoystick()` behave identically to their pre-change state for those devices.

**Plugin interface contracts**: `customKeyMappingNames`, `customKeyMappingLabels`, and `applyAddonKeysToInputMaskOrTouchControls` are unchanged. Plugins implementing these interfaces require no modification.

**EmuThread frame timing**: `pollHidReport()` adds one non-blocking `SDL_hid_read()` call per frame. In non-blocking mode, this returns immediately when no report is pending. The overhead is in the microsecond range and has no practical effect on emulation timing.
