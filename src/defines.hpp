#pragma once

#include "debug/Log.hpp"
#include "helpers/Vector2D.hpp"
#include "includes.hpp"
#include "helpers/Monitor.hpp"
#include "helpers/Color.hpp"
#include "clipboard/Clipboard.hpp"

// git stuff
#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "?"
#endif
#ifndef GIT_BRANCH
#define GIT_BRANCH "?"
#endif
#ifndef GIT_COMMIT_MESSAGE
#define GIT_COMMIT_MESSAGE "?"
#endif
#ifndef GIT_DIRTY
#define GIT_DIRTY "?"
#endif