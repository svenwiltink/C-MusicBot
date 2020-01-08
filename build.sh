#!/bin/bash

mkdir bin
docker build . -t musicbot-builder
docker run -v $(pwd)/bin:/usr/src/musicbot/build/bin musicbot-builder