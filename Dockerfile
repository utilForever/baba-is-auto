FROM ubuntu:26.04
LABEL maintainer "Chris Ohk <utilforever@gmail.com>"

RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    ninja-build \
    pkg-config \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-venv \
    tar \
    unzip \
    zip \
    --no-install-recommends \
    && rm -rf /var/lib/apt/lists/*

ENV CMAKE_VENV=/opt/cmake-venv
RUN python3 -m venv ${CMAKE_VENV} && \
    ${CMAKE_VENV}/bin/pip install "cmake>=3.31.6"
ENV PATH="${CMAKE_VENV}/bin:${PATH}"

ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} && \
    ${VCPKG_ROOT}/bootstrap-vcpkg.sh

COPY . /app

WORKDIR /app
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel "$(nproc)" && \
    cmake --install build && \
    build/bin/UnitTests

WORKDIR /
