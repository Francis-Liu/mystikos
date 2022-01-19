# FROM python:3.9.7-buster
FROM python:3.6-slim-buster

RUN apt-get update && \
    apt-get install -y python-dev python-pip wget git pkg-config zip g++ zlib1g-dev unzip

WORKDIR /app
RUN wget https://github.com/bazelbuild/bazel/releases/download/0.16.1/bazel-0.16.1-installer-linux-x86_64.sh
RUN chmod +x bazel-0.16.1-installer-linux-x86_64.sh
RUN ./bazel-0.16.1-installer-linux-x86_64.sh --user

ADD label_image.manifest.template .
ADD tensorflow.tar.gz .
RUN mv tensorflow-1.9.0 tensorflow

WORKDIR /app/tensorflow
RUN /root/bin/bazel build tensorflow/contrib/lite/examples/label_image

WORKDIR /app
RUN cp tensorflow/bazel-bin/tensorflow/contrib/lite/examples/label_image/label_image .
RUN cp tensorflow/bazel-bin/tensorflow/libtensorflow_framework.so .

ADD inception_v3_2018_04_27.tgz .
RUN cp tensorflow/tensorflow/contrib/lite/java/ovic/src/testdata/labels.txt .
RUN cp tensorflow/tensorflow/contrib/lite/examples/label_image/testdata/grace_hopper.bmp image.bmp

CMD ["/app/label_image", "-m", "/app/inception_v3.tflite", "-i", "/app/image.bmp", "-t", "4"]
