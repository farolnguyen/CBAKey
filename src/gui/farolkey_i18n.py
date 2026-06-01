"""FarolKey i18n helpers — shared by all GUI scripts.

Usage:
    import sys, os
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from farolkey_i18n import setup_i18n, read_ui_language
    _ = setup_i18n(read_ui_language())
"""

import gettext
import os
from pathlib import Path


def _find_locales_dir() -> Path:
    env = os.environ.get('FAROLKEY_LOCALES')
    if env:
        return Path(env)
    # Installed layout: scripts live in <prefix>/bin/, locales in <prefix>/share/farolkey/locales/
    # e.g. ~/.local/bin/ → ~/.local/share/farolkey/locales/
    script_dir = Path(__file__).resolve().parent
    return script_dir.parent / 'share' / 'farolkey' / 'locales'


def setup_i18n(lang: str = 'en'):
    """Return a gettext callable for *lang*. Falls back to passthrough on missing .mo."""
    if lang == 'en':
        return str
    locales_dir = _find_locales_dir()
    try:
        t = gettext.translation('farolkey', localedir=str(locales_dir), languages=[lang])
        return t.gettext
    except FileNotFoundError:
        return str


def read_ui_language() -> str:
    """Read language= from farolkey.conf. Returns 'en' if absent or unreadable."""
    xdg = os.environ.get('XDG_CONFIG_HOME', '')
    base = Path(xdg) if xdg else Path.home() / '.config'
    conf = base / 'farolkey' / 'farolkey.conf'
    try:
        for line in conf.read_text(encoding='utf-8').splitlines():
            line = line.strip()
            if line.startswith('language='):
                val = line.split('=', 1)[1].strip()
                return val if val in ('en', 'vi') else 'en'
    except (FileNotFoundError, OSError):
        pass
    return 'en'
