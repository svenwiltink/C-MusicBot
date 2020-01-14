FROM gcc

RUN apt-get -y update && apt-get install -y --no-install-recommends apt-utils cmake
RUN apt-get install -y libcurl4-openssl-dev libjson-c-dev libwebsockets-dev

RUN mkdir -p /usr/src/musicbot/build/bin

COPY . /usr/src/musicbot

WORKDIR /usr/src/musicbot/build

ENTRYPOINT ["/bin/bash", "-c", "cmake .. -DCMAKE_BUILD_TYPE=DEBUG && cmake --build ."]