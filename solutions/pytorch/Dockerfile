ARG BASE_IMAGE=ubuntu:18.04
ARG PYTHON_VERSION=3.8
ARG PYTORCH_VERSION=v1.10.0

FROM ${BASE_IMAGE} as dev-base
RUN --mount=type=cache,id=apt-dev,target=/var/cache/apt \
    apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        ccache \
        cmake \
        curl \
        git \
        libjpeg-dev \
        libpng-dev && \
    rm -rf /var/lib/apt/lists/*
RUN /usr/sbin/update-ccache-symlinks
RUN mkdir /opt/ccache && ccache --set-config=cache_dir=/opt/ccache
ENV PATH /opt/conda/bin:$PATH

FROM dev-base as conda
ARG PYTHON_VERSION=3.8
RUN curl -fsSL -v -o ~/miniconda.sh -O  https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh  && \
    chmod +x ~/miniconda.sh && \
    ~/miniconda.sh -b -p /opt/conda && \
    rm ~/miniconda.sh && \
    /opt/conda/bin/conda install -y python=${PYTHON_VERSION} conda-build pyyaml numpy ipython && \
    /opt/conda/bin/pip install expecttest hypothesis typing_extensions pytest && \
    /opt/conda/bin/conda clean -ya

FROM dev-base as submodule-update
ENV PYTORCH_VERSION ${PYTORCH_VERSION}
WORKDIR /opt
RUN git clone -b v1.10.0 https://github.com/pytorch/pytorch.git --single-branch --depth 1
# RUN git submodule update --init --recursive --jobs 0

FROM conda as build-base
WORKDIR /opt/pytorch
COPY --from=conda /opt/conda /opt/conda
COPY --from=submodule-update /opt/pytorch /opt/pytorch

FROM build-base as build
WORKDIR /opt/pytorch
RUN ./tools/nightly.py checkout -b my-nightly-branch
SHELL ["/opt/conda/bin/conda", "run", "-n", "pytorch-deps", "/bin/bash", "-c"]
RUN conda install -y numpy && \
    pip install expecttest
ENTRYPOINT [ "/opt/conda/bin/conda", "run", "--no-capture-output", "-n", "pytorch-deps", "python", "test/test_torch.py"]

# # https://github.com/pytorch/pytorch/blob/master/CONTRIBUTING.md#build-only-what-you-need
# # For example:
# # DEBUG=1 USE_DISTRIBUTED=0 USE_MKLDNN=0 USE_CUDA=0 BUILD_TEST=0 USE_FBGEMM=0 USE_NNPACK=0 USE_QNNPACK=0 USE_XNNPACK=0 python setup.py develop
# RUN --mount=type=cache,target=/opt/ccache \
#     CMAKE_PREFIX_PATH="$(dirname $(which conda))/../" \
#     DEBUG=1 \
#     USE_DISTRIBUTED=0 \
#     USE_MKLDNN=0 \
#     BUILD_CAFFE2=0 \
#     USE_CUDA=0 \
#     BUILD_TEST=0 \
#     USE_FBGEMM=0 \
#     USE_NNPACK=0 \
#     USE_QNNPACK=0 \
#     USE_XNNPACK=0 \
#     python setup.py develop
# 
# FROM conda as conda-installs
# ARG PYTHON_VERSION=3.8
# ARG CUDA_VERSION=11.1
# ARG CUDA_CHANNEL=nvidia
# ARG INSTALL_CHANNEL=pytorch-nightly
# ENV CONDA_OVERRIDE_CUDA=${CUDA_VERSION}
# RUN /opt/conda/bin/conda install -c "${INSTALL_CHANNEL}" -c "${CUDA_CHANNEL}" -y python=${PYTHON_VERSION} pytorch torchvision torchtext "cudatoolkit=${CUDA_VERSION}" && \
#     /opt/conda/bin/conda clean -ya
# RUN /opt/conda/bin/pip install torchelastic
# 
# FROM ${BASE_IMAGE} as official
# ARG PYTORCH_VERSION
# LABEL com.nvidia.volumes.needed="nvidia_driver"
# RUN --mount=type=cache,id=apt-final,target=/var/cache/apt \
#     apt-get update && apt-get install -y --no-install-recommends \
#         ca-certificates \
#         libjpeg-dev \
#         libpng-dev && \
#     rm -rf /var/lib/apt/lists/*
# COPY --from=conda-installs /opt/conda /opt/conda
# ENV PATH /opt/conda/bin:$PATH
# ENV NVIDIA_VISIBLE_DEVICES all
# ENV NVIDIA_DRIVER_CAPABILITIES compute,utility
# ENV LD_LIBRARY_PATH /usr/local/nvidia/lib:/usr/local/nvidia/lib64
# ENV PYTORCH_VERSION ${PYTORCH_VERSION}
# WORKDIR /workspace

# FROM official as dev
# Should override the already installed version from the official-image stage
# COPY --from=build /opt/conda /opt/conda
