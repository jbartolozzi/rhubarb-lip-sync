import rhubarb

dialog = open("audio_samples/sample_transcription.txt").read()

print("=== PocketSphinx ===")
cues = rhubarb.animate(
    "audio_samples/sample_audio.wav",
    dialog=dialog,
    extended_shapes="GHIJKLX",
    framerate=12,
)
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

print()
print("=== Whisper (tiny) ===")
cues = rhubarb.animate(
    "audio_samples/sample_audio.wav",
    dialog=dialog,
    extended_shapes="GHIJKLX",
    framerate=12,
    recognizer="whisper",
    whisper_model="tiny",
)
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")
