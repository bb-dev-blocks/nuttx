#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/examples/person_detection/model_settings.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "tensorflow/lite/micro/models/person_detect_model_data.cc"

extern "C" int ajit1_tflite_read(const char *path, void *buf, size_t need);

namespace {

constexpr int kArenaSize = 160 * 1024;
alignas(16) uint8_t g_arena[kArenaSize];

void usage(void) {
  printf("usage: /tflite/person_detection <file>\n");
  printf("file: 9216 raw bytes, 96x96\n");
  printf("example: /tflite/person_detection /tflite/inputs/person_detection/person\n");
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

extern "C" int person_detection_main(int argc, char *argv[]) {
  if (help_arg(argc, argv)) {
    usage();
    return 1;
  }

  tflite::InitializeTarget();
  const tflite::Model *model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf("person_detection: model error\n");
    return 1;
  }

  tflite::MicroMutableOpResolver<5> ops;
  if (ops.AddAveragePool2D(tflite::Register_AVERAGE_POOL_2D_INT8()) !=
          kTfLiteOk ||
      ops.AddConv2D(tflite::Register_CONV_2D_INT8()) != kTfLiteOk ||
      ops.AddDepthwiseConv2D(tflite::Register_DEPTHWISE_CONV_2D_INT8()) !=
          kTfLiteOk ||
      ops.AddReshape() != kTfLiteOk ||
      ops.AddSoftmax(tflite::Register_SOFTMAX_INT8()) != kTfLiteOk) {
    printf("person_detection: model error\n");
    return 1;
  }

  tflite::MicroInterpreter interpreter(model, ops, g_arena, kArenaSize);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("person_detection: model error\n");
    return 1;
  }

  TfLiteTensor *input = interpreter.input(0);
  TfLiteTensor *output = interpreter.output(0);
  if (input == nullptr || output == nullptr || input->bytes == 0) {
    printf("person_detection: model error\n");
    return 1;
  }
  if (ajit1_tflite_read(argv[1], input->data.int8, input->bytes) != 0) {
    usage();
    return 1;
  }
  if (interpreter.Invoke() != kTfLiteOk) {
    printf("person_detection: model error\n");
    return 1;
  }

  int8_t person_score = output->data.int8[kPersonIndex];
  int8_t no_person_score = output->data.int8[kNotAPersonIndex];
  if (person_score > no_person_score) {
    printf("person_detection: person\n");
  } else {
    printf("person_detection: no person\n");
  }
  return 0;
}
