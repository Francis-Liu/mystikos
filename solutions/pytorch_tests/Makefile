TOP = $(abspath ../..)
include $(TOP)/defs.mak

APPBUILDER=$(TOP)/scripts/appbuilder

DOCKER_REGISTRY           = docker.io
DOCKER_ORG                = mystikos
DOCKER_IMAGE              = pytorch-test
DOCKER_FULL_NAME          = $(DOCKER_REGISTRY)/$(DOCKER_ORG)/$(DOCKER_IMAGE)
DOCKER_FILE               = Dockerfile
DOCKER_TAG                = $(PYTORCH_VERSION)

BASE_IMAGE                = python:3.9-buster
PYTORCH_VERSION           = v1.10.0
BUILD_PROGRESS            = auto
BUILD_TARGET              = test
BUILD_ARGS                = --build-arg BASE_IMAGE=$(BASE_IMAGE) \
							--build-arg PYTORCH_VERSION=$(PYTORCH_VERSION)
EXTRA_OPTIONS             = --progress=$(BUILD_PROGRESS) \
							--target $(BUILD_TARGET) \
							-t $(DOCKER_FULL_NAME):$(DOCKER_TAG) \
							$(BUILD_ARGS)

DOCKER_BUILD              = DOCKER_BUILDKIT=1 \
							docker build \
							-f $(DOCKER_FILE) \
							$(EXTRA_ARGS) .
DOCKER_PUSH               = docker push $(DOCKER_FULL_NAME):$(DOCKER_TAG)
DOCKER_RUN                = docker run --rm -it $(DOCKER_FULL_NAME):$(DOCKER_TAG)

ifdef STRACE
OPTS += --strace
endif

.PHONY: all
all: rootfs

.PHONY: image
image:
	$(DOCKER_BUILD)

.PHONY: push
push:
	$(DOCKER_PUSH)

appdir:
	$(APPBUILDER) -i $(DOCKER_FULL_NAME):$(DOCKER_TAG) -v

rootfs: appdir
	$(MYST) mkext2 appdir rootfs

run: rootfs
	# $(MYST_EXEC) rootfs $(OPTS) --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_modules.py -v
	$(MYST_EXEC) rootfs $(OPTS) --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_nn.py -v -k "not test_EmbeddingBag_empty_per_sample_weights_and_offsets_cpu_int32_int32_float32 and not test_EmbeddingBag_empty_per_sample_weights_and_offsets_cpu_int32_int64_float32 and not test_EmbeddingBag_empty_per_sample_weights_and_offsets_cpu_int64_int32_float32 and not test_EmbeddingBag_empty_per_sample_weights_and_offsets_cpu_int64_int64_float32 and not test_EmbeddingBag_per_sample_weights_and_new_offsets_cpu_int32_int32_float32 and not test_EmbeddingBag_per_sample_weights_and_new_offsets_cpu_int32_int64_float32 and not test_EmbeddingBag_per_sample_weights_and_new_offsets_cpu_int64_int32_float32 and not test_EmbeddingBag_per_sample_weights_and_new_offsets_cpu_int64_int64_float32 and not test_EmbeddingBag_per_sample_weights_and_no_offsets_cpu_int64_float32 and not test_EmbeddingBag_per_sample_weights_and_no_offsets_cpu_int32_float32 and not test_EmbeddingBag_per_sample_weights_and_offsets_cpu_int32_int32_float32 and not test_EmbeddingBag_per_sample_weights_and_offsets_cpu_int32_int32_float32 and not test_EmbeddingBag_per_sample_weights_and_offsets_cpu_int32_int64_float32 and not test_embedding_bag_1D_padding_idx_cpu_float32 and not test_EmbeddingBag_per_sample_weights_and_offsets_cpu_int64_int32_float32 and not test_embedding_bag_device_cpu_int32_int32_float32 and not test_EmbeddingBag_per_sample_weights_and_offsets_cpu_int64_int64_float32 and not test_embedding_bag_device_cpu_int32_int64_float32 and not test_embedding_bag_device_cpu_int64_int32_float32 and not test_embedding_bag_device_cpu_int64_int64_float32 and not test_embedding_bag_non_contiguous_weight_cpu_int32_int32_float32 and not test_embedding_bag_non_contiguous_weight_cpu_int32_int64_float32 and not test_embedding_bag_non_contiguous_weight_cpu_int64_int32_float32 and not test_embedding_bag_non_contiguous_weight_cpu_int64_int64_float32 and not test_EmbeddingBag_discontiguous and not test_EmbeddingBag_mean and not test_EmbeddingBag_sparse and not test_EmbeddingBag_sum"

# $(MYST_EXEC) rootfs $(OPTS) --strace --etrace --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_autograd.py -v -s -k "test_record_function and not test_record_function_callbacks and not test_record_function_multithreaded" 2>&1 | tee test_record_function.txt
one: rootfs
	$(MYST_EXEC) rootfs --strace --etrace --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_autograd.py -v -s -k "test_no_grad_copy_sparse" 2>&1 | tee test_no_grad_copy_sparse.txt

gdb: rootfs
	$(MYST_GDB) --args $(MYST_EXEC) --strace --etrace --max-affinity-cpus=1 rootfs --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_autograd.py -v -s -k "test_no_grad_copy_sparse"

local-appdir: DOCKER_FILE := $(DOCKER_FILE).local
local-appdir: DOCKER_TAG := $(PYTORCH_VERSION)-local
local-appdir: export DOCKER_BUILDKIT=1
local-appdir:
	$(APPBUILDER) -e '$(EXTRA_OPTIONS)' -v -d $(DOCKER_FILE) -o local-appdir

local-rootfs: local-appdir
	$(MYST) mkext2 local-appdir local-rootfs

local-one: local-rootfs
	$(MYST_EXEC) local-rootfs $(OPTS) --strace --etrace --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_autograd.py -v -s -k "test_no_grad_copy_sparse"

local-gdb: local-rootfs
	$(MYST_GDB) --args $(MYST_EXEC) --strace --etrace --max-affinity-cpus=1 local-rootfs --app-config-path config.json /usr/local/bin/python -m pytest /workspace/pytorch/test/test_autograd.py -v -s -k "test_no_grad_copy_sparse"

.PHONY: docker-run
docker-run:
	$(DOCKER_RUN) /usr/local/bin/pytest /workspace/pytorch/test/test_autograd.py -v

.PHONY: clean
clean:
	-rm -rf rootfs appdir local-rootfs local-appdir
