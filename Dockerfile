# Use an official Ubuntu image with CMake and build tools
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential \
       cmake \
       libwebsocketpp-dev \
       nlohmann-json3-dev \
       libasio-dev \
       libboost-system-dev \
       libboost-thread-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . /app

# Clean up any existing Windows build artifacts and create build directory
RUN rm -rf build && mkdir -p build

# Create build directory and compile the project
RUN cmake -S . -B build \
    && cmake --build build -- -j$(nproc)

# Default command
CMD ["/app/build/battleship_server"]
