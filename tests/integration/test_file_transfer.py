"""
Integration tests for peer-to-peer file transfer (HotspotFile, type 0x2001).

The file transfer wire format lives in hotspotchat.cpp (client-only, Qt-specific),
but the wire bytes are plain binary so we can generate and verify them here
without any Qt dependency.

Wire layout for HotspotFile payload:
  [u32 LE name_len][name bytes][u32 LE data_len][file bytes]
"""

import pytest
from conftest import (
    pack_file,
    recv_redirect,
    register,
    unpack_file,
)


def _pair(make_client):
    a = make_client()
    b = make_client()
    register(a)
    register(b)
    ip_for_a, port_for_a = recv_redirect(a)
    ip_for_b, port_for_b = recv_redirect(b)
    return a, b, (ip_for_a, port_for_a), (ip_for_b, port_for_b)


def test_small_file_transfer(server, make_client):
    a, b, addr_b, _ = _pair(make_client)

    content = b"Hello, file world!"
    a.sendto(pack_file("hello.txt", content), addr_b)

    data, _ = b.recvfrom(65535)
    name, received = unpack_file(data)

    assert name == "hello.txt"
    assert received == content


def test_file_with_binary_content(server, make_client):
    a, b, addr_b, _ = _pair(make_client)

    content = bytes(range(256)) * 4  # 1 KB of binary data
    a.sendto(pack_file("binary.bin", content), addr_b)

    data, _ = b.recvfrom(65535)
    name, received = unpack_file(data)

    assert name == "binary.bin"
    assert received == content


def test_empty_file(server, make_client):
    a, b, addr_b, _ = _pair(make_client)

    a.sendto(pack_file("empty.dat", b""), addr_b)

    data, _ = b.recvfrom(65535)
    name, received = unpack_file(data)

    assert name == "empty.dat"
    assert received == b""


def test_file_name_with_spaces_and_unicode(server, make_client):
    a, b, addr_b, _ = _pair(make_client)

    name = "мой файл.txt"
    content = b"content"
    a.sendto(pack_file(name, content), addr_b)

    data, _ = b.recvfrom(65535)
    recv_name, recv_content = unpack_file(data)

    assert recv_name == name
    assert recv_content == content


def test_file_transfer_both_directions(server, make_client):
    a, b, addr_b, addr_a = _pair(make_client)

    a.sendto(pack_file("from_a.txt", b"data from A"), addr_b)
    b.sendto(pack_file("from_b.txt", b"data from B"), addr_a)

    data_b, _ = b.recvfrom(65535)
    data_a, _ = a.recvfrom(65535)

    assert unpack_file(data_b) == ("from_a.txt", b"data from A")
    assert unpack_file(data_a) == ("from_b.txt", b"data from B")
