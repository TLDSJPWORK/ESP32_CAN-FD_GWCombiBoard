#!/usr/bin/env python3
"""Verify isolated and finite gateway traffic, without guard/flush frames.

Run on the Linux peer with two independent SocketCAN buses connected to the
gateway. Both interfaces must already be configured for CAN FD 500k/2M.
This test sends generated traffic; use a dedicated test setup.
"""
import argparse
from collections import Counter
import json
import select
import socket
import struct
import time


def key(raw):
    can_id, length, flags = struct.unpack_from("=IBB", raw)
    is_fd = len(raw) == 72
    payload = b"" if can_id & socket.CAN_RTR_FLAG else raw[8:8 + length]
    return can_id, length, is_fd, (flags & 1 if is_fd else 0), payload


def frame(source, index, kind):
    can_id = (0x100 if source == 0 else 0x500) + index
    if index % 2:
        can_id |= socket.CAN_EFF_FLAG | 0x18000000
    is_fd = kind != "classic"
    length = ([0, 1, 8, 12, 16, 20, 24, 32, 48, 64][index % 10]
              if is_fd else index % 9)
    if not is_fd and index % 10 == 9:
        can_id |= socket.CAN_RTR_FLAG
    payload = bytes((index + j + source * 83) % 256 for j in range(length))
    return (struct.pack("=IBB2x", can_id & 0xffffffff, length, int(kind == "fd-brs"))
            + payload.ljust(64 if is_fd else 8, b"\0"))


def run_phase(sockets, kind, label, sources, count, interval, quiet):
    expected = [[], []]
    received = [[], []]
    next_send = time.monotonic()
    sent = 0
    last_send = None
    arrival_ms = []
    while True:
        now = time.monotonic()
        if sent < count and now >= next_send:
            for source in sources:
                index = (8 if kind == "classic" else 9) if count == 1 else sent
                raw = frame(source, index, kind)
                sockets[source].send(raw)
                expected[1 - source].append(key(raw))
            sent += 1
            next_send = now + interval
            if sent == count:
                last_send = now
        for peer in select.select(sockets, [], [], 0.001)[0]:
            while True:
                try:
                    raw = peer.recv(72)
                except BlockingIOError:
                    break
                received[sockets.index(peer)].append(key(raw))
                if count == 1 and last_send is not None:
                    arrival_ms.append(round((time.monotonic() - last_send) * 1000, 3))
        # Always leave the buses idle after the final frame, including when
        # all frames arrived. Never send extra traffic to prompt an interrupt.
        if last_send is not None and time.monotonic() - last_send >= quiet:
            break
    results = []
    for destination in range(2):
        want, got = Counter(expected[destination]), Counter(received[destination])
        results.append({
            "interface": sockets[destination].getsockname()[0],
            "expected": sum(want.values()), "received": sum(got.values()),
            "missing": sum((want - got).values()),
            "unexpected_or_duplicate": sum((got - want).values()),
            "order_matches": expected[destination] == received[destination],
        })
    passed = expected == received
    print(json.dumps({"kind": kind, "phase": label, "count_per_source": count,
                      "quiet_seconds": quiet, "guard_frames": False,
                      "single_arrival_ms": arrival_ms, "results": results,
                      "pass": passed}), flush=True)
    return passed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("interfaces", nargs=2, metavar="IFACE")
    parser.add_argument("--count", type=int, default=100)
    parser.add_argument("--interval", type=float, default=0.005)
    parser.add_argument("--quiet", type=float, default=1.0)
    args = parser.parse_args()
    if not 1 <= args.count <= 256 or args.interval <= 0 or args.quiet < 0.1:
        parser.error("count must be 1..256, interval > 0, quiet >= 0.1")
    peers = []
    try:
        for name in args.interfaces:
            peer = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
            peers.append(peer)
            peer.setsockopt(socket.SOL_CAN_RAW, socket.CAN_RAW_FD_FRAMES, 1)
            peer.bind((name,))
            peer.setblocking(False)
        for kind in ("classic", "fd-no-brs", "fd-brs"):
            for count, size in ((1, "isolated"), (args.count, "finite")):
                for label, sources in (("0_to_1", (0,)), ("1_to_0", (1,)),
                                       ("simultaneous", (0, 1))):
                    if not run_phase(peers, kind, size + "_" + label, sources,
                                     count, args.interval, args.quiet):
                        return 1
        return 0
    finally:
        for peer in peers:
            peer.close()


if __name__ == "__main__":
    raise SystemExit(main())
