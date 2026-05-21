"""
Shared fixtures and protocol helpers for hApp integration tests.

Wire format (little-endian, host byte order for IPs/ports in payloads):
  [u32 type_id] [payload...]

Type IDs:
  0x0002  std::string      [u32 len][utf-8 bytes]
  0x1002  WaitroomRequest  [u32 ip][u16 port][u16 pad]
  0x1003  RedirectAnswer   [u32 ip][u16 port][u16 pad]
  0x1004  RouterGreet      (no payload)
  0x2001  HotspotFile      [u32 name_len][name][u32 data_len][data]
"""

import os
import socket
import struct
import subprocess
import time
from pathlib import Path

import pytest

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

BUILD_DIR   = Path(os.environ.get("BUILD_DIR", "build"))
SERVER_BIN  = BUILD_DIR / "server" / "hServer"
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 17171

TYPE_STRING   = 0x0002
TYPE_WAITROOM = 0x1002
TYPE_REDIRECT = 0x1003
TYPE_GREET    = 0x1004
TYPE_FILE     = 0x2001

RECV_TIMEOUT  = 3.0  # seconds

# ---------------------------------------------------------------------------
# Protocol pack / unpack helpers
# ---------------------------------------------------------------------------

def pack_waitroom(ip_host: int, port: int) -> bytes:
    """RouterCreateWaitroomRequest: type_id + ip (host BO) + port (host BO) + pad."""
    return struct.pack("<IIHH", TYPE_WAITROOM, ip_host, port, 0)


def unpack_redirect(data: bytes) -> tuple[int, int]:
    """RouterRedirectAnswer → (ip_host_bo, port)."""
    type_id, ip, port, _pad = struct.unpack("<IIHH", data[:12])
    assert type_id == TYPE_REDIRECT, f"Expected 0x{TYPE_REDIRECT:04X}, got 0x{type_id:04X}"
    return ip, port


def ip_from_host_bo(ip_host: int) -> str:
    """Convert a host-byte-order uint32 IP to dotted-decimal string."""
    return socket.inet_ntoa(struct.pack("!I", ip_host))


def pack_greet() -> bytes:
    return struct.pack("<I", TYPE_GREET)


def pack_string(text: str) -> bytes:
    encoded = text.encode("utf-8")
    return struct.pack("<II", TYPE_STRING, len(encoded)) + encoded


def unpack_string(data: bytes) -> str:
    type_id, length = struct.unpack_from("<II", data)
    assert type_id == TYPE_STRING
    return data[8 : 8 + length].decode("utf-8")


def pack_file(name: str, content: bytes) -> bytes:
    name_bytes = name.encode("utf-8")
    return (
        struct.pack("<II", TYPE_FILE, len(name_bytes))
        + name_bytes
        + struct.pack("<I", len(content))
        + content
    )


def unpack_file(data: bytes) -> tuple[str, bytes]:
    type_id, name_len = struct.unpack_from("<II", data)
    assert type_id == TYPE_FILE
    name = data[8 : 8 + name_len].decode("utf-8")
    (data_len,) = struct.unpack_from("<I", data, 8 + name_len)
    content = data[8 + name_len + 4 : 8 + name_len + 4 + data_len]
    return name, content


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture
def server():
    """
    Start hServer and yield; terminate on teardown.

    Scope is per-test (default "function") so each test gets a clean server
    state — the server holds one waiting-peer slot that would otherwise leak
    between tests.
    """
    assert SERVER_BIN.exists(), (
        f"Server binary not found: {SERVER_BIN}. "
        "Run: cmake -B build && cmake --build build --target hServer"
    )
    proc = subprocess.Popen(
        [str(SERVER_BIN)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    # Block until the server prints its "listening" line (flushed immediately).
    ready_line = proc.stdout.readline()
    assert "listening" in ready_line, f"Unexpected startup line: {ready_line!r}"

    yield proc

    proc.terminate()
    try:
        proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        proc.kill()
    # Brief pause so the OS releases the UDP port before the next test's server starts.
    time.sleep(0.05)


@pytest.fixture
def make_client():
    """
    Factory fixture that creates bound UDP sockets acting as test peers.
    All sockets are closed after the test.
    """
    sockets = []

    def _make() -> socket.socket:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((SERVER_HOST, 0))
        sock.settimeout(RECV_TIMEOUT)
        sockets.append(sock)
        return sock

    yield _make

    for s in sockets:
        s.close()


def register(sock: socket.socket) -> None:
    """Send RouterCreateWaitroomRequest to the server."""
    _ip, port = sock.getsockname()
    # Server uses the actual UDP source address for redirects; payload IP is ignored.
    sock.sendto(pack_waitroom(0x7F000001, port), (SERVER_HOST, SERVER_PORT))


def recv_redirect(sock: socket.socket) -> tuple[str, int]:
    """Receive a RouterRedirectAnswer and return (ip_str, port)."""
    data, _ = sock.recvfrom(1500)
    ip_host, port = unpack_redirect(data)
    return ip_from_host_bo(ip_host), port
