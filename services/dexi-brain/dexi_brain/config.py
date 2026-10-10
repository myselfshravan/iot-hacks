"""Settings from .env (see .env.example)."""

import os
from dataclasses import dataclass
from pathlib import Path

from dotenv import load_dotenv

load_dotenv(Path(__file__).resolve().parent.parent / ".env")


@dataclass(frozen=True)
class Config:
    llm_base_url: str
    llm_api_key: str
    llm_model: str
    dexi_url: str
    owner: str
    latitude: float
    longitude: float
    think_every_s: int
    share_active_app: bool


def load() -> Config:
    key = os.environ.get("LLM_API_KEY", "")
    if not key:
        raise SystemExit("LLM_API_KEY is empty: copy .env.example to .env and fill it in")
    return Config(
        llm_base_url=os.environ.get("LLM_BASE_URL", "https://api.openai.com/v1"),
        llm_api_key=key,
        llm_model=os.environ.get("LLM_MODEL", "gpt-4o-mini"),
        dexi_url=os.environ.get("DEXI_URL", "http://dexi.local").rstrip("/"),
        owner=os.environ.get("OWNER_NAME", "my human"),
        latitude=float(os.environ.get("LATITUDE", "12.97")),
        longitude=float(os.environ.get("LONGITUDE", "77.59")),
        think_every_s=int(os.environ.get("THINK_EVERY_S", "180")),
        share_active_app=os.environ.get("SHARE_ACTIVE_APP", "1") == "1",
    )
