#!/bin/bash

# Check if Docker is running
docker ps > /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "Failed to execute 'docker ps'. Ensure Docker is up and running."
  exit 1
fi

# Check if os_build_custom container exists, create or start it
docker container inspect os_build_custom > /dev/null 2>&1
if [ $? -ne 0 ]; then
  set -e
  echo "Container os_build_custom not found. Creating a new one."
  echo "Mounting current directory '$(pwd)' to /src in the container."
  docker run -i -d --name os_build_custom -v "$(pwd)":/src Romanko2024/os_build_custom:0.1
else
  set -e
  echo "Container os_build_custom exists. Starting it."
  docker start os_build_custom > /dev/null
fi

# Build kernel inside the container
echo "Building kernel.bin inside os_build_custom container..."
docker exec os_build_custom bash -l -c "cd /src && make"

# Run kernel using QEMU
echo "Running kernel.bin with QEMU..."
qemu-system-i386 -kernel build/kernel.bin
