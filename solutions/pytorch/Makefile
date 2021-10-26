TOP = $(abspath ../..)
include $(TOP)/defs.mak

APPBUILDER=$(TOP)/scripts/appbuilder

DOCKER_REGISTRY           = docker.io
DOCKER_ORG                = mystikos
DOCKER_IMAGE              = pytorch-test
DOCKER_FULL_NAME          = $(DOCKER_REGISTRY)/$(DOCKER_ORG)/$(DOCKER_IMAGE)
DOCKER_FILE               = Dockerfile

BASE_IMAGE                = ubuntu:18.04
PYTHON_VERSION            = 3.8
PYTORCH_VERSION           = v1.10.0
BUILD_TYPE                = build
BUILD_PROGRESS            = auto
BUILD_ARGS                = --build-arg BASE_IMAGE=$(BASE_IMAGE) \
							--build-arg PYTHON_VERSION=$(PYTHON_VERSION) \
							--build-arg PYTORCH_VERSION=$(PYTORCH_VERSION)
EXTRA_OPTIONS             = --progress=$(BUILD_PROGRESS) \
							--target $(BUILD_TYPE) \
							-t $(DOCKER_FULL_NAME):$(DOCKER_TAG) \
							$(BUILD_ARGS)

ifdef STRACE
OPTS += --strace
endif

.PHONY: all
all: rootfs

appdir: DOCKER_TAG := $(PYTORCH_VERSION)-nightly
appdir: export DOCKER_BUILDKIT=1
appdir:
	$(APPBUILDER) -e '$(EXTRA_OPTIONS)' -v -d $(DOCKER_FILE)

rootfs: appdir
	$(MYST) mkext2 appdir rootfs

run: rootfs
	$(MYST_EXEC) rootfs $(OPTS) --app-config-path config.json /opt/conda/bin/conda run --no-capture-output -n pytorch-deps python -m pytest /opt/pytorch/test/test_torch.py -v

one: rootfs
	$(MYST_EXEC) rootfs $(OPTS) --app-config-path config.json /opt/conda/bin/conda run --no-capture-output -n pytorch-deps python -m pytest /opt/pytorch/test/test_torch.py -v -k TestTorchDeviceTypeCPU.test_corrcoef_cpu_complex64

.PHONY: clean
clean:
	-rm -rf rootfs appdir
	-docker rmi -f $(shell docker images -q $(DOCKER_FULL_NAME))
