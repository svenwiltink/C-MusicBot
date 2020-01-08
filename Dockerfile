FROM gcc

RUN apt-get -y update && apt-get install -y --no-install-recommends apt-utils cmake
RUN apt-get install -y libcurl4-openssl-dev libcjson-dev

RUN mkdir -p /usr/src/musicbot/build/bin

COPY . /usr/src/musicbot

WORKDIR /usr/src/musicbot/build

ENTRYPOINT ["/bin/bash", "-c", "cmake .. && cmake --build ."]