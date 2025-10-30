FROM ubuntu:20.04
RUN apt-get update && apt-get install -y build-essential
WORKDIR /test
COPY . . 
RUN gcc -o shell shell.c
CMD ["./shell"]
