#pragma once

#include <string>
#include <unordered_map>

// Android (AAPCS64) passes every stack argument in its own 8-byte slot, while Apple's arm64 ABI packs stack
// arguments to their natural size. A GL entry point with more than 8 integer parameters therefore reads garbage
// for its stack-passed parameters when the game calls the host library directly (e.g. glCopyImageSubData, which
// the game uses to blit screenshots and world thumbnails, silently failed with GL_INVALID_OPERATION).
// These adapters accept the game's layout and call the host function with the proper prototype.
class AppleArm64Abi {
public:
    static void installGL(std::unordered_map<std::string, void *> &overrides, void *(*resolver)(const char *));
};
