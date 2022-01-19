TOP = $(abspath ../..)
include $(TOP)/defs.mak

APPBUILDER=$(TOP)/scripts/appbuilder

ifdef STRACE
OPTS = --strace
endif

GIT_COMMIT ?= v1.9.0

all: rootfs

tensorflow.tar.gz:
	wget https://github.com/tensorflow/tensorflow/archive/refs/tags/v1.9.0.tar.gz -O tensorflow.tar.gz

inception_v3_2018_04_27.tgz:
	curl https://storage.googleapis.com/download.tensorflow.org/models/tflite/model_zoo/upload_20180427/inception_v3_2018_04_27.tgz -o inception_v3_2018_04_27.tgz

appdir: tensorflow.tar.gz inception_v3_2018_04_27.tgz
	$(APPBUILDER) Dockerfile

rootfs: appdir
	$(MYST) mkext2 appdir rootfs

run: rootfs
	$(MYST_EXEC) rootfs $(OPTS) --app-config-path config.json /app/label_image -m /app/inception_v3.tflite -i /app/image.bmp -t 4

clean:
	rm -rf rootfs appdir

distclean: clean
	rm -rf tensorflow.tar.gz inception_v3_2018_04_27.tgz
