FROM mcr.microsoft.com/vscode/devcontainers/base:0-ubuntu

RUN apt-get update && \
    env DEBIAN_FRONTEND=noninteractive \
    apt-get install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
