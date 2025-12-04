FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    libopencv-dev \
    libtesseract-dev \
    libleptonica-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/Kitware/CMake/releases/download/v3.27.7/cmake-3.27.7-linux-x86_64.sh \
    && chmod +x cmake-3.27.7-linux-x86_64.sh \
    && ./cmake-3.27.7-linux-x86_64.sh --prefix=/usr/local --skip-license \
    && rm cmake-3.27.7-linux-x86_64.sh

WORKDIR /app

COPY CMakeLists.txt .
COPY protos/ocr.proto .
COPY Server/ Server/
COPY tessdata/ tessdata/

RUN mkdir Client && echo "cmake_minimum_required(VERSION 3.24)\nproject(ClientDummy)" > Client/CMakeLists.txt

RUN mkdir build && cd build && \
    cmake .. && \
    make -j4

ENV TESSDATA_PREFIX=/app/Server/tessdata

EXPOSE 50051

CMD ["./build/Server/OCRServer"]