#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "labels.h"

extern "C" int ajit1_tflite_read(const char *path, void *buf, size_t need);
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

extern "C" {
extern const uint8_t _binary_model_tflite_start[];
extern const uint8_t _binary_model_tflite_end[];
}

namespace {

constexpr int kTensorArenaSize = 64 * 1024 * 1024;
alignas(16) uint8_t g_arena[kTensorArenaSize];

void usage(void) {
  printf("usage: /tflite/resnet50 <file>\n");
  printf("file: 150528 raw bytes, 224x224x3\n");
  printf("example: /tflite/resnet50 /tflite/inputs/resnet50/hopper\n");
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

void model_error(void) {
  printf("resnet50: model error\n");
}

float dequant(const TfLiteTensor *t, int i) {
  if (t->type == kTfLiteFloat32) {
    return t->data.f[i];
  }
  if (t->type == kTfLiteInt8) {
    return (static_cast<int>(t->data.int8[i]) - t->params.zero_point) *
           t->params.scale;
  }
  if (t->type == kTfLiteUInt8) {
    return (static_cast<int>(t->data.uint8[i]) - t->params.zero_point) *
           t->params.scale;
  }
  return 0.f;
}

}  // namespace

extern "C" int resnet50_main(int argc, char *argv[]) {
  if (help_arg(argc, argv)) {
    usage();
    return 1;
  }

  tflite::InitializeTarget();
  const tflite::Model *model = tflite::GetModel(_binary_model_tflite_start);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    model_error();
    return 1;
  }

  tflite::MicroMutableOpResolver<24> ops;
  if (ops.AddAdd() != kTfLiteOk || ops.AddAveragePool2D() != kTfLiteOk ||
      ops.AddConcatenation() != kTfLiteOk || ops.AddConv2D() != kTfLiteOk ||
      ops.AddDepthwiseConv2D() != kTfLiteOk ||
      ops.AddDequantize() != kTfLiteOk ||
      ops.AddFullyConnected() != kTfLiteOk || ops.AddMaxPool2D() != kTfLiteOk ||
      ops.AddMean() != kTfLiteOk || ops.AddMul() != kTfLiteOk ||
      ops.AddPad() != kTfLiteOk || ops.AddPadV2() != kTfLiteOk ||
      ops.AddQuantize() != kTfLiteOk || ops.AddRelu() != kTfLiteOk ||
      ops.AddRelu6() != kTfLiteOk || ops.AddReshape() != kTfLiteOk ||
      ops.AddSoftmax() != kTfLiteOk || ops.AddSqueeze() != kTfLiteOk ||
      ops.AddSub() != kTfLiteOk) {
    model_error();
    return 1;
  }

  memset(g_arena, 0, kTensorArenaSize);
  tflite::MicroInterpreter interpreter(model, ops, g_arena, kTensorArenaSize);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    model_error();
    return 1;
  }

  TfLiteTensor *input = interpreter.input(0);
  TfLiteTensor *output = interpreter.output(0);
  if (input == nullptr || output == nullptr || input->bytes == 0) {
    model_error();
    return 1;
  }
  if (ajit1_tflite_read(argv[1], input->data.data, input->bytes) != 0) {
    usage();
    return 1;
  }
  if (interpreter.Invoke() != kTfLiteOk) {
    model_error();
    return 1;
  }

  int n = 0;
  if (output->dims != nullptr && output->dims->size > 0) {
    n = output->dims->data[output->dims->size - 1];
  }
  int top_i = -1;
  float top_s = 0.f;
  for (int i = 0; i < n; i++) {
    float score = dequant(output, i);
    if (top_i < 0 || score > top_s) {
      top_s = score;
      top_i = i;
    }
  }
  if (top_i < 0 || top_i >= kLabelCount) {
    model_error();
    return 1;
  }
  printf("top1: %d %s\n", top_i, kLabels[top_i]);
  (void)_binary_model_tflite_end;
  return 0;
}
