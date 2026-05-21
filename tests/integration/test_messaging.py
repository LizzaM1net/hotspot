"""
Integration tests for peer-to-peer text message exchange.

Flow: two clients register → server redirects them to each other →
      clients send messages directly (bypassing the server).
"""

import pytest
from conftest import (
    make_client,
    pack_greet,
    pack_string,
    recv_redirect,
    register,
    unpack_string,
)


def _pair(make_client):
    """Register two clients with the server and return (a, b, addr_b, addr_a)."""
    a = make_client()
    b = make_client()

    register(a)
    register(b)

    ip_for_a, port_for_a = recv_redirect(a)
    ip_for_b, port_for_b = recv_redirect(b)

    return a, b, (ip_for_a, port_for_a), (ip_for_b, port_for_b)


def test_basic_text_message(server, make_client):
    a, b, addr_b, addr_a = _pair(make_client)

    # Greet each other (optional for the server, but mirrors real client behaviour).
    a.sendto(pack_greet(), addr_b)
    b.sendto(pack_greet(), addr_a)
    b.recvfrom(1500)  # consume greet
    a.recvfrom(1500)

    # A → B
    a.sendto(pack_string("hello from A"), addr_b)
    data, _ = b.recvfrom(1500)
    assert unpack_string(data) == "hello from A"


def test_message_both_directions(server, make_client):
    a, b, addr_b, addr_a = _pair(make_client)

    a.sendto(pack_string("ping"), addr_b)
    b.sendto(pack_string("pong"), addr_a)

    data_b, _ = b.recvfrom(1500)
    data_a, _ = a.recvfrom(1500)

    assert unpack_string(data_b) == "ping"
    assert unpack_string(data_a) == "pong"


def test_empty_string_message(server, make_client):
    a, b, addr_b, _ = _pair(make_client)

    a.sendto(pack_string(""), addr_b)
    data, _ = b.recvfrom(1500)
    assert unpack_string(data) == ""


def test_unicode_message(server, make_client):
    a, b, addr_b, _ = _pair(make_client)

    msg = "Привет 🌍 世界"
    a.sendto(pack_string(msg), addr_b)
    data, _ = b.recvfrom(1500)
    assert unpack_string(data) == msg


def test_multiple_messages_in_sequence(server, make_client):
    a, b, addr_b, addr_a = _pair(make_client)

    messages = ["first", "second", "third"]
    for m in messages:
        a.sendto(pack_string(m), addr_b)

    received = []
    for _ in messages:
        data, _ = b.recvfrom(1500)
        received.append(unpack_string(data))

    assert received == messages
