/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"

#include <libyuv.h>
#include <X11/Xlib.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    auto formatCounter{
        commandlineArguments.count("argb") +
        commandlineArguments.count("rgb") +
        commandlineArguments.count("abgr") +
        commandlineArguments.count("bgr")
    };
    if ( (0 == commandlineArguments.count("in")) ||
         (0 == commandlineArguments.count("out")) ||
         (0 == commandlineArguments.count("width")) ||
         (0 == commandlineArguments.count("height")) ||
         (1 != formatCounter)) {
        std::cerr << argv[0] << " waits on a shared memory containing an image in (A)RGB/(A)BGR format to transform it into a corresponding image in I420 format in another shared memory." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --in=<name of shared memory for the (A)RGB/(A)BGR image> --width=<width> --height=<height> --out=<name of shared memory to be created for the I420 image> --argb|--rgb|--abgr|--bgr [--verbose]" << std::endl;
        std::cerr << "         --in:      name of the shared memory area containing the (A)RBG/(A)BGR image" << std::endl;
        std::cerr << "         --out:     name of the shared memory area to be created for the I420 image" << std::endl;
        std::cerr << "         --width:   width of the input image" << std::endl;
        std::cerr << "         --height:  height of the input image" << std::endl;
        std::cerr << "         --argb:    format of the input image (choose exactly one!)" << std::endl;
        std::cerr << "         --rgb:     format of the input image (choose exactly one!)" << std::endl;
        std::cerr << "         --abgr:    format of the input image (choose exactly one!)" << std::endl;
        std::cerr << "         --bgr:     format of the input image (choose exactly one!)" << std::endl;
        std::cerr << "         --verbose: display input image" << std::endl;
        std::cerr << "Example: " << argv[0] << " --in=img.argb --width=640 --height=480 --argb --out=imgout.i420 --verbose" << std::endl;
    }
    else {
        const std::string IN{commandlineArguments["in"]};
        const std::string OUT{commandlineArguments["out"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool ARGB{commandlineArguments.count("argb") != 0};
        const bool RGB{commandlineArguments.count("rgb") != 0};
        const bool ABGR{commandlineArguments.count("abgr") != 0};
        const bool BGR{commandlineArguments.count("bgr") != 0};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        std::unique_ptr<cluon::SharedMemory> sharedMemoryIN(new cluon::SharedMemory{IN});
        if (sharedMemoryIN && sharedMemoryIN->valid()) {
            std::clog << "[argb2i420]: Attached to '" << sharedMemoryIN->name() << "' (" << sharedMemoryIN->size() << " bytes)." << std::endl;

            std::unique_ptr<cluon::SharedMemory> sharedMemoryOUT(new cluon::SharedMemory{OUT, WIDTH * HEIGHT * 3/2});
            if (sharedMemoryOUT && sharedMemoryOUT->valid()) {
                std::clog << "[argb2i420]: Created shared memory " << OUT << " (" << sharedMemoryOUT->size() << " bytes) for an I420 image (width = " << WIDTH << ", height = " << HEIGHT << ")." << std::endl;

                Display *display{nullptr};
                Visual *visual{nullptr};
                Window window{0};
                XImage *ximage{nullptr};

                if (VERBOSE) {
                    display = XOpenDisplay(NULL);
                    visual = DefaultVisual(display, 0);
                    window = XCreateSimpleWindow(display, RootWindow(display, 0), 0, 0, WIDTH, HEIGHT, 1, 0, 0);
                    ximage = XCreateImage(display, visual, 24, ZPixmap, 0, reinterpret_cast<char*>(sharedMemoryIN->data()), WIDTH, HEIGHT, 32, 0);
                    XMapWindow(display, window);
                }

                while (!cluon::TerminateHandler::instance().isTerminated) {
                    sharedMemoryIN->wait();
                    sharedMemoryIN->lock();
                    {
                        sharedMemoryOUT->lock();
                        {
                            if (ARGB) {
                                libyuv::ARGBToI420(reinterpret_cast<uint8_t*>(sharedMemoryIN->data()), WIDTH * 4 /* 4*WIDTH for ARGB*/,
                                                   reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()), WIDTH,
                                                   reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT)), WIDTH/2,
                                                   reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT + ((WIDTH * HEIGHT) >> 2))), WIDTH/2,
                                                   WIDTH, HEIGHT);
                            }
                            else if (RGB) {
                                libyuv::RGB24ToI420(reinterpret_cast<uint8_t*>(sharedMemoryIN->data()), WIDTH * 3 /* 3*WIDTH for RGB24*/,
                                                    reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()), WIDTH,
                                                    reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT)), WIDTH/2,
                                                    reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT + ((WIDTH * HEIGHT) >> 2))), WIDTH/2,
                                                    WIDTH, HEIGHT);
                            }
                            else if (ABGR) {
                                libyuv::ABGRToI420(reinterpret_cast<uint8_t*>(sharedMemoryIN->data()), WIDTH * 4 /* 4*WIDTH for ARGB*/,
                                                   reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()), WIDTH,
                                                   reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT)), WIDTH/2,
                                                   reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT + ((WIDTH * HEIGHT) >> 2))), WIDTH/2,
                                                   WIDTH, HEIGHT);
                            }
                            else if (BGR) {
                                libyuv::RAWToI420(reinterpret_cast<uint8_t*>(sharedMemoryIN->data()), WIDTH * 3 /* 3*WIDTH for RGB24*/,
                                                  reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()), WIDTH,
                                                  reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT)), WIDTH/2,
                                                  reinterpret_cast<uint8_t*>(sharedMemoryOUT->data()+(WIDTH * HEIGHT + ((WIDTH * HEIGHT) >> 2))), WIDTH/2,
                                                  WIDTH, HEIGHT);
                            }
                        }
                        sharedMemoryOUT->unlock();
                        if (VERBOSE) {
                            XPutImage(display, window, DefaultGC(display, 0), ximage, 0, 0, 0, 0, WIDTH, HEIGHT);
                        }
                    }
                    sharedMemoryIN->unlock();

                    // Notify listeners.
                    sharedMemoryOUT->notifyAll();
                }

                if (VERBOSE) {
                    XCloseDisplay(display);
                }
                retCode = 0;
            }
            else {
                std::cerr << "[argb2i420]: Failed to create shared memory for output image." << std::endl;
            }
        }
        else {
            std::cerr << "[argb2i420]: Failed to attach to shared memory '" << IN << "'." << std::endl;
        }
    }
    return retCode;
}

