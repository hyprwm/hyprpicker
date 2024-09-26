#pragma once

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>

#include <pthread.h>
#include <cmath>
#include <math.h>

#include "protocols/cursor-shape-v1.hpp"
#include "protocols/fractional-scale-v1.hpp"
#include "protocols/wlr-layer-shell-unstable-v1.hpp"
#include "protocols/wlr-screencopy-unstable-v1.hpp"
#include "protocols/viewporter.hpp"
#include "protocols/wayland.hpp"

#include <assert.h>
#include <cairo.h>
#include <cairo/cairo.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <algorithm>
#include <filesystem>
#include <thread>
#include <unordered_map>

#include <hyprutils/memory/WeakPtr.hpp>
using namespace Hyprutils::Memory;

#define SP CSharedPointer
#define WP CWeakPointer
