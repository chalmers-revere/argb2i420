Moved to https://git.opendlv.org.

## Microservice to transform an input image in (A)RGB/(A)BGR format to I420

This repository provides source code to transform an input image in (A)RGB/(A)BGR
format residing in a shared memory to an image in I420 residing in a new shared
memory suitable for subsequent video compression.

[![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)


## Table of Contents
* [Dependencies](#dependencies)
* [Usage](#usage)
* [Build from sources on the example of Ubuntu 16.04 LTS](#build-from-sources-on-the-example-of-ubuntu-1604-lts)
* [License](#license)


## Dependencies
You need a C++14-compliant compiler to compile this project.

The following dependency is part of the source distribution:
* [libcluon](https://github.com/chrberger/libcluon) - [![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)

The following dependency is downloaded and installed during the Docker-ized build:
* [libyuv](https://chromium.googlesource.com/libyuv/libyuv/+/master) - [![License: BSD 3-Clause](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause) - [Google Patent License Conditions](https://chromium.googlesource.com/libyuv/libyuv/+/master/PATENTS)


## Usage
To run this microservice using `docker-compose`, you can simply add the following
section to your `docker-compose.yml`:

```yml
version: '2' # Must be present exactly once at the beginning of the docker-compose.yml file
services:    # Must be present exactly once at the beginning of the docker-compose.yml file
    argb2i420:
        image: chalmersrevere/argb2i420-multi:v0.0.1
        restart: on-failure
        ipc: "host"
        volumes:
        - /tmp:/tmp
        environment:
        - DISPLAY=${DISPLAY}
        command: "--in=img.argb --width=640 --height=480 --argb --out=imgout.i420"
```

As this microservice is connecting to an existing shared memory to read the (A)RGB/(A)BGR
image for transform it into a new shared memory area using SysV IPC, the `docker-compose.yml`
file specifies the use of `ipc:host`. The folder `/tmp` is shared into the Docker
container to provide tokens describing the shared memory areas.
The parameters to the application are:

* `--in`: Name of the shared memory area containing the (A)RBG/(A)BGR image
* `--out`: Name of the shared memory area to be created for the I420 image
* `--width`: Width of the input image
* `--height`: Height of the input image
* `--argb`: Format of the input image (choose exactly one!)
* `--rgb`: Format of the input image (choose exactly one!)
* `--abgr`: Format of the input image (choose exactly one!)
* `--bgr`: Format of the input image (choose exactly one!)
* `--verbose`: Display decoding information and render the image to screen (requires X11; run `xhost +` to allow access to you X11 server)


## Build from sources on the example of Ubuntu 16.04 LTS
To build this software, you need cmake, C++14 or newer, libyuv, libvpx, and make.
Having these preconditions, just run `cmake` and `make` as follows:

```
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make && make test && make install
```


## License

* This project is released under the terms of the GNU GPLv3 License

