# hClient

A peer-to-peer instant messaging client built with C++ (C++20) and Qt6/QML. Communicates over UDP using a custom binary protocol called **hProto**.

## Tech Stack

- **Language**: C++20
- **UI**: Qt6 Quick (QML)
- **Networking**: `QUdpSocket` (UDP)
- **Build**: CMake 3.16+, Qt 6.8+

## Build

```sh
cmake -B build
cmake --build build
```

## Project Structure

```
main.cpp          - Qt app entry point
Main.qml          - UI (640×480 window, address bar, message list, file drop)
hotspotchat.h/cpp - HotspotChat class: QUdpSocket subclass, protocol logic
hproto.h          - Binary serialization framework (template metaprogramming)
CMakeLists.txt    - Build config
```

## Architecture

`HotspotChat` extends `QUdpSocket` and is exposed as a QML element. The QML layer calls `send(text)`, `sendFile(url)`, and `greetAddress(url)` directly. Incoming datagrams are handled in the `readyRead()` slot.

Connection flow:
1. User enters router address → `setUrl()` sends `RouterCreateWaitroomRequest`
2. Router replies with `RouterRedirectAnswer` (peer IP:port)
3. Client auto-sends `RouterGreet` to the peer
4. Direct P2P text and file exchange begins

## hProto Protocol

Each datagram is `[4-byte type ID][payload]`, little-endian only (compile error on big-endian).

| Type ID  | Type                        | Payload layout                                      |
|----------|-----------------------------|-----------------------------------------------------|
| `0x0002` | `std::string`               | `[u32 len][bytes]`                                  |
| `0x1002` | `RouterCreateWaitroomRequest` | `[u32 ip][u16 port][2 pad]` (8 bytes)             |
| `0x1003` | `RouterRedirectAnswer`      | `[u32 ip][u16 port][2 pad]` (8 bytes)              |
| `0x1004` | `RouterGreet`               | empty                                               |
| `0x2001` | `HotspotFile`               | `[u32 name_len][name][u32 data_len][data]`         |

Fixed-size types use `HOTSPOT_SIZED_OBJECT(name, id, size)` which enforces size via `static_assert`. Empty types use `HOTSPOT_EMPTY_OBJECT`. Unknown type IDs deserialize to `std::monostate` and are discarded.

## Known Limitations / TODOs

- `ConnectionState` enum (`Disconnected` / `ConnectedToRouter` / `ConnectedToPeer`) is defined but never transitions — `connected` property always returns `false`
- Router address is hardcoded in `Main.qml` (`Component.onCompleted`)
- No encryption, authentication, or integrity verification
- UDP with no reliability layer — messages can be lost or arrive out of order
- File transfer sends entire file as one datagram — will fail silently above MTU
- `hproto_accepts_size` for strings does not validate declared length against actual remaining bytes
