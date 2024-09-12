alias s := setup
alias b := build
alias c := clean
alias r := run

_default:
    just --list --no-aliases

# Clean the build files
clean:
    rm -rf build/

# Run the simulator
run *args:
    ./build/obc-simulator {{args}}

# Debug the simulator
gdb:
    gdb ./build/obc-simulator

# Setup meson 
setup:
    meson setup build/

# Compile the project
build:
    ninja -C build/

# Build the docker container
docker-container:
    docker build --tag obc --build-arg USER_ID=$(id -u) .

# Setup meson using docker
docker-setup:
    docker run -v ./:/home obc sh -c "cd home/ && meson setup build/"

# Compile the project using docker
docker-build:
    docker run -v ./:/home obc sh -c "cd home/ && meson compile -C build/"

# Run csh
csh version='1':
    csh -i csh/udp_{{version}}.csh

