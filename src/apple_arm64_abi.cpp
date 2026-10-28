#include "apple_arm64_abi.h"

#if defined(__APPLE__) && defined(__aarch64__)

#include <cstdint>
#include <log.h>

namespace {

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef unsigned int GLbitfield;
typedef int GLint;
typedef int GLsizei;
// A 32-bit argument the game passed on the stack: an 8-byte slot of which only the low half is defined.
typedef uint64_t slot;

template <int Alias>
struct CopyImageSubData {
    static void (*host)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
    static void call(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, slot dstLevel, slot dstX, slot dstY, slot dstZ, slot width, slot height, slot depth) {
        host(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, (GLint)dstLevel, (GLint)dstX, (GLint)dstY, (GLint)dstZ, (GLsizei)width, (GLsizei)height, (GLsizei)depth);
    }
};
template <int Alias>
void (*CopyImageSubData<Alias>::host)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = nullptr;

template <int Alias>
struct BlitFramebuffer {
    static void (*host)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
    static void call(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, slot mask, slot filter) {
        host(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, (GLbitfield)mask, (GLenum)filter);
    }
};
template <int Alias>
void (*BlitFramebuffer<Alias>::host)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) = nullptr;

template <int Alias>
struct TexSubImage3D {
    static void (*host)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
    static void call(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, slot format, slot type, const void *pixels) {
        host(target, level, xoffset, yoffset, zoffset, width, height, depth, (GLenum)format, (GLenum)type, pixels);
    }
};
template <int Alias>
void (*TexSubImage3D<Alias>::host)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *) = nullptr;

template <int Alias>
struct CompressedTexSubImage3D {
    static void (*host)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *);
    static void call(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, slot format, slot imageSize, const void *data) {
        host(target, level, xoffset, yoffset, zoffset, width, height, depth, (GLenum)format, (GLsizei)imageSize, data);
    }
};
template <int Alias>
void (*CompressedTexSubImage3D<Alias>::host)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *) = nullptr;

template <typename Adapter>
void adapt(std::unordered_map<std::string, void *> &overrides, void *(*resolver)(const char *), const char *name) {
    void *fn = resolver(name);
    if(!fn)
        return;
    Adapter::host = (decltype(Adapter::host))fn;
    overrides[name] = (void *)&Adapter::call;
}

}  // namespace

void AppleArm64Abi::installGL(std::unordered_map<std::string, void *> &overrides, void *(*resolver)(const char *)) {
    // Every GL ES 3.x / extension entry point the game imports with more than 8 integer parameters followed by
    // further stack parameters (glTexImage3D and the 9-parameter functions end with a pointer and are laid out
    // identically by both ABIs).
    adapt<CopyImageSubData<0>>(overrides, resolver, "glCopyImageSubData");
    adapt<CopyImageSubData<1>>(overrides, resolver, "glCopyImageSubDataEXT");
    adapt<CopyImageSubData<2>>(overrides, resolver, "glCopyImageSubDataOES");
    adapt<BlitFramebuffer<0>>(overrides, resolver, "glBlitFramebuffer");
    adapt<BlitFramebuffer<1>>(overrides, resolver, "glBlitFramebufferANGLE");
    adapt<BlitFramebuffer<2>>(overrides, resolver, "glBlitFramebufferNV");
    adapt<TexSubImage3D<0>>(overrides, resolver, "glTexSubImage3D");
    adapt<TexSubImage3D<1>>(overrides, resolver, "glTexSubImage3DOES");
    adapt<CompressedTexSubImage3D<0>>(overrides, resolver, "glCompressedTexSubImage3D");
    adapt<CompressedTexSubImage3D<1>>(overrides, resolver, "glCompressedTexSubImage3DOES");
    Log::info("AppleArm64Abi", "Installed stack-argument adapters for GL functions with more than 8 parameters");
}

#else

void AppleArm64Abi::installGL(std::unordered_map<std::string, void *> &, void *(*)(const char *)) {
}

#endif
