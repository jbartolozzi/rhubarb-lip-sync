import rhubarb

dialog = open("audio_samples/sample_transcription2.txt").read()
audio = "audio_samples/sample_audio2.wav"
shapes = "GHIJKLX"
fps = 12

results = {}

print("=== PocketSphinx WITH dialog ===")
cues = rhubarb.animate(audio, dialog=dialog, extended_shapes=shapes, framerate=fps)
results["ps+dialog"] = cues
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

print()
print("=== PocketSphinx WITHOUT dialog ===")
cues = rhubarb.animate(audio, extended_shapes=shapes, framerate=fps)
results["ps"] = cues
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

print()
print("=== Whisper (tiny) ===")
cues = rhubarb.animate(audio, extended_shapes=shapes, framerate=fps,
                       recognizer="whisper", whisper_model="tiny")
results["whisper-tiny"] = cues
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

print()
print("=== Whisper (base) ===")
cues = rhubarb.animate(audio, extended_shapes=shapes, framerate=fps,
                       recognizer="whisper", whisper_model="base")
results["whisper-base"] = cues
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

print()
print("=== Hybrid: Whisper (tiny) + PocketSphinx ===")
cues = rhubarb.animate(audio, extended_shapes=shapes, framerate=fps,
                       recognizer="whisperPocketSphinx", whisper_model="tiny")
results["hybrid-tiny"] = cues
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

print()
print("=== Hybrid: Whisper (base) + PocketSphinx ===")
cues = rhubarb.animate(audio, extended_shapes=shapes, framerate=fps,
                       recognizer="whisperPocketSphinx", whisper_model="base")
results["hybrid-base"] = cues
for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")

# Summary comparison
print()
print("=== Summary ===")
for name, cues in results.items():
    shapes_str = "".join(c.shape for c in cues)
    print(f"  {name:20s}: {len(cues):2d} cues | {shapes_str}")
