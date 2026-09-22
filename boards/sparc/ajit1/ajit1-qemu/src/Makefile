############################################################################
# boards/sparc/ajit1/ajit1-qemu/src/Makefile
#
# SPDX-License-Identifier: Apache-2.0
############################################################################

-include $(TOPDIR)/Make.defs

ASRCS =
CSRCS = ajit1_boot.c

ifeq ($(CONFIG_AJIT1_QEMU_TFLITE),y)
CSRCS += ajit1_tflite_dir.c
endif

include $(TOPDIR)/boards/Board.mk

ifeq ($(CONFIG_AJIT1_QEMU_TFLITE),y)
TFLITE_ROOT := $(TOPDIR)/../tflite-micro
TFLITE_CXXFLAGS := \
  -m32 -mcpu=v8 -std=c++17 \
  -fno-rtti -fno-exceptions -fno-threadsafe-statics \
  -fno-unwind-tables -fno-asynchronous-unwind-tables \
  -ffunction-sections -fdata-sections \
  -funsigned-char -fno-delete-null-pointer-checks -fomit-frame-pointer \
  -fno-use-cxa-atexit -fpermissive \
  -DTF_LITE_STATIC_MEMORY -DTF_LITE_MCU_DEBUG_LOG \
  -DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS \
  -DTF_LITE_USE_GLOBAL_MIN -DTF_LITE_USE_GLOBAL_MAX \
  -DTF_LITE_DISABLE_X86_NEON \
  -I$(TFLITE_ROOT) \
  -I$(TFLITE_ROOT)/gen/ajit_sparc_default_gcc/genfiles \
  -I$(TFLITE_ROOT)/tensorflow/lite/micro/tools/make/downloads \
  -I$(TFLITE_ROOT)/tensorflow/lite/micro/tools/make/downloads/gemmlowp \
  -I$(TFLITE_ROOT)/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include \
  -I$(TFLITE_ROOT)/tensorflow/lite/micro/tools/make/downloads/kissfft \
  -I$(TFLITE_ROOT)/tensorflow/lite/micro/tools/make/downloads/ruy

TFLITE_OBJS = hello_world.o micro_speech.o person_detection.o resnet50.o cxxstub.o
RESNET_SRC := $(TOPDIR)/../os/rtos/cortos2/examples/tflite/resnet50/resnet50_int8.tflite
RESNET_GEN_DIR := $(TOPDIR)/boards/sparc/ajit1/ajit1-qemu/src/resnet_gen
RESNET_OBJS := $(RESNET_GEN_DIR)/labels.o

TFLITE_INPUT_DIR := $(TOPDIR)/../tflite-inputs
TFLITE_INPUTS := $(shell find $(TFLITE_INPUT_DIR) 2>/dev/null)

$(RESNET_GEN_DIR)/stamp: $(RESNET_SRC) \
    $(TOPDIR)/../scripts/gen-nuttx-resnet50.py
	python3 $(TOPDIR)/../scripts/gen-nuttx-resnet50.py --out $(RESNET_GEN_DIR)
	touch $@

$(RESNET_GEN_DIR)/libmodelblob.a: $(RESNET_GEN_DIR)/stamp
	test -f $@

$(RESNET_GEN_DIR)/libromfs.a: $(TOPDIR)/../scripts/gen-nuttx-tflite-fs.py \
    $(TFLITE_INPUTS)
	python3 $(TOPDIR)/../scripts/gen-nuttx-tflite-fs.py --out $(RESNET_GEN_DIR) \
	  --inputs $(TFLITE_INPUT_DIR)

$(TFLITE_OBJS): %.o: %.cxx $(RESNET_GEN_DIR)/stamp
	$(CXX) -c $(TFLITE_CXXFLAGS) -I$(RESNET_GEN_DIR) -o $@ $<

$(RESNET_GEN_DIR)/labels.o: $(RESNET_GEN_DIR)/stamp
	$(CXX) -c $(TFLITE_CXXFLAGS) -I$(RESNET_GEN_DIR) -o $@ $(RESNET_GEN_DIR)/labels.cc

# Board.mk already archives CSRCS. Append these objects in the same recipe.
libboard$(LIBEXT): $(TFLITE_OBJS) $(RESNET_OBJS) \
    $(RESNET_GEN_DIR)/libmodelblob.a $(RESNET_GEN_DIR)/libromfs.a
	$(call ARCHIVE, $@, $(OBJS) $(TFLITE_OBJS) $(RESNET_OBJS))
endif
