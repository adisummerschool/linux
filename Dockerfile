FROM ubuntu:latest

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    gcc-arm-linux-gnueabi \
    bc \
    flex \
    bison \
    libssl-dev \
    libelf-dev \
    u-boot-tools \
    device-tree-compiler \
    git && \
    rm -rf /var/lib/apt/lists/*