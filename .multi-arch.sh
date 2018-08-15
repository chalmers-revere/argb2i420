#!/bin/sh

VERSION=$1

cat <<EOF >/tmp/multi.yml
image: chalmersrevere/argb2i420-multi:$VERSION
manifests:	
  - image: chalmersrevere/argb2i420-amd64:$VERSION
    platform:
      architecture: amd64
      os: linux
  - image: chalmersrevere/argb2i420-armhf:$VERSION
    platform:
      architecture: arm
      os: linux
  - image: chalmersrevere/argb2i420-aarch64:$VERSION
    platform:
      architecture: arm64
      os: linux
EOF
manifest-tool-linux-amd64 push from-spec /tmp/multi.yml
