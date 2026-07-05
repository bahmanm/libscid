# Build with: docker build --platform linux/amd64 -t libscid-qc:ubuntu-24.04 -f cmake/qc/qc.Dockerfile .

FROM ubuntu:24.04

LABEL org.opencontainers.image.authors="Bahman Movaqar <Bahman@BahmanM.com>"
LABEL description="A consistent setup to run the QC checks."

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install --yes \
           clang-20 \
           clang-format-20 \
           clang-tidy-20 \
           clang-tools-20 \
           cmake \
           cppcheck \
           g++ \
           make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /libscid
