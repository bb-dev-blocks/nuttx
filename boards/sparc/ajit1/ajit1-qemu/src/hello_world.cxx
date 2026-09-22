#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" int ajit1_tflite_read_text(const char *path, char *buf, size_t cap);

#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "tensorflow/lite/micro/examples/hello_world/models/hello_world_int8_model_data.cc"

namespace {

constexpr int kTensorArenaSize = 8192;
uint8_t g_tensor_arena[kTensorArenaSize];

void usage(void) {
  printf("usage: /tflite/hello_world <file>\n");
  printf("file: one number, such as 0, 1.57, or 3.14\n");
  printf("example: /tflite/hello_world /tflite/inputs/hello_world/0\n");
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

}  // namespace

extern "C" int hello_world_main(int argc, char *argv[]) {
  if (help_arg(argc, argv)) {
    usage();
    return 1;
  }

  char text[32];
  if (ajit1_tflite_read_text(argv[1], text, sizeof(text)) <= 0) {
    usage();
    return 1;
  }

  char *end = nullptr;
  float x = strtof(text, &end);
  if (end == text) {
    usage();
    return 1;
  }
  while (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t') {
    end++;
  }
  if (*end != '\0') {
    usage();
    return 1;
  }

  const char *shown = nullptr;
  char shown_buf[16];
  if (fabsf(x) < 0.0001f) {
    shown = "0";
  } else if (fabsf(x - 1.57f) < 0.001f) {
    shown = "1.57";
  } else if (fabsf(x - 3.14f) < 0.001f) {
    shown = "3.14";
  } else {
    snprintf(shown_buf, sizeof(shown_buf), "%.4f", x);
    shown = shown_buf;
  }

  tflite::InitializeTarget();
  const tflite::Model *model = tflite::GetModel(g_hello_world_int8_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf("hello_world: model error\n");
    return 1;
  }

  tflite::MicroMutableOpResolver<1> op_resolver;
  if (op_resolver.AddFullyConnected() != kTfLiteOk) {
    printf("hello_world: model error\n");
    return 1;
  }

  tflite::MicroInterpreter interpreter(model, op_resolver, g_tensor_arena,
                                       kTensorArenaSize);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("hello_world: model error\n");
    return 1;
  }

  TfLiteTensor *input = interpreter.input(0);
  TfLiteTensor *output = interpreter.output(0);
  float scaled = x / input->params.scale + input->params.zero_point;
  int q = static_cast<int>(roundf(scaled));
  if (q > 127) {
    q = 127;
  }
  if (q < -128) {
    q = -128;
  }
  input->data.int8[0] = static_cast<int8_t>(q);

  if (interpreter.Invoke() != kTfLiteOk) {
    printf("hello_world: model error\n");
    return 1;
  }

  float y = (output->data.int8[0] - output->params.zero_point) *
            output->params.scale;
  printf("hello_world: x=%s y=%.4f\n", shown, y);
  return 0;
}
