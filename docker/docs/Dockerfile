ARG BUSYBOX_VERSION=latest

FROM busybox:${BUSYBOX_VERSION}

RUN adduser -D static
USER static
WORKDIR /

COPY ./build/web/public/ .

CMD ["busybox", "httpd", "-f", "-v", "-p", "3000"]
