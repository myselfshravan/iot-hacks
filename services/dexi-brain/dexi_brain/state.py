"""State shared between the think loop and the console server."""

import threading
import time
from dataclasses import dataclass, field


@dataclass
class Shared:
    lock: threading.RLock = field(default_factory=threading.RLock)
    dexi_state: dict = field(default_factory=dict)
    last_context: dict = field(default_factory=dict)
    next_think: float = 0.0
    status: str = "starting"
    think_now: threading.Event = field(default_factory=threading.Event)
    started: float = field(default_factory=time.time)
