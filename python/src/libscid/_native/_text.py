from __future__ import annotations

import ctypes


def encode(value: str | bytes) -> bytes:
    if isinstance(value, bytes):
        return value
    return value.encode("utf-8")


def decode_buffer(buffer: ctypes.Array[ctypes.c_char], size: int) -> str:
    return bytes(buffer.raw[:size]).decode("utf-8")
