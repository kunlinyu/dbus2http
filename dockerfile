FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

COPY source.list.d/ubuntu.sources /etc/apt/sources.list.d/ubuntu.sources
RUN apt-get update

# install basic tools
RUN apt-get install -y sudo
RUN apt-get install -y curl wget iputils-ping
RUN apt-get install -y build-essential pkg-config
RUN apt-get install -y file gdb
RUN apt-get install -y git cmake repo
RUN apt-get install -y vim tree silversearcher-ag

# install toolchain
RUN apt-get install -y gcc-aarch64-linux-gnu
RUN apt-get install -y g++-aarch64-linux-gnu

# for coverage test report
RUN apt-get install -y lcov

# install python
RUN apt-get install -y python3 python3-pip
RUN rm /usr/lib/python3.12/EXTERNALLY-MANAGED # install globally

## install conan
RUN pip install conan

## for coverage test report
RUN pip install gcovr

## for interface test
RUN pip install pytest pytest-html requests

# copy musl toolchain
RUN wget https://musl.cc/aarch64-linux-musl-cross.tgz
RUN tar xzvf /aarch64-linux-musl-cross.tgz

USER ubuntu
WORKDIR /home/ubuntu/

COPY --chown=ubuntu conan/global.conf global.conf
RUN conan config install global.conf

COPY --chown=ubuntu conan/profile_build.txt profile_build.txt
COPY --chown=ubuntu conan/profile_host_armv8.txt profile_host_armv8.txt
COPY --chown=ubuntu conan/profile_host_x86_64.txt profile_host_x86_64.txt

COPY --chown=ubuntu conandata.yml conandata.yml
COPY --chown=ubuntu conanfile.py conanfile.py

RUN conan install conanfile.py --build=missing --profile:build=profile_build.txt --profile:host=profile_host_x86_64.txt \
    && conan cache clean --source --build --download --temp

COPY --chown=ubuntu conan_download_cache conan_download_cache
RUN conan install conanfile.py --build=missing --profile:build=profile_build.txt --profile:host=profile_host_armv8.txt \
    && conan cache clean --source --build --download --temp

COPY --chown=ubuntu conan_download_cache conan_download_cache
RUN conan install conanfile.py --build=missing --profile:build=profile_build.txt --profile:host=profile_host_armv8.txt \
    --settings:host="build_type=Debug" \
    && conan cache clean --source --build --download --temp

# cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH=/home/ubuntu/conan_out_x86_64/build/Release/generators/
# -DCMAKE_PREFIX_PATH=/home/ubuntu/conan_out_x86_64/build/Release/generators/ ..





