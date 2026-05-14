ARG UBUNTU_VERSION=latest


FROM ubuntu:${UBUNTU_VERSION}

RUN apt-get update && apt-get install -y --no-install-recommends \
build-essential \
pkg-config \
cmake \
ninja-build \
python3 \
python3-pip \
libsdl2-dev \
libavutil-dev \
libavcodec-dev \
libavformat-dev \
libswscale-dev \
python3-dev \
libgtk-3-dev \
libglib2.0-dev \
libgtk2.0-dev \
libgl1-mesa-dev \
libssl-dev \
libopencv-dev \
&& rm -rf /var/lib/apt/lists/*

WORKDIR /app
