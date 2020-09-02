FROM gcc:latest as build

RUN apt-get update && \
    apt-get -y install cmake

ADD ./src /app/src

WORKDIR /app/build

RUN cmake ../src && make 

FROM ubuntu:latest

RUN groupadd -r sample && useradd -r -g sample sample
USER sample

COPY --from=build /app/build/poll_server .
COPY --from=build /app/build/client .

ADD run.sh .

ENTRYPOINT [ "./run.sh" ]
