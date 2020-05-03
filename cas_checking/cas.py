from typing import NamedTuple


class CAS(NamedTuple):
    value_from: int
    value_to: int
    result: bool
