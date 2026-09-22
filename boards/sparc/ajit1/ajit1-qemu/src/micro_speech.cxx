#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/examples/micro_speech/micro_model_settings.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "tensorflow/lite/micro/examples/micro_speech/models/audio_preprocessor_int8_model_data.cc"
#include "tensorflow/lite/micro/examples/micro_speech/models/micro_speech_quantized_model_data.cc"

extern "C" int ajit1_tflite_read(const char *path, void *buf, size_t need);

namespace {

constexpr size_t kArenaSize = 28584;
alignas(16) uint8_t g_arena[kArenaSize];

using Features = int8_t[kFeatureCount][kFeatureSize];
Features g_features;

constexpr int kAudioSampleDurationCount =
    kFeatureDurationMs * kAudioSampleFrequency / 1000;
constexpr int kAudioSampleStrideCount =
    kFeatureStrideMs * kAudioSampleFrequency / 1000;

constexpr size_t kClipSamples = 16000;
alignas(2) int16_t g_clip[kClipSamples];

void usage(void) {
  printf("usage: /tflite/micro_speech <file>\n");
  printf("file: 16000 big-endian int16 samples, 1000 ms\n");
  printf("example: /tflite/micro_speech /tflite/inputs/micro_speech/yes\n");
  printf("see /tflite/README.md\n");
}

bool help_arg(int argc, char *argv[]) {
  if (argc != 2) {
    return true;
  }
  const char *arg = argv[1];
  return strcmp(arg, "help") == 0 || strcmp(arg, "-h") == 0 ||
         strcmp(arg, "--help") == 0;
}

bool model_error(const char *where) {
  printf("micro_speech: model error\n");
  (void)where;
  return false;
}

bool generate_features(const int16_t *audio, size_t audio_size) {
  const tflite::Model *model =
      tflite::GetModel(g_audio_preprocessor_int8_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    return model_error("preprocessor version");
  }

  tflite::MicroMutableOpResolver<18> ops;
  if (ops.AddReshape() != kTfLiteOk || ops.AddCast() != kTfLiteOk ||
      ops.AddStridedSlice() != kTfLiteOk ||
      ops.AddConcatenation() != kTfLiteOk || ops.AddMul() != kTfLiteOk ||
      ops.AddAdd() != kTfLiteOk || ops.AddDiv() != kTfLiteOk ||
      ops.AddMinimum() != kTfLiteOk || ops.AddMaximum() != kTfLiteOk ||
      ops.AddWindow() != kTfLiteOk || ops.AddFftAutoScale() != kTfLiteOk ||
      ops.AddRfft() != kTfLiteOk || ops.AddEnergy() != kTfLiteOk ||
      ops.AddFilterBank() != kTfLiteOk ||
      ops.AddFilterBankSquareRoot() != kTfLiteOk ||
      ops.AddFilterBankSpectralSubtraction() != kTfLiteOk ||
      ops.AddPCAN() != kTfLiteOk || ops.AddFilterBankLog() != kTfLiteOk) {
    return model_error("preprocessor ops");
  }

  tflite::MicroInterpreter interpreter(model, ops, g_arena, kArenaSize);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    return model_error("preprocessor allocate");
  }

  size_t remaining = audio_size;
  size_t feature_index = 0;
  while (remaining >= static_cast<size_t>(kAudioSampleDurationCount) &&
         feature_index < kFeatureCount) {
    TfLiteTensor *input = interpreter.input(0);
    TfLiteTensor *output = interpreter.output(0);
    if (input == nullptr || output == nullptr) {
      return model_error("preprocessor tensors");
    }
    memcpy(input->data.i16, audio,
           kAudioSampleDurationCount * sizeof(int16_t));
    if (interpreter.Invoke() != kTfLiteOk) {
      return model_error("preprocessor invoke");
    }
    memcpy(g_features[feature_index], output->data.int8, kFeatureSize);
    feature_index++;
    audio += kAudioSampleStrideCount;
    remaining -= kAudioSampleStrideCount;
  }
  return true;
}

bool classify(void) {
  const tflite::Model *model =
      tflite::GetModel(g_micro_speech_quantized_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    return model_error("speech version");
  }

  tflite::MicroMutableOpResolver<4> ops;
  if (ops.AddReshape() != kTfLiteOk || ops.AddFullyConnected() != kTfLiteOk ||
      ops.AddDepthwiseConv2D() != kTfLiteOk || ops.AddSoftmax() != kTfLiteOk) {
    return model_error("speech ops");
  }

  tflite::MicroInterpreter interpreter(model, ops, g_arena, kArenaSize);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    return model_error("speech allocate");
  }

  TfLiteTensor *input = interpreter.input(0);
  TfLiteTensor *output = interpreter.output(0);
  if (input == nullptr || output == nullptr) {
    return model_error("speech tensors");
  }
  memcpy(input->data.int8, &g_features[0][0], kFeatureElementCount);
  if (interpreter.Invoke() != kTfLiteOk) {
    return model_error("speech invoke");
  }

  float best = 0.f;
  int best_index = 0;
  for (int i = 0; i < kCategoryCount; i++) {
    float score = (output->data.int8[i] - output->params.zero_point) *
                  output->params.scale;
    if (i == 0 || score > best) {
      best = score;
      best_index = i;
    }
  }
  printf("micro_speech: %s\n", kCategoryLabels[best_index]);
  return true;
}

}  // namespace

extern "C" int micro_speech_main(int argc, char *argv[]) {
  if (help_arg(argc, argv)) {
    usage();
    return 1;
  }

  if (ajit1_tflite_read(argv[1], g_clip, sizeof(g_clip)) != 0) {
    usage();
    return 1;
  }

  tflite::InitializeTarget();
  if (!generate_features(g_clip, kClipSamples)) {
    return 1;
  }
  if (!classify()) {
    return 1;
  }
  return 0;
}
