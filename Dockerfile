# Ubuntu 20.04
# FROM ubuntu:focal

# Ubuntu 22.04
FROM ubuntu:jammy

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_ID

RUN adduser --disabled-password --gecos '' --uid $USER_ID user

RUN apt-get update
RUN apt-get install -y gcc g++ meson libyaml-0-2 libglib2.0-dev libbsd-dev

USER user

