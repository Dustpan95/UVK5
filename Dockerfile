FROM ubuntu@sha256:0a9887931b0e38af2e3487ed39e29436b0535c4f3f52e57745baaf79d1898956
COPY tools/install-container.sh /tmp/install-container.sh
RUN bash /tmp/install-container.sh && rm /tmp/install-container.sh
WORKDIR /src
