FROM ubuntu:22.04

# Set locale to UTF-8
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install linux dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    g++ cmake make build-essential \
    python3=3.10.* python3-dev=3.10.* python3-pip=22.0.* python-is-python3 \
    libgl1-mesa-dev libxkbcommon-x11-0 \
    libxcb1 libxcb-util1 libxcb-keysyms1 libxcb-image0 libxcb-render0 \
    libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 \
    libxcb-icccm4 libxcb-composite0 libxcb-xfixes0 libxcb-sync1 \
    libxcb-damage0 libxcb-dpms0 libxcb-present0 libxcb-record0 \
    libxcb-res0 libxcb-screensaver0 libxcb-xinput0 libxcb-cursor0 \
    libdbus-1-3 \
    zlib1g-dev libbz2-dev libglib2.0-dev\
    wget curl xz-utils libx11-6 \
    x11-apps x11-utils xserver-xorg-core xserver-xorg-video-dummy \
    x11-xserver-utils xvfb \
    && rm -rf /var/lib/apt/lists/*

# Copy the project source and kedro umbrella into the container
# Assuming correct versions of descartes-builder and kedro-umbrella are present
# in "~/descartes-builder/docker_deps"
RUN mkdir /app && mkdir /app/deps
WORKDIR /app/
COPY descartes-builder /app/deps/descartes-builder
COPY kedro-umbrella /app/deps/kedro-umbrella

# Install kedro-umbrella
RUN pip install --upgrade pip setuptools wheel
RUN --mount=type=cache,target=/root/.cache/pip cd /app/deps/kedro-umbrella && pip install .

# Install Qt using aqt (unofficial helper library: https://github.com/miurahr/aqtinstall)
RUN pip install aqtinstall
RUN aqt install-qt --outputdir /opt/qt linux desktop 6.7.3 linux_gcc_64 -m qt5compat

# Set Qt environment variables
ENV QT_DIR="/opt/qt/6.7.3/gcc_64/"
ENV PATH="${QT_DIR}/bin:$PATH"
ENV LD_LIBRARY_PATH="${QT_DIR}/lib"

# Build the project
WORKDIR /app/build
RUN cmake -S ../deps/descartes-builder/app/ -B . && cmake --build .

# Run the executable
CMD ["/app/build/bin/DescartesBuilder"]

# Start the container with the following to use the host display (only for linux)
# docker run -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix desc-build:v1


