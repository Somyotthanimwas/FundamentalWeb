FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        g++ \
        libboost-dev \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY backend/main.cpp /app/main.cpp
COPY data/fundamental_v4.csv /app/data/fundamental_v4.csv

RUN g++ -std=c++20 -O2 \
    /app/main.cpp \
    -o /app/fundamental_backend \
    -lpthread

EXPOSE 10000

CMD ["/app/fundamental_backend"]
