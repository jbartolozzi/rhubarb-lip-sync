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
