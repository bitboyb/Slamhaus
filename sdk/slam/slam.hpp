#pragma once

/** @file
 *  Public C++17 SDK for Slamhaus WebAssembly pages.
 *
 *  The SDK exposes browser capabilities through small, renderer-agnostic
 *  value types and platform helpers. Applications can include this header in
 *  native tests; browser-only operations become safe no-ops outside Emscripten.
 */

#include <array>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#define EM_JS(ReturnType, Name, Parameters, Code) inline ReturnType Name Parameters
#endif

namespace Slam {

/** Fixed-width integer aliases used by the public SDK ABI. */
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using FString = std::string;
using FName = std::string;

/** C++ container alias used for SDK collections. */
template <typename T> using TArray = std::vector<T>;
/** Optional ownership alias for SDK-created objects. */
template <typename T> using TOptional = std::unique_ptr<T>;
/** Callback alias used by asynchronous browser APIs. */
template <typename Signature> using TDelegate = std::function<Signature>;

/** Error categories returned by browser and platform helpers. */
enum class EPlatformError : uint8 {
  None,
  Unsupported,
  PermissionDenied,
  RequiresUserGesture,
  InvalidArgument,
  NotFound,
  Network,
  Cancelled,
  Internal
};

/** A value-or-error result that avoids exceptions across the WASM boundary. */
template <typename T> class TResult {
public:
  /** Construct a successful result. */
  static auto Ok(T Value) -> TResult { return TResult(std::move(Value)); }
  /** Construct a failed result with a stable platform error code. */
  static auto Error(EPlatformError Code, FString Message) -> TResult {
    return TResult(Code, std::move(Message));
  }
  explicit operator bool() const { return bOk; }
  auto IsOk() const -> bool { return bOk; }
  auto GetValue() const -> const T & { return Value; }
  auto GetError() const -> EPlatformError { return ErrorCode; }
  auto GetMessage() const -> const FString & { return Message; }

private:
  explicit TResult(T InValue) : bOk(true), Value(std::move(InValue)) {}
  TResult(EPlatformError Code, FString InMessage)
      : ErrorCode(Code), Message(std::move(InMessage)) {}
  bool bOk = false;
  T Value{};
  EPlatformError ErrorCode = EPlatformError::None;
  FString Message;
};

/** Void specialization of TResult for command-style operations. */
template <> class TResult<void> {
public:
  /** Construct a successful void result. */
  static auto Ok() -> TResult { return TResult(true); }
  /** Construct a failed void result. */
  static auto Error(EPlatformError Code, FString Message) -> TResult {
    return TResult(Code, std::move(Message));
  }
  explicit operator bool() const { return bOk; }
  auto IsOk() const -> bool { return bOk; }
  auto GetError() const -> EPlatformError { return ErrorCode; }
  auto GetMessage() const -> const FString & { return Message; }

private:
  explicit TResult(bool InOk) : bOk(InOk) {}
  TResult(EPlatformError Code, FString InMessage)
      : ErrorCode(Code), Message(std::move(InMessage)) {}
  bool bOk = false;
  EPlatformError ErrorCode = EPlatformError::None;
  FString Message;
};

/** Two-dimensional floating-point coordinate. */
struct FVector2D {
  float X = 0.0F;
  float Y = 0.0F;
};
/** Two-dimensional integer coordinate. */
struct FIntPoint {
  int32 X = 0;
  int32 Y = 0;
};
/** RGBA color in normalized 8-bit channels. */
struct FColor {
  uint8 R = 0;
  uint8 G = 0;
  uint8 B = 0;
  uint8 A = 255;
};
/** CSS and framebuffer dimensions of the active canvas. */
struct FCanvasSize {
  int32 Width = 0;
  int32 Height = 0;
  int32 FramebufferWidth = 0;
  int32 FramebufferHeight = 0;
  float DeviceScale = 1.0F;
};
/** Four-sided inset or padding measurement. */
struct FInsets {
  float Top = 0;
  float Right = 0;
  float Bottom = 0;
  float Left = 0;
};
/** One active touch contact. */
struct FTouchPoint {
  int32 Id = -1;
  FVector2D Position;
  float Pressure = 0.0F;
};
/** Snapshot of a connected gamepad's axes and buttons. */
struct FGamepadState {
  bool bConnected = false;
  TArray<float> Axes;
  TArray<float> Buttons;
};
/** Result returned by an asynchronous HTTP request. */
struct FHttpResponse {
  int32 Status = 0;
  FString Body;
};
/** Opaque handle used to cancel an asynchronous request. */
struct FRequestHandle {
  uint32 Id = 0;
  explicit operator bool() const { return Id != 0; }
};

/** Keyboard codes exposed by the browser input bridge. */
enum class EKey : int32 {
  Unknown = 0,
  Backspace = 8,
  Tab = 9,
  Enter = 13,
  Shift = 16,
  Control = 17,
  Alt = 18,
  Escape = 27,
  SpaceBar = 32,
  Left = 37,
  Up = 38,
  Right = 39,
  Down = 40,
  A = 65,
  D = 68,
  S = 83,
  W = 87
};
/** Mouse button identifiers. */
enum class EMouseButton : int32 {
  Left = 0,
  Middle = 1,
  Right = 2,
  Back = 3,
  Forward = 4
};
/** Current viewport orientation. */
enum class EOrientation : uint8 { Unknown, Portrait, Landscape };

#ifdef __EMSCRIPTEN__
// clang-format off
EM_JS(int, SlamInputState, (int Kind, int Code), {
  const s=globalThis.__slamhaus?.input;if(!s)return 0;
  const set=Kind===0?s.keys:Kind===1?s.pressed:Kind===2?s.released:s.buttons;
  return set?.has(Code)?1:0;
});
EM_JS(double, SlamInputNumber, (int Kind), {
  const i=globalThis.__slamhaus?.input||{};return [i.x||0,i.y||0,i.dx||0,i.dy||0,i.wheelX||0,i.wheelY||0][Kind]||0;
});
EM_JS(double, SlamDisplayNumber, (int Kind), {
  const c=globalThis.__slamhaus?.canvas;if(Kind===0)return innerWidth;if(Kind===1)return innerHeight;if(Kind===2)return devicePixelRatio||1;if(Kind===3)return c?.width||0;return c?.height||0;
});
EM_JS(int, SlamTouchCount, (), {return globalThis.__slamhaus?.input?.touches?.size||0;});
EM_JS(double, SlamTouchNumber, (int Index, int Field), {
  const t=[...(globalThis.__slamhaus?.input?.touches?.entries()||[])][Index];if(!t)return 0;return Field===0?t[0]:Field===1?t[1].x:Field===2?t[1].y:t[1].pressure||0;
});
EM_JS(int, SlamGamepadCount, (), {return navigator.getGamepads?[...navigator.getGamepads()].filter(Boolean).length:0;});
EM_JS(double, SlamGamepadNumber, (int Index, int Field, int Item), {
  const g=[...navigator.getGamepads()].filter(Boolean)[Index];if(!g)return 0;if(Field===0)return 1;if(Field===1)return g.axes[Item]||0;return g.buttons[Item]?.value||0;
});
EM_JS(int, SlamDocumentCommand, (int Command, const char* Selector, const char* Value), {
  const e=document.querySelector(UTF8ToString(Selector));if(!e)return 0;const v=Value?UTF8ToString(Value):String();
  if(Command===0)e.textContent=v;else if(Command===1)e.value=v;else if(Command===2)e.classList.add(v);else if(Command===3)e.classList.remove(v);else if(Command===4)e.hidden=!!+v;else if(Command===5)e.focus();return 1;
});
EM_JS(char*, SlamDocumentRead, (const char* Selector, int Property), {
  const e=document.querySelector(UTF8ToString(Selector));const s=e?(Property===0?(e.value==null?String():e.value):(e.textContent==null?String():e.textContent)):String();const n=lengthBytesUTF8(s)+1,p=_malloc(n);stringToUTF8(s,p,n);return p;
});
EM_JS(int, SlamStorageWrite, (int Session, const char* Key, const char* Value), {try{(Session?sessionStorage:localStorage).setItem(UTF8ToString(Key),UTF8ToString(Value));return 1}catch{return 0}});
EM_JS(char*, SlamStorageRead, (int Session, const char* Key), {let s=String();try{const v=(Session?sessionStorage:localStorage).getItem(UTF8ToString(Key));s=v==null?String():v}catch{}const n=lengthBytesUTF8(s)+1,p=_malloc(n);stringToUTF8(s,p,n);return p;});
EM_JS(int, SlamStorageRemove, (int Session, const char* Key), {try{(Session?sessionStorage:localStorage).removeItem(UTF8ToString(Key));return 1}catch{return 0}});
EM_JS(int, SlamBrowserCommand, (int Command, const char* Value), {
  const v=Value?UTF8ToString(Value):String();if(Command===0&&navigator.vibrate){navigator.vibrate(+v||20);return 1}if(Command===1&&navigator.clipboard){navigator.clipboard.writeText(v);return 1}if(Command===2){location.assign(v);return 1}if(Command===3){window.open(v,'_blank','noopener');return 1}if(Command===4&&globalThis.__slamhaus?.canvas?.requestFullscreen){globalThis.__slamhaus.canvas.requestFullscreen();return 1}if(Command===5&&globalThis.__slamhaus?.canvas?.requestPointerLock){globalThis.__slamhaus.canvas.requestPointerLock();return 1}if(Command===6&&globalThis.__slamhaus?.install){globalThis.__slamhaus.install();return 1}return 0;
});
EM_JS(int, SlamBrowserCapability, (int Capability), {if(Capability===0)return navigator.onLine?1:0;if(Capability===1)return navigator.clipboard?1:0;if(Capability===2)return navigator.vibrate?1:0;if(Capability===3)return globalThis.__slamhaus?.canInstall?1:0;if(Capability===4)return matchMedia('(prefers-reduced-motion: reduce)').matches?1:0;return 0;});
EM_JS(int, SlamAudioPlay, (const char* Source, int Loop, double Volume), {
  const s=globalThis.__slamhaus||(globalThis.__slamhaus={});if(!s.audio)s.audio=new Map();if(!s.nextAudio)s.nextAudio=1;const a=new Audio(UTF8ToString(Source));a.loop=!!Loop;a.volume=Math.max(0,Math.min(1,Volume));const id=s.nextAudio++;s.audio.set(id,a);a.addEventListener('ended',()=>{if(!a.loop)s.audio.delete(id)});a.play().catch(error=>console.error('Slamhaus audio',error));return id;
});
EM_JS(int, SlamAudioCommand, (int Command, int Handle, double Value), {const a=globalThis.__slamhaus?.audio?.get(Handle);if(!a)return 0;if(Command===0)a.pause();else if(Command===1){a.pause();a.currentTime=0;globalThis.__slamhaus.audio.delete(Handle)}else if(Command===2)a.volume=Math.max(0,Math.min(1,Value));else if(Command===3)a.play();return 1;});
EM_JS(void, SlamHttpGet, (int Handle, const char* Url), {
  fetch(UTF8ToString(Url)).then(async response=>[response.status,await response.text(),String()]).catch(error=>[0,String(),String(error)]).then(([status,body,error])=>{const bn=lengthBytesUTF8(body)+1,bp=_malloc(bn);stringToUTF8(body,bp,bn);const en=lengthBytesUTF8(error)+1,ep=_malloc(en);stringToUTF8(error,ep,en);_slam_http_complete(Handle,status,bp,ep);_free(bp);_free(ep)});
});
EM_JS(double, SlamSensorNumber, (int Field), {const s=globalThis.__slamhaus?.sensors||{};return [s.alpha||0,s.beta||0,s.gamma||0,s.ax||0,s.ay||0,s.az||0][Field]||0;});
EM_JS(int, SlamAnnounce, (const char* Message), {let e=document.getElementById('slamhaus-announcer');if(!e){e=document.createElement('div');e.id='slamhaus-announcer';e.setAttribute('role','status');e.setAttribute('aria-live','polite');Object.assign(e.style,{position:'fixed',width:'1px',height:'1px',overflow:'hidden',clip:'rect(0 0 0 0)'});document.body.append(e)}e.textContent=UTF8ToString(Message);return 1;});
EM_JS(void, SlamLogMessage, (int Level, const char* Message), {(Level===2?console.error:Level===1?console.warn:console.log)(UTF8ToString(Message));});
// clang-format on
#else
inline auto SlamInputState(int, int) -> int { return 0; }
inline auto SlamInputNumber(int) -> double { return 0; }
inline auto SlamDisplayNumber(int) -> double { return 0; }
inline auto SlamTouchCount() -> int { return 0; }
inline auto SlamTouchNumber(int, int) -> double { return 0; }
inline auto SlamGamepadCount() -> int { return 0; }
inline auto SlamGamepadNumber(int, int, int) -> double { return 0; }
inline auto SlamDocumentCommand(int, const char *, const char *) -> int { return 0; }
inline auto SlamDocumentRead(const char *, int) -> char * { return nullptr; }
inline auto SlamStorageWrite(int, const char *, const char *) -> int { return 0; }
inline auto SlamStorageRead(int, const char *) -> char * { return nullptr; }
inline auto SlamStorageRemove(int, const char *) -> int { return 0; }
inline auto SlamBrowserCommand(int, const char *) -> int { return 0; }
inline auto SlamBrowserCapability(int) -> int { return 0; }
inline auto SlamAudioPlay(const char *, int, double) -> int { return 0; }
inline auto SlamAudioCommand(int, int, double) -> int { return 0; }
inline void SlamHttpGet(int, const char *) {}
inline auto SlamSensorNumber(int) -> double { return 0; }
inline auto SlamAnnounce(const char *) -> int { return 0; }
inline void SlamLogMessage(int, const char *) {}
#endif

/** Read keyboard, pointer, touch, and gamepad state. */
class FInput {
public:
  /** Return whether a key is currently held. */
  static auto IsKeyDown(EKey Key) -> bool {
    return SlamInputState(0, static_cast<int>(Key)) != 0;
  }
  static auto WasKeyPressed(EKey Key) -> bool {
    return SlamInputState(1, static_cast<int>(Key)) != 0;
  }
  static auto WasKeyReleased(EKey Key) -> bool {
    return SlamInputState(2, static_cast<int>(Key)) != 0;
  }
  static auto IsMouseDown(EMouseButton Button) -> bool {
    return SlamInputState(3, static_cast<int>(Button)) != 0;
  }
  static auto GetPointerPosition() -> FVector2D {
    return {static_cast<float>(SlamInputNumber(0)),
            static_cast<float>(SlamInputNumber(1))};
  }
  static auto GetPointerDelta() -> FVector2D {
    return {static_cast<float>(SlamInputNumber(2)),
            static_cast<float>(SlamInputNumber(3))};
  }
  static auto GetWheelDelta() -> FVector2D {
    return {static_cast<float>(SlamInputNumber(4)),
            static_cast<float>(SlamInputNumber(5))};
  }
  static auto GetTouches() -> TArray<FTouchPoint> {
    TArray<FTouchPoint> Values;
    const auto Count = SlamTouchCount();
    Values.reserve(static_cast<std::size_t>(Count));
    for (int Index = 0; Index < Count; ++Index)
      Values.push_back({static_cast<int32>(SlamTouchNumber(Index, 0)),
                        {static_cast<float>(SlamTouchNumber(Index, 1)),
                         static_cast<float>(SlamTouchNumber(Index, 2))},
                        static_cast<float>(SlamTouchNumber(Index, 3))});
    return Values;
  }
  /** Return the current state of a gamepad, or an empty state if absent. */
  static auto GetGamepad(int32 Index = 0) -> FGamepadState {
    FGamepadState State;
    State.bConnected = SlamGamepadNumber(Index, 0, 0) != 0;
    if (!State.bConnected)
      return State;
    for (int Item = 0; Item < 8; ++Item)
      State.Axes.push_back(static_cast<float>(SlamGamepadNumber(Index, 1, Item)));
    for (int Item = 0; Item < 20; ++Item)
      State.Buttons.push_back(static_cast<float>(SlamGamepadNumber(Index, 2, Item)));
    return State;
  }
};

/** Query the page canvas and request browser canvas capabilities. */
/** Browser canvas metrics and permission-gated canvas controls. */
class FCanvas {
public:
  /** Query the active canvas size and framebuffer scale. */
  static auto GetSize() -> FCanvasSize {
    return {static_cast<int32>(SlamDisplayNumber(0)),
            static_cast<int32>(SlamDisplayNumber(1)),
            static_cast<int32>(SlamDisplayNumber(3)),
            static_cast<int32>(SlamDisplayNumber(4)),
            static_cast<float>(SlamDisplayNumber(2))};
  }
  /** Request browser fullscreen for the active canvas. */
  static auto RequestFullscreen() -> TResult<void> { return Command(4, ""); }
  /** Request pointer-lock input for the active canvas. */
  static auto RequestPointerLock() -> TResult<void> { return Command(5, ""); }

private:
  static auto Command(int Code, const char *Value) -> TResult<void> {
    return SlamBrowserCommand(Code, Value)
               ? TResult<void>::Ok()
               : TResult<void>::Error(EPlatformError::Unsupported,
                                      "Browser capability is unavailable.");
  }
};

/** Query viewport size, device scale, and orientation. */
/** Browser viewport and display characteristics. */
class FDisplay {
public:
  /** Return the browser viewport in CSS pixels. */
  static auto GetViewportSize() -> FIntPoint {
    return {static_cast<int32>(SlamDisplayNumber(0)),
            static_cast<int32>(SlamDisplayNumber(1))};
  }
  /** Return the browser device pixel ratio. */
  static auto GetDeviceScale() -> float {
    return static_cast<float>(SlamDisplayNumber(2));
  }
  /** Return the current screen orientation. */
  static auto GetOrientation() -> EOrientation {
    const auto Size = GetViewportSize();
    if (Size.X == 0 || Size.Y == 0)
      return EOrientation::Unknown;
    return Size.X >= Size.Y ? EOrientation::Landscape : EOrientation::Portrait;
  }
};

/** Perform safe text, value, class, visibility, and focus operations. */
/** Safe DOM operations exposed to generated or application code. */
class FDocument {
public:
  /** Set text content for the first element matching Selector. */
  static auto SetText(std::string_view Selector, std::string_view Value) -> bool {
    return Command(0, Selector, Value);
  }
  /** Set the value property of the first matching form control. */
  static auto SetValue(std::string_view Selector, std::string_view Value) -> bool {
    return Command(1, Selector, Value);
  }
  /** Add a CSS class to the first matching element. */
  static auto AddClass(std::string_view Selector, std::string_view Value) -> bool {
    return Command(2, Selector, Value);
  }
  /** Remove a CSS class from the first matching element. */
  static auto RemoveClass(std::string_view Selector, std::string_view Value) -> bool {
    return Command(3, Selector, Value);
  }
  /** Set or clear the hidden state of the first matching element. */
  static auto SetHidden(std::string_view Selector, bool Hidden) -> bool {
    return Command(4, Selector, Hidden ? "1" : "0");
  }
  /** Move browser focus to the first matching element. */
  static auto Focus(std::string_view Selector) -> bool {
    return Command(5, Selector, "");
  }
  /** Read a form control value. */
  static auto GetValue(std::string_view Selector) -> FString {
    return Read(Selector, 0);
  }
  /** Read text content from the first matching element. */
  static auto GetText(std::string_view Selector) -> FString {
    return Read(Selector, 1);
  }

private:
  static auto Command(int Code, std::string_view Selector, std::string_view Value)
      -> bool {
    const FString S(Selector), V(Value);
    return SlamDocumentCommand(Code, S.c_str(), V.c_str()) != 0;
  }
  static auto Read(std::string_view Selector, int Property) -> FString {
    const FString S(Selector);
    char *Raw = SlamDocumentRead(S.c_str(), Property);
    if (!Raw)
      return {};
    FString Value(Raw);
    std::free(Raw);
    return Value;
  }
};

/** Namespaced local and session storage helpers. */
class FStorage {
public:
  /** Store a namespaced value in local or session storage. */
  static auto Set(std::string_view Key, std::string_view Value, bool Session = false)
      -> bool {
    const FString K(Key), V(Value);
    return SlamStorageWrite(Session, K.c_str(), V.c_str()) != 0;
  }
  /** Read a value, returning an empty string when it is absent. */
  static auto Get(std::string_view Key, bool Session = false) -> FString {
    const FString K(Key);
    char *Raw = SlamStorageRead(Session, K.c_str());
    if (!Raw)
      return {};
    FString Value(Raw);
    std::free(Raw);
    return Value;
  }
  /** Remove a stored value. */
  static auto Remove(std::string_view Key, bool Session = false) -> bool {
    const FString K(Key);
    return SlamStorageRemove(Session, K.c_str()) != 0;
  }
};

/** Clipboard write helper, subject to browser permissions and gestures. */
class FClipboard {
public:
  /** Copy text after a browser user gesture when required. */
  static auto WriteText(std::string_view Text) -> bool {
    const FString V(Text);
    return SlamBrowserCommand(1, V.c_str()) != 0;
  }
};
/** Haptic feedback helper for supported devices. */
class FHaptics {
public:
  /** Request a short device vibration. */
  static auto Pulse(int32 Milliseconds = 20) -> bool {
    const auto V = std::to_string(Milliseconds);
    return SlamBrowserCommand(0, V.c_str()) != 0;
  }
};
/** Current-page and external navigation helpers. */
class FNavigation {
public:
  /** Navigate the current page to Url. */
  static auto Navigate(std::string_view Url) -> bool {
    const FString V(Url);
    return SlamBrowserCommand(2, V.c_str()) != 0;
  }
  /** Open an external URL in a new browser context. */
  static auto OpenExternal(std::string_view Url) -> bool {
    const FString V(Url);
    return SlamBrowserCommand(3, V.c_str()) != 0;
  }
};
/** Browser-console logging helpers. */
class FLog {
public:
  /** Write an informational browser-console message. */
  static void Information(std::string_view Message) { Write(0, Message); }
  /** Write a warning to the browser console. */
  static void Warning(std::string_view Message) { Write(1, Message); }
  /** Write an error to the browser console. */
  static void Error(std::string_view Message) { Write(2, Message); }

private:
  static void Write(int Level, std::string_view Message) {
    const FString V(Message);
    SlamLogMessage(Level, V.c_str());
  }
};

/** Handle for an audio resource owned by the browser runtime. */
struct FAudioHandle {
  int32 Id = 0;
  explicit operator bool() const { return Id != 0; }
};
/** Browser audio playback helpers. */
class FAudio {
public:
  /** Start loading and playing an audio source. */
  static auto Play(std::string_view Source, bool Loop = false, float Volume = 1.0F)
      -> FAudioHandle {
    const FString V(Source);
    return {SlamAudioPlay(V.c_str(), Loop, Volume)};
  }
  static auto Pause(FAudioHandle Handle) -> bool {
    return SlamAudioCommand(0, Handle.Id, 0) != 0;
  }
  /** Stop playback and release an audio handle. */
  static auto Stop(FAudioHandle Handle) -> bool {
    return SlamAudioCommand(1, Handle.Id, 0) != 0;
  }
  /** Change playback volume, clamped by the browser runtime. */
  static auto SetVolume(FAudioHandle Handle, float Volume) -> bool {
    return SlamAudioCommand(2, Handle.Id, Volume) != 0;
  }
  /** Resume a paused audio handle. */
  static auto Resume(FAudioHandle Handle) -> bool {
    return SlamAudioCommand(3, Handle.Id, 0) != 0;
  }
};

/** Browser network capability helpers. */
class FNetwork {
public:
  /** Return the browser's current online state. */
  static auto IsOnline() -> bool { return SlamBrowserCapability(0) != 0; }
};
/** Accessibility announcements and user-preference helpers. */
class FAccessibility {
public:
  /** Announce a message through the runtime live region. */
  static auto Announce(std::string_view Message) -> bool {
    const FString V(Message);
    return SlamAnnounce(V.c_str()) != 0;
  }
  static auto PrefersReducedMotion() -> bool { return SlamBrowserCapability(4) != 0; }
};
/** Device orientation and acceleration readings. */
class FSensors {
public:
  /** Read device orientation sensor angles. */
  static auto GetOrientation() -> std::array<float, 3> {
    return {static_cast<float>(SlamSensorNumber(0)),
            static_cast<float>(SlamSensorNumber(1)),
            static_cast<float>(SlamSensorNumber(2))};
  }
  static auto GetAcceleration() -> std::array<float, 3> {
    return {static_cast<float>(SlamSensorNumber(3)),
            static_cast<float>(SlamSensorNumber(4)),
            static_cast<float>(SlamSensorNumber(5))};
  }
};
/** Progressive Web App installation helpers. */
class FPwa {
public:
  /** Return whether the site can currently be installed. */
  static auto CanInstall() -> bool { return SlamBrowserCapability(3) != 0; }
  /** Ask the browser to show the install prompt. */
  static auto PromptInstall() -> bool { return SlamBrowserCommand(6, "") != 0; }
};

/** Base lifecycle object for a page-local or shared WASM application. */
class FApplication {
public:
  virtual ~FApplication() = default;
  /** Called once when the runtime starts the application. */
  virtual void BeginPlay() {}
  /** Called once per animation tick with elapsed seconds. */
  virtual void Tick(float DeltaTime) { (void)DeltaTime; }
  /** Called when the runtime shuts the application down. */
  virtual void EndPlay() {}
  /** Called when the page is suspended or backgrounded. */
  virtual void OnSuspend() {}
  /** Called when the page resumes from suspension. */
  virtual void OnResume() {}
  /** Called after the browser viewport or canvas size changes. */
  virtual void OnResize(const FCanvasSize &Size) { (void)Size; }
};

namespace Private {

inline auto SplitArguments(std::string_view Payload) -> TResult<TArray<FString>> {
  TArray<FString> Values;
  std::size_t Cursor = 0;
  while (Cursor < Payload.size()) {
    const auto Colon = Payload.find(':', Cursor);
    if (Colon == std::string_view::npos)
      return TResult<TArray<FString>>::Error(EPlatformError::InvalidArgument,
                                             "Invalid argument payload.");
    std::size_t Length = 0;
    try {
      Length = static_cast<std::size_t>(
          std::stoull(FString(Payload.substr(Cursor, Colon - Cursor))));
    } catch (...) {
      return TResult<TArray<FString>>::Error(EPlatformError::InvalidArgument,
                                             "Invalid argument length.");
    }
    Cursor = Colon + 1;
    if (Length > Payload.size() - Cursor)
      return TResult<TArray<FString>>::Error(EPlatformError::InvalidArgument,
                                             "Truncated argument payload.");
    Values.emplace_back(Payload.substr(Cursor, Length));
    Cursor += Length;
  }
  return TResult<TArray<FString>>::Ok(std::move(Values));
}

template <typename T> auto Convert(const FString &Value) -> TResult<T> {
  try {
    if constexpr (std::is_same_v<T, FString>)
      return TResult<T>::Ok(Value);
    else if constexpr (std::is_same_v<T, bool>) {
      if (Value == "true" || Value == "1" || Value == "yes" || Value == "on")
        return TResult<T>::Ok(true);
      if (Value == "false" || Value == "0" || Value == "no" || Value == "off" ||
          Value.empty())
        return TResult<T>::Ok(false);
    } else if constexpr (std::is_integral_v<T>) {
      std::size_t Used = 0;
      const auto Parsed = std::stoll(Value, &Used);
      if (Used == Value.size() && Parsed >= std::numeric_limits<T>::min() &&
          Parsed <= std::numeric_limits<T>::max())
        return TResult<T>::Ok(static_cast<T>(Parsed));
    } else if constexpr (std::is_floating_point_v<T>) {
      std::size_t Used = 0;
      const auto Parsed = std::stod(Value, &Used);
      if (Used == Value.size())
        return TResult<T>::Ok(static_cast<T>(Parsed));
    }
  } catch (...) {
  }
  return TResult<T>::Error(EPlatformError::InvalidArgument,
                           "Could not convert argument value.");
}

template <typename T> auto Stringify(const T &Value) -> FString {
  if constexpr (std::is_same_v<T, FString>)
    return Value;
  else if constexpr (std::is_same_v<T, bool>)
    return Value ? "true" : "false";
  else {
    std::ostringstream Out;
    Out << Value;
    return Out.str();
  }
}

struct IBinding {
  virtual ~IBinding() = default;
  virtual auto Invoke(const TArray<FString> &) -> TResult<FString> = 0;
};
inline auto Bindings() -> std::unordered_map<FName, std::unique_ptr<IBinding>> & {
  static std::unordered_map<FName, std::unique_ptr<IBinding>> Value;
  return Value;
}

template <typename> struct TFunctionBinding;
template <typename Return, typename... Args>
struct TFunctionBinding<Return (*)(Args...)> final : IBinding {
  using FPointer = Return (*)(Args...);
  explicit TFunctionBinding(FPointer InFunction) : Function(InFunction) {}
  auto Invoke(const TArray<FString> &Values) -> TResult<FString> override {
    if (Values.size() != sizeof...(Args))
      return TResult<FString>::Error(EPlatformError::InvalidArgument,
                                     "Function argument count does not match.");
    return InvokeAt(Values, std::index_sequence_for<Args...>{});
  }
  template <std::size_t... I>
  auto InvokeAt(const TArray<FString> &Values, std::index_sequence<I...>)
      -> TResult<FString> {
    auto Converted = std::tuple<TResult<std::decay_t<Args>>...>(
        Convert<std::decay_t<Args>>(Values[I])...);
    bool Valid = (std::get<I>(Converted).IsOk() && ...);
    if (!Valid)
      return TResult<FString>::Error(EPlatformError::InvalidArgument,
                                     "One or more arguments could not be converted.");
    if constexpr (std::is_void_v<Return>) {
      Function(std::get<I>(Converted).GetValue()...);
      return TResult<FString>::Ok({});
    } else
      return TResult<FString>::Ok(
          Stringify(Function(std::get<I>(Converted).GetValue()...)));
  }
  FPointer Function;
};

template <typename Pointer> struct TBindingRegistration {
  TBindingRegistration(const char *Name, Pointer Function) {
    Bindings()[Name] = std::make_unique<TFunctionBinding<Pointer>>(Function);
  }
};

using FApplicationFactory = std::unique_ptr<FApplication> (*)();
inline auto ApplicationFactory() -> FApplicationFactory & {
  static FApplicationFactory Value = nullptr;
  return Value;
}
inline auto Application() -> std::unique_ptr<FApplication> & {
  static std::unique_ptr<FApplication> Value;
  return Value;
}
template <typename T> struct TApplicationRegistration {
  TApplicationRegistration() {
    ApplicationFactory() = []() -> std::unique_ptr<FApplication> {
      return std::make_unique<T>();
    };
  }
};
inline FString BridgeResult;
inline auto HttpCallbacks()
    -> std::unordered_map<uint32, TDelegate<void(TResult<FHttpResponse>)>> & {
  static std::unordered_map<uint32, TDelegate<void(TResult<FHttpResponse>)>> Value;
  return Value;
}
inline uint32 NextRequest = 1;

} // namespace Private

class FHttpClient {
public:
  /** Fetch a URL and deliver the response asynchronously. */
  static auto Get(std::string_view Url,
                  TDelegate<void(TResult<FHttpResponse>)> Callback) -> FRequestHandle {
    const auto Handle = Private::NextRequest++;
    Private::HttpCallbacks()[Handle] = std::move(Callback);
    const FString Value(Url);
    SlamHttpGet(static_cast<int>(Handle), Value.c_str());
    return {Handle};
  }
  /** Cancel a pending request; returns false if it already completed. */
  static auto Cancel(FRequestHandle Handle) -> bool {
    return Private::HttpCallbacks().erase(Handle.Id) != 0;
  }
};
class FAssetManager {
public:
  /** Fetch a text asset and return only its body through Callback. */
  static auto LoadText(std::string_view Url, TDelegate<void(TResult<FString>)> Callback)
      -> FRequestHandle {
    return FHttpClient::Get(Url, [Callback = std::move(Callback)](
                                     TResult<FHttpResponse> Response) mutable {
      if (Response)
        Callback(TResult<FString>::Ok(Response.GetValue().Body));
      else
        Callback(TResult<FString>::Error(Response.GetError(), Response.GetMessage()));
    });
  }
};

extern "C" EMSCRIPTEN_KEEPALIVE inline void
slam_http_complete(int Handle, int Status, const char *Body, const char *Error) {
  const auto Found = Private::HttpCallbacks().find(static_cast<uint32>(Handle));
  if (Found == Private::HttpCallbacks().end())
    return;
  auto Callback = std::move(Found->second);
  Private::HttpCallbacks().erase(Found);
  if (Error && *Error)
    Callback(TResult<FHttpResponse>::Error(EPlatformError::Network, Error));
  else
    Callback(TResult<FHttpResponse>::Ok({Status, Body ? Body : ""}));
}

extern "C" EMSCRIPTEN_KEEPALIVE inline auto slam_invoke(const char *Name,
                                                        const char *Payload) -> const
    char * {
  const auto Found = Private::Bindings().find(Name ? Name : "");
  if (Found == Private::Bindings().end()) {
    Private::BridgeResult = "1:Function is not registered.";
    return Private::BridgeResult.c_str();
  }
  const auto Values = Private::SplitArguments(Payload ? Payload : "");
  if (!Values) {
    Private::BridgeResult = "1:" + Values.GetMessage();
    return Private::BridgeResult.c_str();
  }
  const auto Result = Found->second->Invoke(Values.GetValue());
  Private::BridgeResult =
      Result ? "0:" + Result.GetValue() : "1:" + Result.GetMessage();
  return Private::BridgeResult.c_str();
}
extern "C" EMSCRIPTEN_KEEPALIVE inline void slam_start_application() {
  if (auto Factory = Private::ApplicationFactory()) {
    Private::Application() = Factory();
    Private::Application()->BeginPlay();
  }
}
extern "C" EMSCRIPTEN_KEEPALIVE inline void slam_tick(float DeltaTime) {
  if (Private::Application())
    Private::Application()->Tick(DeltaTime);
}
extern "C" EMSCRIPTEN_KEEPALIVE inline void slam_stop_application() {
  if (Private::Application()) {
    Private::Application()->EndPlay();
    Private::Application().reset();
  }
}
extern "C" EMSCRIPTEN_KEEPALIVE inline void slam_suspend_application(int Suspended) {
  if (Private::Application()) {
    if (Suspended)
      Private::Application()->OnSuspend();
    else
      Private::Application()->OnResume();
  }
}
extern "C" EMSCRIPTEN_KEEPALIVE inline void slam_resize_application() {
  if (Private::Application())
    Private::Application()->OnResize(FCanvas::GetSize());
}

} // namespace Slam

#define SLAM_PRIVATE_JOIN_INNER(A, B) A##B
#define SLAM_PRIVATE_JOIN(A, B) SLAM_PRIVATE_JOIN_INNER(A, B)
#define SLAM_BIND(Function)                                                            \
  static ::Slam::Private::TBindingRegistration<decltype(&Function)> SLAM_PRIVATE_JOIN( \
      SlamBinding_, __COUNTER__)(#Function, &Function)
#define SLAM_IMPLEMENT_APPLICATION(Type)                                               \
  static ::Slam::Private::TApplicationRegistration<Type> SLAM_PRIVATE_JOIN(            \
      SlamApplication_, __COUNTER__)
