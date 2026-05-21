# hApp

A peer-to-peer instant messaging system built in C++20. Consists of a Qt6/QML client, a lightweight UDP router/matchmaker server, and a shared hProto binary protocol library.

## Tech Stack

- **Language**: C++20
- **Client UI**: Qt6 Quick (QML)
- **Server**: POSIX UDP sockets (no Qt)
- **Protocol**: hProto — custom binary serialization (template metaprogramming)
- **Build**: CMake 3.16+, Qt 6.8+ (client only)

## Repository Layout

```
CMakeLists.txt        - Root build: adds common, client, server
common/
  hproto.h            - Binary serialization framework (Qt-free)
  hproto_types.h      - Shared wire types: RouterCreateWaitroomRequest,
                        RouterRedirectAnswer, RouterGreet
  CMakeLists.txt      - INTERFACE library target "hproto"
client/
  main.cpp            - Qt app entry point
  Main.qml            - UI (640×480, address bar, message list, file drop)
  hotspotchat.h/cpp   - HotspotChat: QUdpSocket subclass, protocol logic
  CMakeLists.txt      - Qt6 executable, links hproto
server/
  main.cpp            - UDP matchmaker server (POSIX sockets, no Qt)
  CMakeLists.txt      - Plain C++ executable, links hproto
```

## Build

```sh
# Everything (client + server)
cmake -B build
cmake --build build

# Server only (no Qt required)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target hServer
```

## Architecture

### Client

`HotspotChat` extends `QUdpSocket` and is exposed as a QML element. The QML layer calls `send(text)`, `sendFile(url)`, and `greetAddress(url)` directly. Incoming datagrams are handled in the `readyRead()` slot.

Connection flow:
1. User enters router address → `setUrl()` sends `RouterCreateWaitroomRequest`
2. Router replies with `RouterRedirectAnswer` (peer IP:port)
3. Client auto-sends `RouterGreet` to the peer
4. Direct P2P text and file exchange begins

### Server

Single-threaded synchronous UDP loop. Stores one waiting peer; when a second peer registers it sends `RouterRedirectAnswer` to both, then resets. Listens on port **17171**.

## hProto Protocol

Each datagram is `[4-byte type ID][payload]`, little-endian only (compile error on big-endian). All IP/port values in the payload are **host byte order**.

| Type ID  | Type                          | Payload layout                              |
|----------|-------------------------------|---------------------------------------------|
| `0x0002` | `std::string`                 | `[u32 len][bytes]`                          |
| `0x1002` | `RouterCreateWaitroomRequest` | `[u32 ip][u16 port][2 pad]` (8 bytes)       |
| `0x1003` | `RouterRedirectAnswer`        | `[u32 ip][u16 port][2 pad]` (8 bytes)       |
| `0x1004` | `RouterGreet`                 | empty                                       |
| `0x2001` | `HotspotFile`                 | `[u32 name_len][name][u32 data_len][data]`  |

Fixed-size types use `HOTSPOT_SIZED_OBJECT(name, id, size)` — enforced via `static_assert`. Empty types use `HOTSPOT_EMPTY_OBJECT`. Unknown type IDs deserialize to `std::monostate` and are silently discarded.

`HotspotFile` is client-only (depends on `QByteArray`) and lives in `client/hotspotchat.cpp`.

## Known Limitations / TODOs

- `ConnectionState` enum (`Disconnected` / `ConnectedToRouter` / `ConnectedToPeer`) is defined but never transitions — `connected` property always returns `false`
- Router address is hardcoded in `Main.qml` (`Component.onCompleted`)
- No encryption, authentication, or integrity verification
- UDP with no reliability layer — messages can be lost or arrive out of order
- File transfer sends entire file as one datagram — will fail silently above MTU
- `hproto_accepts_size` for strings does not validate declared length against actual remaining bytes
- Server only supports IPv4
