#pragma once

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

#include <pthread.h>
#include <cmath>

#include "protocols/cursor-shape-v1.hpp"
#include "protocols/fractional-scale-v1.hpp"
#include "protocols/wlr-layer-shell-unstable-v1.hpp"
#include "protocols/wlr-screencopy-unstable-v1.hpp"
#include "protocols/viewporter.hpp"
#include "protocols/wayland.hpp"

#include <cassert>
#include <cairo.h>
#include <cairo/cairo.h>
#include <fcntl.h>
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <algorithm>
#include <filesystem>
#include <thread>
#include <unordered_map>

#include <hyprutils/memory/WeakPtr.hpp>
#include <hyprutils/os/Process.hpp>
using namespace Hyprutils::Memory;
using namespace Hyprutils::OS;

#define SP CSharedPointer
#define WP CWeakPointer
