import rhubarb

dialog = open("audio_samples/sample_transcription.txt").read()
cues = rhubarb.animate(
    "audio_samples/sample_audio.wav",
    dialog=dialog,
    extended_shapes="GHIJKLX",
)

for cue in cues:
    print(f"{cue.start:.2f}\t{cue.end:.2f}\t{cue.shape}")
