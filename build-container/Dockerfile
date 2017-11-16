FROM ubuntu:artful

RUN apt-get update \
 && apt-get install -y \
      build-essential \
      libgl1-mesa-dev \
      libgles2-mesa-dev \
      libsdl2-dev \
      libsdl2-ttf-dev \
      libsdl2-mixer-dev \
      libsdl2-image-dev \
      libenet-dev \
      libpng-dev \
      cmake \
      g++-7 \
      git \
      ninja-build \
 && rm -rf /var/lib/apt/lists/*
