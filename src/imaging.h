// Image pipeline: load PNG/JPEG, fit to PS5 slots, BC7 DDS encode/decode, PNG output.
#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include "lib/stb_image.h"
#include "lib/stb_image_resize2.h"
#include "lib/stb_image_write.h"
#include "lib/bc7enc.h"
#include "lib/bc7decomp.h"

namespace img {

struct Image { int w = 0, h = 0; std::vector<uint8_t> px; };  // RGBA8

inline bool decode(const std::vector<uint8_t> &bytes, Image &out) {
    int n; uint8_t *d = stbi_load_from_memory(bytes.data(), (int)bytes.size(), &out.w, &out.h, &n, 4);
    if (!d) return false;
    out.px.assign(d, d + (size_t)out.w * out.h * 4); stbi_image_free(d); return true;
}

inline Image resize(const Image &s, int w, int h) {
    Image o; o.w = w; o.h = h; o.px.resize((size_t)w * h * 4);
    stbir_resize_uint8_srgb(s.px.data(), s.w, s.h, 0, o.px.data(), w, h, 0, STBIR_RGBA);
    return o;
}

inline Image cover(const Image &s, int W, int H) {
    double k = std::max((double)W / s.w, (double)H / s.h);
    int rw = std::max(W, (int)(s.w * k + .5)), rh = std::max(H, (int)(s.h * k + .5));
    Image r = resize(s, rw, rh), o; o.w = W; o.h = H; o.px.resize((size_t)W * H * 4);
    int ox = (rw - W) / 2, oy = (rh - H) / 2;
    for (int y = 0; y < H; y++) memcpy(&o.px[(size_t)y * W * 4], &r.px[((size_t)(y + oy) * rw + ox) * 4], (size_t)W * 4);
    return o;
}

inline Image contain(const Image &s, int W, int H, double margin) {
    double k = std::min(W * (1 - margin) / s.w, H * (1 - margin) / s.h);
    int rw = std::max(1, (int)(s.w * k + .5)), rh = std::max(1, (int)(s.h * k + .5));
    Image r = resize(s, rw, rh), o; o.w = W; o.h = H; o.px.assign((size_t)W * H * 4, 0);
    int ox = (W - rw) / 2, oy = (H - rh) / 2;
    for (int y = 0; y < rh; y++) memcpy(&o.px[((size_t)(y + oy) * W + ox) * 4], &r.px[(size_t)y * rw * 4], (size_t)rw * 4);
    return o;
}

inline void opaque(Image &im) { for (size_t i = 3; i < im.px.size(); i += 4) im.px[i] = 255; }
inline bool has_alpha(const Image &im) { for (size_t i = 3; i < im.px.size(); i += 4) if (im.px[i] < 250) return true; return false; }

inline void png_sink(void *ctx, void *data, int size) {
    auto *v = (std::vector<uint8_t> *)ctx; v->insert(v->end(), (uint8_t *)data, (uint8_t *)data + size);
}
inline std::vector<uint8_t> to_png(const Image &im) {
    std::vector<uint8_t> out;
    stbi_write_png_to_func(png_sink, &out, im.w, im.h, 4, im.px.data(), im.w * 4);
    return out;
}

// BC7 DDS with a DX10 header - byte-for-byte the layout PS5 titles ship (single mip).
inline std::vector<uint8_t> to_bc7_dds(const Image &im) {
    const int bw = im.w / 4, bh = im.h / 4;
    std::vector<uint8_t> out(148 + (size_t)bw * bh * 16, 0);
    auto u32 = [&](int off, uint32_t v) { memcpy(&out[off], &v, 4); };
    memcpy(&out[0], "DDS ", 4);
    u32(4, 124); u32(8, 0x000A1007); u32(12, im.h); u32(16, im.w); u32(20, (uint32_t)(bw * bh * 16));
    u32(24, 1); u32(28, 1); u32(76, 32); u32(80, 4); memcpy(&out[84], "DX10", 4); u32(108, 0x1000);
    u32(128, 98); u32(132, 3); u32(136, 0); u32(140, 1); u32(144, 1);
    static bool init = (bc7enc_compress_block_init(), true); (void)init;
    unsigned nt = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    for (unsigned t = 0; t < nt; t++) pool.emplace_back([&, t] {
        bc7enc_compress_block_params p; bc7enc_compress_block_params_init(&p);
        p.m_uber_level = 1; p.m_max_partitions = 64;
        uint8_t blk[64];
        for (int by = (int)t; by < bh; by += (int)nt)
            for (int bx = 0; bx < bw; bx++) {
                for (int y = 0; y < 4; y++) memcpy(blk + y * 16, &im.px[((size_t)(by * 4 + y) * im.w + bx * 4) * 4], 16);
                bc7enc_compress_block(&out[148 + ((size_t)by * bw + bx) * 16], blk, &p);
            }
    });
    for (auto &th : pool) th.join();
    return out;
}

// Decode a BC7 DX10 DDS (as written above / as PS5 titles ship). Returns false for other formats.
inline bool from_bc7_dds(const std::vector<uint8_t> &d, Image &out) {
    if (d.size() < 148 || memcmp(d.data(), "DDS ", 4) != 0 || memcmp(&d[84], "DX10", 4) != 0) return false;
    uint32_t h, w, fmt; memcpy(&h, &d[12], 4); memcpy(&w, &d[16], 4); memcpy(&fmt, &d[128], 4);
    if ((fmt != 98 && fmt != 99) || w % 4 || h % 4 || d.size() < 148 + (size_t)w * h) return false;
    out.w = (int)w; out.h = (int)h; out.px.resize((size_t)w * h * 4);
    const int bw = (int)w / 4;
    for (int by = 0; by < (int)h / 4; by++)
        for (int bx = 0; bx < bw; bx++) {
            uint8_t px[64];
            bc7decomp::unpack_bc7(&d[148 + ((size_t)by * bw + bx) * 16], (bc7decomp::color_rgba *)px);
            for (int y = 0; y < 4; y++) memcpy(&out.px[((size_t)(by * 4 + y) * w + bx * 4) * 4], px + y * 16, 16);
        }
    return true;
}

}  // namespace img
