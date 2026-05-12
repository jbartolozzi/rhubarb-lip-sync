#ifndef RHUBARB_H
#define RHUBARB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RHUBARB_OK = 0,
    RHUBARB_ERROR_INVALID_ARGUMENT = 1,
    RHUBARB_ERROR_FILE_NOT_FOUND = 2,
    RHUBARB_ERROR_RECOGNITION_FAILED = 3,
    RHUBARB_ERROR_INTERNAL = 99
} RhubarbStatus;

typedef enum {
    RHUBARB_RECOGNIZER_POCKET_SPHINX = 0,
    RHUBARB_RECOGNIZER_PHONETIC = 1
} RhubarbRecognizer;

typedef struct {
    double start_seconds;
    double end_seconds;
    char shape;
    char _padding[7];
} RhubarbCue;

typedef struct {
    RhubarbRecognizer recognizer;
    const char* dialog;
    const char* extended_shapes;
    int framerate;
    int thread_count;

    /*
     * Optional ARKit blendshape track aligned to the same audio file.
     * Layout is flat row-major: the j-th blendshape weight for frame i
     * lives at blendshape_values[i * blendshape_values_per_frame + j],
     * and the timestamp (in seconds, monotonically increasing) for that
     * frame is blendshape_timestamps[i].
     *
     * Set blendshape_frame_count = 0 (and/or the pointers to NULL) to
     * indicate no blendshape data is available — the recognizer must
     * fall back to its audio-only behavior in that case.
     *
     * The caller retains ownership of both arrays; rhubarb_animate may
     * only read them during the call.
     */
    const double* blendshape_timestamps;
    const float* blendshape_values;
    size_t blendshape_frame_count;
    size_t blendshape_values_per_frame;

    /*
     * Optional blendshape-driven voice activity gating. When enabled
     * (and blendshape data is supplied), rhubarb intersects its audio
     * VAD output with a mask derived from jawOpen exceeding the
     * threshold, so utterances are kept only where both audio AND
     * visual evidence agree on speech. Cuts out cue runs caused by
     * background noise.
     *
     * When `vad_use_blendshape_mask` is false (the default) the next
     * two fields are ignored and behavior matches upstream.
     */
    int vad_use_blendshape_mask;      /* nonzero = enabled */
    float vad_jaw_open_threshold;     /* sensible default 0.05 */
    int vad_min_silence_ms;           /* sensible default 200 */
} RhubarbOptions;

void rhubarb_default_options(RhubarbOptions* options);

void rhubarb_set_resource_directory(const char* path);

RhubarbStatus rhubarb_animate(
    const char* audio_path,
    const RhubarbOptions* options,
    RhubarbCue** out_cues,
    size_t* out_count
);

void rhubarb_free_cues(RhubarbCue* cues);

const char* rhubarb_last_error(void);

const char* rhubarb_version(void);

#ifdef __cplusplus
}
#endif

#endif
