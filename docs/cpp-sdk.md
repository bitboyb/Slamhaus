# C++ SDK reference

The public SDK headers are Doxygen-compatible. With Doxygen installed, build
the generated API reference with:

```sh
cmake --build build --target doxygen
```

The HTML reference is written to `build/docs-build/doxygen/html/` and is not
committed.

Page-local `cpp wasm` fences automatically receive `<slam/slam.hpp>` and
`using namespace Slam`. Sources under `code/` are ordinary C++ and include the
header explicitly. The installed header is available to IDE indexers.

## Bindings

Functions referenced by Slam UI are exported automatically when they are
defined in the page's executable fence. They must be namespace-scope,
non-overloaded free functions using `void`, `bool`, fixed-width integer aliases,
floating-point values, or `FString`.

Shared modules use `SLAM_BIND(Function)`. Argument conversion and result
formatting happen in C++. Invalid counts, values, or names return typed bridge
errors rather than throwing through a browser event.

## Applications

Set `runtime.application` and `runtime.canvas` in page front matter. Slamhaus
generates `SLAM_IMPLEMENT_APPLICATION` for page-local applications. Shared
modules register it explicitly.

Derive one `FApplication` per module. Override `BeginPlay`, `Tick`, `EndPlay`,
`OnSuspend`, `OnResume`, and `OnResize` as needed.

## Platform helpers

- `FInput`: keys, mouse buttons, pointer position/delta, wheel, touch, gamepads.
- `FCanvas`, `FDisplay`: framebuffer, scale, orientation, fullscreen, pointer lock.
- `FDocument`: text/value, classes, visibility, focus, and safe selectors.
- `FAudio`: play, pause, resume, stop, looping, and volume handles.
- `FHttpClient`, `FAssetManager`: asynchronous text requests with cancellation.
- `FStorage`: namespaced local or session string storage.
- `FNavigation`, `FClipboard`, `FHaptics`: browser interaction helpers.
- `FSensors`, `FAccessibility`, `FNetwork`, `FPwa`, `FLog`: mobile and app state.

Permission- and gesture-gated calls return `false` or `TResult` errors when the
browser rejects a request. Start audio, clipboard, haptics, fullscreen, pointer
lock, and installation from a user event.

Slamhaus intentionally provides no `UObject`, reflection, actors, garbage
collector, scene graph, collision engine, or renderer. Unreal-style names are
an ergonomic convention, not an Unreal Engine compatibility layer.
