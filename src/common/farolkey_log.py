"""FarolKey Python logging — shared by clipboard, screenshot, template tools.

PRIVACY RULES (strictly enforced by convention):
  - NEVER log user-typed text, clipboard content, or dict/template expansions.
  - Safe to log: operation names, char counts, state changes, error messages,
    file paths of config files (not content), tool availability, timestamps.

Usage:
    from farolkey_log import get_logger
    log = get_logger('clipboard')
    log.info('popup opened, %d items', n)
    log.error('seat grab failed: %s', err)
"""

import logging
import logging.handlers
import os
import platform
import shutil
import subprocess
import time
import zipfile
from pathlib import Path


def _log_path() -> Path:
    xdg = os.environ.get('XDG_CACHE_HOME', '')
    base = Path(xdg) if xdg else Path.home() / '.cache'
    d = base / 'farolkey'
    d.mkdir(parents=True, exist_ok=True)
    return d / 'tools.log'


def _setup_root_logger():
    root = logging.getLogger('farolkey')
    if root.handlers:
        return  # already configured

    root.setLevel(logging.DEBUG)

    # Rotating file handler: 5 MB max, 1 backup
    fh = logging.handlers.RotatingFileHandler(
        _log_path(),
        maxBytes=5 * 1024 * 1024,
        backupCount=1,
        encoding='utf-8',
    )
    fh.setLevel(logging.DEBUG)
    fh.setFormatter(logging.Formatter(
        '%(asctime)s.%(msecs)03d [%(levelname)-5s] [%(name)s] %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
    ))
    root.addHandler(fh)


def get_logger(component: str) -> logging.Logger:
    """Return a logger for *component* (e.g. 'clipboard', 'screenshot')."""
    _setup_root_logger()
    return logging.getLogger(f'farolkey.{component}')


def _cache_dir() -> Path:
    xdg = os.environ.get('XDG_CACHE_HOME', '')
    return (Path(xdg) if xdg else Path.home() / '.cache') / 'farolkey'


def _system_info() -> str:
    """Collect non-sensitive system info for bug reports."""
    lines = [
        f'date:           {time.strftime("%Y-%m-%d %H:%M:%S")}',
        f'os:             {platform.freedesktop_os_release().get("PRETTY_NAME", platform.system())}',
        f'kernel:         {platform.release()}',
        f'session_type:   {os.environ.get("XDG_SESSION_TYPE", "unknown")}',
        f'desktop:        {os.environ.get("XDG_CURRENT_DESKTOP", "unknown")}',
        f'display:        {os.environ.get("WAYLAND_DISPLAY") or os.environ.get("DISPLAY") or "none"}',
    ]
    # fcitx5 version
    try:
        r = subprocess.run(['fcitx5', '--version'], capture_output=True, text=True, timeout=3)
        lines.append(f'fcitx5:         {r.stdout.strip() or r.stderr.strip()}')
    except Exception:
        lines.append('fcitx5:         not found')
    # python version
    lines.append(f'python:         {platform.python_version()}')
    # tool availability
    for tool in ('grim', 'maim', 'wl-copy', 'xclip', 'xdotool', 'wtype', 'notify-send'):
        found = '✓' if shutil.which(tool) else '✗'
        lines.append(f'  {found} {tool}')
    return '\n'.join(lines) + '\n'


def export_log_bundle(dest_path: str) -> str:
    """Create a zip bundle of all FarolKey logs + system info at *dest_path*.

    Returns the final path of the created zip file.
    """
    cache = _cache_dir()
    log_files = [
        (cache / 'farolkey.log',  'farolkey.log'),   # C++ engine
        (cache / 'farolkey.log.1', 'farolkey.log.1'),
        (cache / 'tools.log',     'tools.log'),       # Python tools
        (cache / 'tools.log.1',   'tools.log.1'),
    ]

    with zipfile.ZipFile(dest_path, 'w', zipfile.ZIP_DEFLATED) as zf:
        for src, arcname in log_files:
            if src.exists():
                zf.write(src, arcname)
        zf.writestr('system_info.txt', _system_info())

    return dest_path
