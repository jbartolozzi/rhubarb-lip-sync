"""Rhubarb Lip Sync - automatic lip sync from voice recordings."""

from rhubarb._rhubarb import MouthCue
from rhubarb._rhubarb import animate as _animate

import os
import sys
import urllib.request
from pathlib import Path

VALID_WHISPER_MODELS = ("tiny", "base", "small", "medium", "large")


def get_default_model_dir():
    """Returns the default directory for storing Whisper model files."""
    # Use ~/.cache/rhubarb on Unix, ~/AppData/Local/rhubarb on Windows
    if sys.platform == "win32":
        base = Path(os.environ.get("LOCALAPPDATA", Path.home() / "AppData" / "Local"))
    else:
        base = Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))
    return base / "rhubarb" / "models"


def download_whisper_model(model_name="tiny", model_dir=None):
    """Download a Whisper GGML model file if not already present.

    Args:
        model_name: Model size - 'tiny' (~75MB), 'base' (~142MB), or 'small' (~466MB).
        model_dir: Directory to store the model. Defaults to ~/.cache/rhubarb/models/.

    Returns:
        Path to the downloaded model file.
    """
    if model_name not in VALID_WHISPER_MODELS:
        raise ValueError(
            f"Unknown model '{model_name}'. Choose from: {', '.join(VALID_WHISPER_MODELS)}"
        )

    if model_dir is None:
        model_dir = get_default_model_dir()
    else:
        model_dir = Path(model_dir)

    model_dir.mkdir(parents=True, exist_ok=True)
    model_path = model_dir / f"ggml-{model_name}.bin"

    if model_path.exists():
        return str(model_path)

    url = f"https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-{model_name}.bin"
    print(f"Downloading Whisper {model_name} model to {model_path}...")

    tmp_path = model_path.with_suffix(".bin.tmp")
    try:
        urllib.request.urlretrieve(url, tmp_path)
        tmp_path.rename(model_path)
    except Exception:
        if tmp_path.exists():
            tmp_path.unlink()
        raise

    print(f"Download complete: {model_path}")
    return str(model_path)


def _resolve_whisper_model(whisper_model):
    """Resolve a whisper_model argument to an absolute file path.

    Accepts:
        - A model name like "tiny", "base", "small" — auto-downloads if needed
        - An absolute or relative file path like "/path/to/ggml-tiny.bin"
        - An empty string (no model)

    Returns:
        Resolved file path as a string, or empty string if no model specified.
    """
    if not whisper_model:
        return ""

    # If it's a known model name, download/cache it
    if whisper_model in VALID_WHISPER_MODELS:
        return download_whisper_model(whisper_model)

    # Otherwise treat it as a file path
    path = Path(whisper_model).expanduser().resolve()
    if not path.exists():
        raise FileNotFoundError(
            f"Whisper model not found at: {path}\n"
            f"Pass a model name ({', '.join(VALID_WHISPER_MODELS)}) to auto-download, "
            f"or provide a valid file path."
        )
    return str(path)


def animate(
    input_file,
    dialog="",
    recognizer="pocketSphinx",
    extended_shapes="GHX",
    threads=0,
    framerate=0,
    whisper_model="",
):
    """Analyze an audio file and generate lip sync mouth cues.

    Args:
        input_file: Path to a WAVE (.wav) or Ogg Vorbis (.ogg) audio file.
        dialog: Optional dialog text to improve recognition accuracy.
        recognizer: Speech recognizer to use: 'pocketSphinx' (English),
            'phonetic' (any language), or 'whisper' (English, best accuracy).
        extended_shapes: Extended mouth shapes to use. Default 'GHX'.
            Use 'GHIJKLX' for all, or '' for basic shapes only (A-F).
        threads: Max worker threads. 0 means use all available CPU cores.
        framerate: Target animation frame rate in fps (e.g. 12, 24). Shape
            transitions are snapped to frame boundaries. 0 means no frame
            snapping (default).
        whisper_model: Whisper model name or file path. Pass a model name
            ('tiny', 'base', 'small') to auto-download, or an absolute path
            to a GGML model file. Only used with recognizer='whisper'.

    Returns:
        List of MouthCue objects with start, end (seconds), and shape attributes.
    """
    resolved_model = _resolve_whisper_model(whisper_model)
    return _animate(
        input_file,
        dialog=dialog,
        recognizer=recognizer,
        extended_shapes=extended_shapes,
        threads=threads,
        framerate=framerate,
        whisper_model=resolved_model,
    )


__all__ = ["MouthCue", "animate", "download_whisper_model", "get_default_model_dir"]
