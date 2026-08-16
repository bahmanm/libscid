# Build with:
#   docker build -t libscid-qc:ubuntu-24.04 -f etc/cmake/qc/qc.Dockerfile .
#   docker build --platform linux/amd64 -t libscid-qc:ubuntu-24.04-amd64 -f etc/cmake/qc/qc.Dockerfile .
#   docker build --platform linux/arm64 -t libscid-qc:ubuntu-24.04-arm64 -f etc/cmake/qc/qc.Dockerfile .

FROM ubuntu:24.04

LABEL org.opencontainers.image.authors="Bahman Movaqar <Bahman@BahmanM.com>"
LABEL description="A consistent setup to run the QC checks."

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ARG MAKE_VERSION=4.4.1
ARG MAKE_SHA256=dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3
ARG BMAKELIB_VERSION=0.8.0
ARG BMAKELIB_SHA256=eb2bedeee0c6e677d392a1ca1ddaacf7cc58e2b75bff323c91b34fc5fe3053f3
ARG CURL_FLAGS=
ARG PIP_FLAGS=

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/usr/local/bin:${PATH}"
ENV LIBSCID_BMAKELIB=/usr/local/include/bmakelib/bmakelib.mk
ENV LIBSCID_C_COMPILER=clang-20
ENV LIBSCID_CXX_COMPILER=clang++-20

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
           build-essential \
           ca-certificates \
           clang-20 \
           clang-format-20 \
           clang-tidy-20 \
           clang-tools-20 \
           cmake \
           cppcheck \
           curl \
           python3 \
           python3-pip \
    && build_dir="$(mktemp -d)" \
    && curl -fsSL ${CURL_FLAGS} "https://ftp.gnu.org/gnu/make/make-${MAKE_VERSION}.tar.gz" \
           -o "${build_dir}/make.tar.gz" \
    && echo "${MAKE_SHA256}  ${build_dir}/make.tar.gz" | sha256sum -c - \
    && tar -xzf "${build_dir}/make.tar.gz" -C "${build_dir}" \
    && make_jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)" \
    && cd "${build_dir}/make-${MAKE_VERSION}" \
    && ./configure --prefix=/usr/local \
    && make -j"${make_jobs}" \
    && make install \
    && hash -r \
    && make --version | head -n 1 | grep -F "GNU Make ${MAKE_VERSION}" \
    && cd / \
    && rm -rf "${build_dir}" \
    && build_dir="$(mktemp -d)" \
    && curl -fsSL \
           ${CURL_FLAGS} \
           "https://github.com/bahmanm/bmakelib/releases/download/v${BMAKELIB_VERSION}/bmakelib-${BMAKELIB_VERSION}.tar.gz" \
           -o "${build_dir}/bmakelib.tar.gz" \
    && echo "${BMAKELIB_SHA256}  ${build_dir}/bmakelib.tar.gz" | sha256sum -c - \
    && tar -xzf "${build_dir}/bmakelib.tar.gz" -C "${build_dir}" \
    && mkdir -p /usr/local/include/bmakelib \
    && cp "${build_dir}/bmakelib-${BMAKELIB_VERSION}/src/"*.mk \
          "${build_dir}/bmakelib-${BMAKELIB_VERSION}/src/VERSION" \
          /usr/local/include/bmakelib/ \
    && test -f "${LIBSCID_BMAKELIB}" \
    && rm -rf "${build_dir}" \
    && python3 -m pip install --break-system-packages ${PIP_FLAGS} uv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /libscid
