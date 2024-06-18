FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    valgrind \
    git

# Copy the source code
COPY . /app

# Set the working directory
WORKDIR /app

# Run bench.sh
CMD ["./bench.sh", "50"]