# Pinned Linux build and test environment, plus a small image to play in.
#
#   docker build -t dnd_project .            # builds and runs the tests
#   docker compose run --rm dnd              # plays, saving to ./data
#
# The build fails if any test fails, so a successful image is a tested one.

# ---- build: compile and test --------------------------------------------------
FROM ubuntu:24.04 AS build

# git is needed by CMake's FetchContent to download GoogleTest.
RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ cmake make git ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake --preset release \
 && cmake --build --preset release --parallel \
 && ctest --test-dir build/release --output-on-failure

# ---- runtime: just the game and its data --------------------------------------
# Same base as the build stage, so the libstdc++ the binary links against matches.
FROM ubuntu:24.04 AS runtime

WORKDIR /app
COPY --from=build /src/build/release/DND_PROJECT /app/DND_PROJECT
COPY data/SpellBook.txt /app/data/SpellBook.txt

# The game finds data/ beside its executable, so /app/data is used. Mount a
# host folder over it to keep characters between runs. Run as the image's
# built-in uid 1000 user so saves on a Linux host aren't owned by root; the
# folder is world-writable so a different --user still works.
RUN mkdir -p /app/data/characters && chmod -R a+rwX /app/data
USER ubuntu

ENTRYPOINT ["/app/DND_PROJECT"]
