"""
Integration tests for the router redirect flow.
The server should cross-introduce two peers when they both register.
"""

import pytest
from conftest import recv_redirect, register


def test_two_clients_both_receive_redirect(server, make_client):
    a = make_client()
    b = make_client()

    register(a)
    register(b)

    ip_for_a, port_for_a = recv_redirect(a)
    ip_for_b, port_for_b = recv_redirect(b)

    _, port_a = a.getsockname()
    _, port_b = b.getsockname()

    # Each client should have been redirected to the other's port.
    assert port_for_a == port_b, "A was not redirected to B's port"
    assert port_for_b == port_a, "B was not redirected to A's port"
    assert ip_for_a == "127.0.0.1"
    assert ip_for_b == "127.0.0.1"


def test_single_client_gets_no_redirect(server, make_client):
    """A lone client should not receive a redirect — server stores it and waits."""
    a = make_client()
    register(a)
    a.settimeout(0.5)

    with pytest.raises(TimeoutError):
        a.recvfrom(1500)


def test_same_client_re_registers_no_self_redirect(server, make_client):
    """
    If the same socket registers twice, the server should update the waiting slot
    rather than redirect the peer to itself.
    """
    a = make_client()
    register(a)
    register(a)  # re-register from same address
    a.settimeout(0.5)

    with pytest.raises(TimeoutError):
        a.recvfrom(1500)


def test_third_client_pairs_with_fresh_slot(server, make_client):
    """
    After two clients are paired and the slot resets, a third client
    should be stored as the new waiting peer without receiving a redirect.
    """
    a = make_client()
    b = make_client()
    c = make_client()

    register(a)
    register(b)

    # Drain A and B redirects.
    recv_redirect(a)
    recv_redirect(b)

    # C registers — slot is empty, so no redirect yet.
    register(c)
    c.settimeout(0.5)
    with pytest.raises(TimeoutError):
        c.recvfrom(1500)
