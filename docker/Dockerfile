FROM ubuntu:18.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    openjdk-11-jdk-headless \
    unzip \
    wget \
    zip \
    && apt-get autoremove -y \
    && apt-get clean \
    && rm /var/lib/apt/lists/* -r \
    && rm -rf /usr/share/man/*

RUN useradd -ms /bin/bash droid \
    && adduser droid sudo \
    && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers \
    && printf "\nexport ANDROID_HOME=~/android-sdk\n" >> /home/droid/.bashrc \
    && printf '#!/bin/bash\n/home/droid/android-sdk/platform-tools/adb -H host.docker.internal "$@"\n' > /usr/local/bin/adb \
    && chmod +x /usr/local/bin/adb

COPY ./bg.sh /

ENV ANDROID_HOME=/home/droid/android-sdk
ENV URL_CLITOOLS=https://dl.google.com/android/repository/commandlinetools-linux-6200805_latest.zip

USER droid

ENV TMPFILE="/home/droid/temp.zip"

RUN mkdir ${ANDROID_HOME} \
    && wget --quiet ${URL_CLITOOLS} -O ${TMPFILE} \
    && unzip -d ${ANDROID_HOME} ${TMPFILE} \
    && rm ${TMPFILE} \
    && yes | $ANDROID_HOME/tools/bin/sdkmanager --sdk_root=${ANDROID_HOME} --licenses \
    && $ANDROID_HOME/tools/bin/sdkmanager --sdk_root=${ANDROID_HOME} "build-tools;29.0.3" "cmake;3.10.2.4988404" "ndk;21.1.6352462" "patcher;v4" "platform-tools" "platforms;android-29" "tools"
