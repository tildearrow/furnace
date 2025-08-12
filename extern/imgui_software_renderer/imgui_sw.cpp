// By Emil Ernerfeldt 2018
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.

#include "imgui.h"
#include "imgui_internal.h"
#ifndef IMGUI_DISABLE
#include "imgui_sw.hpp"

#include <algorithm>
#include <math.h>
#include <vector>
#include <SDL.h>

struct ImGui_ImplSW_Data
{
    SDL_Window*  Window;
    SWTexture*   FontTexture;

    ImGui_ImplSW_Data() { memset((void*)this, 0, sizeof(*this)); }
};

static ImGui_ImplSW_Data* ImGui_ImplSW_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplSW_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

struct PaintTarget
{
  uint32_t *pixels;
  int width;
  int height;
};

// ----------------------------------------------------------------------------

#pragma pack(push, 1)
union ColorInt
{
  struct {
#ifdef TA_BIG_ENDIAN
    uint8_t a, r, g, b;
#else
    uint8_t b, g, r, a;
#endif
  };
  uint32_t u32;
  ColorInt():
    u32(0) {}

  ColorInt(uint32_t c):
    u32(c) {}

  ColorInt(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha):
    b(blue),
    g(green),
    r(red),
    a(alpha) {}

  static ColorInt bgra(uint32_t c) {
    return ColorInt((c&0xff00ff00)|((c&0xff)<<16)|((c&0xff0000)>>16));
  }


  ColorInt &operator*=(const ColorInt &other)
  {
    r = (r * other.r + 255) >> 8;
    g = (g * other.g + 255) >> 8;
    b = (b * other.b + 255) >> 8;
    a = (a * other.a + 255) >> 8;
    return *this;
  }
};
#pragma pack(pop)

static inline uint32_t blend(const ColorInt &target, const ColorInt &source)
{
  if (source.a == 0) return target.u32;
  if (source.a >= 255) return source.u32;
  const unsigned char ia=255-source.a;
  return (
    (target.a << 24u) |
    (((source.r * source.a + target.r * ia + 255) >> 8) << 16u) |
    (((source.g * source.a + target.g * ia + 255) >> 8) << 8u) |
    (((source.b * source.a + target.b * ia + 255) >> 8) << 0u)
  );
}

// ----------------------------------------------------------------------------
// Used for interpolating vertex attributes (color and texture coordinates) in a triangle.

struct Barycentric
{
  float w0, w1, w2;
};

Barycentric operator*(const float f, const Barycentric &va) { return { f * va.w0, f * va.w1, f * va.w2 }; }

void operator+=(Barycentric &a, const Barycentric &b)
{
  a.w0 += b.w0;
  a.w1 += b.w1;
  a.w2 += b.w2;
}

Barycentric operator+(const Barycentric &a, const Barycentric &b)
{
  return Barycentric{ a.w0 + b.w0, a.w1 + b.w1, a.w2 + b.w2 };
}

// ----------------------------------------------------------------------------
// Useful operators on ImGui vectors:

ImVec2 operator*(const float f, const ImVec2 &v) { return ImVec2{ f * v.x, f * v.y }; }

ImVec4 operator*(const float f, const ImVec4 &v) { return ImVec4{ f * v.x, f * v.y, f * v.z, f * v.w }; }


ColorInt operator*(const float other, const ColorInt& that)
{
  return ColorInt(
    (that.r * (int)(other * 256.0f)) >> 8,
    (that.g * (int)(other * 256.0f)) >> 8,
    (that.b * (int)(other * 256.0f)) >> 8,
    (that.a * (int)(other * 256.0f)) >> 8
  );
}

ColorInt operator+(const ColorInt& l, const ColorInt& r) {
  return ColorInt(l.u32+r.u32);
}

// ----------------------------------------------------------------------------
// Copies of functions in ImGui, inlined for speed:

inline ImVec4 color_convert_u32_to_float4(ImU32 in)
{
  const float s = 1.0f / 255.0f;
  return ImVec4(((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
    ((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
    ((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
    ((in >> IM_COL32_A_SHIFT) & 0xFF) * s);
}

inline ImU32 color_convert_float4_to_u32(const ImVec4 &in)
{
  ImU32 out;
  out = uint32_t(in.x * 255.0f + 0.5f) << IM_COL32_B_SHIFT;
  out |= uint32_t(in.y * 255.0f + 0.5f) << IM_COL32_G_SHIFT;
  out |= uint32_t(in.z * 255.0f + 0.5f) << IM_COL32_R_SHIFT;
  out |= uint32_t(in.w * 255.0f + 0.5f) << IM_COL32_A_SHIFT;
  return out;
}

// ----------------------------------------------------------------------------
// For fast and subpixel-perfect triangle rendering we used fixed point arithmetic.
// To keep the code simple we use 64 bits to avoid overflows.
// TODO: make it 32-bit or else

using Int = int32_t;
const Int kFixedBias = 1;

struct Point
{
  Int x, y;
};

Int orient2d(const Point &a, const Point &b, const Point &c)
{
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

Int as_int(float v) { return static_cast<Int>(floor(v * kFixedBias)); }

Point as_point(ImVec2 v) { return Point{ as_int(v.x), as_int(v.y) }; }

// ----------------------------------------------------------------------------

inline float min3(float a, float b, float c)
{
  if (a < b && a < c) { return a; }
  return b < c ? b : c;
}

inline float max3(float a, float b, float c)
{
  if (a > b && a > c) { return a; }
  return b > c ? b : c;
}

inline float barycentric(const ImVec2 &a, const ImVec2 &b, const ImVec2 &point)
{
  return (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x);
}

inline uint8_t sample_font_texture(const SWTexture &texture, int x, int y)
{
  return ((const uint8_t*)texture.pixels)[x + y];
}

inline uint32_t sample_texture(const SWTexture &texture, int x, int y) { return texture.pixels[x + y]; }

static void paint_uniform_rectangle(const PaintTarget &target,
  const ImVec2 &min_f,
  const ImVec2 &max_f,
  const ColorInt &color)
{
  // don't if our rectangle is transparent
  if (color.a==0) return;

  // Integer bounding box [min, max):
  int min_x_i = (int)(min_f.x + 0.5f);
  int min_y_i = (int)(min_f.y + 0.5f);
  int max_x_i = (int)(max_f.x + 0.5f);
  int max_y_i = (int)(max_f.y + 0.5f);

  // Clamp to render target:
  min_x_i = std::max(min_x_i, 0);
  min_y_i = std::max(min_y_i, 0);
  max_x_i = std::min(max_x_i, target.width);
  max_y_i = std::min(max_y_i, target.height);

  if (color.a==255) {
    // fast path if alpha blending is not necessary
    for (int y = min_y_i; y < max_y_i; ++y) {
      uint32_t* target_pixel = &target.pixels[y * target.width + min_x_i - 1];
      for (int x = min_x_i; x < max_x_i; ++x) {
        ++target_pixel;
        *target_pixel = color.u32;
      }
    }
  } else {
    // We often blend the same colors over and over again, so optimize for this (saves 25% total cpu):
    uint32_t last_target_pixel = target.pixels[min_y_i * target.width + min_x_i];
    const ColorInt* lastColorRef = (const ColorInt*)(&last_target_pixel);
    uint32_t last_output = blend(*lastColorRef, color);

    for (int y = min_y_i; y < max_y_i; ++y) {
      uint32_t* target_pixel = &target.pixels[y * target.width + min_x_i - 1];
      for (int x = min_x_i; x < max_x_i; ++x) {
        ++target_pixel;
        if (*target_pixel == last_target_pixel) {
          *target_pixel = last_output;
          continue;
        }
        last_target_pixel = *target_pixel;
        const ColorInt* colorRef = (const ColorInt*)(target_pixel);
        *target_pixel = blend(*colorRef, color);
        last_output = *target_pixel;
      }
    }
  }
}

static void paint_uniform_textured_rectangle(const PaintTarget &target,
  const SWTexture &texture,
  const ImVec4 &clip_rect,
  const ImDrawVert &min_v,
  const ImDrawVert &max_v)
{
  const ImVec2 min_p = ImVec2(min_v.pos.x, min_v.pos.y);
  const ImVec2 max_p = ImVec2(max_v.pos.x, max_v.pos.y);

  float distanceX = max_p.x - min_p.x;
  float distanceY = max_p.y - min_p.y;
  if (distanceX == 0 || distanceY == 0) { return; }

  // Find bounding box:
  float min_x_f = min_p.x;
  float min_y_f = min_p.y;
  float max_x_f = max_p.x;
  float max_y_f = max_p.y;

  // Clip against clip_rect:
  min_x_f = std::max(min_x_f, clip_rect.x);
  min_y_f = std::max(min_y_f, clip_rect.y);
  max_x_f = std::min(max_x_f, clip_rect.z - 0.5f);
  max_y_f = std::min(max_y_f, clip_rect.w - 0.5f);

  // Integer bounding box [min, max):
  int min_x_i = (int)(min_x_f);
  int min_y_i = (int)(min_y_f);
  int max_x_i = (int)(max_x_f + 1.0f);
  int max_y_i = (int)(max_y_f + 1.0f);

  // Clip against render target:
  min_x_i = std::max(min_x_i, 0);
  min_y_i = std::max(min_y_i, 0);
  max_x_i = std::min(max_x_i, target.width);
  max_y_i = std::min(max_y_i, target.height);

  const auto topleft = ImVec2(min_x_i + 0.5f, min_y_i + 0.5f);
  const ImVec2 delta_uv_per_pixel = {
    (max_v.uv.x - min_v.uv.x) / distanceX,
    (max_v.uv.y - min_v.uv.y) / distanceY,
  };
  const ImVec2 uv_topleft = {
    min_v.uv.x + (topleft.x - min_v.pos.x) * delta_uv_per_pixel.x,
    min_v.uv.y + (topleft.y - min_v.pos.y) * delta_uv_per_pixel.y,
  };

  int startX = uv_topleft.x * (texture.width - 1.0f) + 0.5f;
  int startY = uv_topleft.y * (texture.height - 1.0f) + 0.5f;

  if (startX<0) startX=0;
  if (startX>texture.width-1) startX=texture.width-1;
  if (startY<0) startY=0;
  if (startY>texture.height-1) startY=texture.height-1;

  int currentX = startX;
  int currentY = startY * texture.width;

  float deltaX = delta_uv_per_pixel.x * texture.width;
  float deltaY = delta_uv_per_pixel.y * texture.height;

  const ColorInt colorRef = ColorInt::bgra(min_v.col);

  for (int y = min_y_i; y < max_y_i; ++y) {
    currentX = startX;
    uint32_t* target_pixel = &target.pixels[y * target.width - 1 + min_x_i];
    for (int x = min_x_i; x < max_x_i; ++x) {
      ++target_pixel;
      const ColorInt* targetColorRef = (const ColorInt*)(target_pixel);

      if (texture.isAlpha) {
        uint8_t texel = sample_font_texture(texture, currentX, currentY);
        if (deltaX != 0 && currentX < texture.width - 1) { currentX += 1; }

        // The font texture is all black or all white, so optimize for this:
        // anti-aliasing will be lost, but it doesn't matter
        if (texel & 0x80) {
          *target_pixel = blend(*targetColorRef, colorRef);
        }
        continue;

      } else {
        uint32_t texColor = sample_texture(texture, currentX, currentY);
        auto src_color = ColorInt(texColor);

        if (deltaX != 0 && currentX < texture.width - 1) { currentX += 1; }

        src_color *= colorRef;
        *target_pixel = blend(*targetColorRef, src_color);
      }
    }
    if (deltaY != 0 && currentY < (texture.height - 1)*texture.width) { currentY += texture.width; }
  }
}

// When two triangles share an edge, we want to draw the pixels on that edge exactly once.
// The edge will be the same, but the direction will be the opposite
// (assuming the two triangles have the same winding order).
// Which edge wins? This functions decides.
static bool is_dominant_edge(ImVec2 edge)
{
  // return edge.x < 0 || (edge.x == 0 && edge.y > 0);
  return edge.y > 0 || (edge.y == 0 && edge.x < 0);
}

// Handles triangles in any winding order (CW/CCW)
static void paint_triangle(const PaintTarget &target,
  const SWTexture *texture,
  const ImVec4 &clip_rect,
  const ImDrawVert &v0,
  const ImDrawVert &v1,
  const ImDrawVert &v2)
{
  const ImVec2 p0 = ImVec2(v0.pos.x, v0.pos.y);
  const ImVec2 p1 = ImVec2(v1.pos.x, v1.pos.y);
  const ImVec2 p2 = ImVec2(v2.pos.x, v2.pos.y);

  const auto rect_area = barycentric(p0, p1, p2);// Can be positive or negative depending on winding order
  if (rect_area == 0.0f) { return; }
  // if (rect_area < 0.0f) { return paint_triangle(target, texture, clip_rect, v0, v2, v1); }

  // Find bounding box:
  float min_x_f = min3(p0.x, p1.x, p2.x);
  float min_y_f = min3(p0.y, p1.y, p2.y);
  float max_x_f = max3(p0.x, p1.x, p2.x);
  float max_y_f = max3(p0.y, p1.y, p2.y);

  // Clip against clip_rect:
  min_x_f = std::max(min_x_f, clip_rect.x);
  min_y_f = std::max(min_y_f, clip_rect.y);
  max_x_f = std::min(max_x_f, clip_rect.z - 0.5f);
  max_y_f = std::min(max_y_f, clip_rect.w - 0.5f);

  // Integer bounding box [min, max):
  int min_x_i = (int)(min_x_f);
  int min_y_i = (int)(min_y_f);
  int max_x_i = (int)(max_x_f + 1.0f);
  int max_y_i = (int)(max_y_f + 1.0f);

  // Clip against render target:
  min_x_i = std::max(min_x_i, 0);
  min_y_i = std::max(min_y_i, 0);
  max_x_i = std::min(max_x_i, target.width);
  max_y_i = std::min(max_y_i, target.height);

  // ------------------------------------------------------------------------
  // Set up interpolation of barycentric coordinates:

  const auto topleft = ImVec2(min_x_i + 0.5f, min_y_i + 0.5f);
  const auto dx = ImVec2(1, 0);
  const auto dy = ImVec2(0, 1);

  const auto w0_topleft = barycentric(p1, p2, topleft);
  const auto w1_topleft = barycentric(p2, p0, topleft);
  const auto w2_topleft = barycentric(p0, p1, topleft);

  const auto w0_dx = barycentric(p1, p2, topleft + dx) - w0_topleft;
  const auto w1_dx = barycentric(p2, p0, topleft + dx) - w1_topleft;
  const auto w2_dx = barycentric(p0, p1, topleft + dx) - w2_topleft;

  const auto w0_dy = barycentric(p1, p2, topleft + dy) - w0_topleft;
  const auto w1_dy = barycentric(p2, p0, topleft + dy) - w1_topleft;
  const auto w2_dy = barycentric(p0, p1, topleft + dy) - w2_topleft;

  const Barycentric bary_0{ 1, 0, 0 };
  const Barycentric bary_1{ 0, 1, 0 };
  const Barycentric bary_2{ 0, 0, 1 };

  const auto inv_area = 1 / rect_area;
  const Barycentric bary_topleft = inv_area * (w0_topleft * bary_0 + w1_topleft * bary_1 + w2_topleft * bary_2);
  const Barycentric bary_dx = inv_area * (w0_dx * bary_0 + w1_dx * bary_1 + w2_dx * bary_2);
  const Barycentric bary_dy = inv_area * (w0_dy * bary_0 + w1_dy * bary_1 + w2_dy * bary_2);

  Barycentric bary_current_row = bary_topleft;

  // ------------------------------------------------------------------------
  // For pixel-perfect inside/outside testing:

  const int sign = rect_area > 0 ? 1 : -1;// winding order?

  const int bias0i = is_dominant_edge(p2 - p1) ? 0 : -1;
  const int bias1i = is_dominant_edge(p0 - p2) ? 0 : -1;
  const int bias2i = is_dominant_edge(p1 - p0) ? 0 : -1;

  const auto p0i = as_point(p0);
  const auto p1i = as_point(p1);
  const auto p2i = as_point(p2);

  // ------------------------------------------------------------------------

  const bool has_uniform_color = (v0.col == v1.col && v0.col == v2.col);

  const ColorInt c0 = ColorInt::bgra(v0.col);
  const ColorInt c1 = ColorInt::bgra(v1.col);
  const ColorInt c2 = ColorInt::bgra(v2.col);

  // We often blend the same colors over and over again, so optimize for this (saves 10% total cpu):
  uint32_t last_target_pixel = 0;
  const ColorInt* lastColorRef = (const ColorInt*)(&last_target_pixel);
  const ColorInt colorRef = ColorInt::bgra(v0.col);
  uint32_t last_output = blend(*lastColorRef, colorRef);

  for (int y = min_y_i; y < max_y_i; ++y) {
    auto bary = bary_current_row;

    bool has_been_inside_this_row = false;

    uint32_t* target_pixel = &target.pixels[y * target.width + min_x_i - 1];

    for (int x = min_x_i; x < max_x_i; ++x) {
      const auto w0 = bary.w0;
      const auto w1 = bary.w1;
      const auto w2 = bary.w2;
      bary += bary_dx;

      ++target_pixel;

      {
        // Inside/outside test:
        const auto p = Point{ kFixedBias * x + kFixedBias / 2, kFixedBias * y + kFixedBias / 2 };
        const auto w0i = sign * orient2d(p1i, p2i, p) + bias0i;
        const auto w1i = sign * orient2d(p2i, p0i, p) + bias1i;
        const auto w2i = sign * orient2d(p0i, p1i, p) + bias2i;
        if (w0i < 0 || w1i < 0 || w2i < 0) {
          if (has_been_inside_this_row) {
            break;// Gives a nice 10% speedup
          } else {
            continue;
          }
        }
      }
      has_been_inside_this_row = true;


      if (has_uniform_color && !texture) {
        if (*target_pixel == last_target_pixel) {
          *target_pixel = last_output;
          continue;
        }
        last_target_pixel = *target_pixel;
        *target_pixel = blend(*lastColorRef, colorRef);
        last_output = *target_pixel;
        continue;
      }

      ColorInt src_color;

      if (has_uniform_color) {
        src_color = c0;
      } else {
        src_color = w0 * c0 + w1 * c1 + w2 * c2;
      }

      if (texture) {
        if (!texture->isAlpha) { printf("warning: different texture\n"); }

        const ImVec2 uv = w0 * v0.uv + w1 * v1.uv + w2 * v2.uv;
        int x = uv.x * (texture->width - 1.0f) + 0.5f;
        int y = uv.y * (texture->height - 1.0f) + 0.5f;
        src_color.a = (src_color.a * sample_font_texture(*texture, x, y) + 255) >> 8;
      }

      if (!src_color.a) { continue; }// Transparent.
      if (src_color.a == 255) {
        // Opaque, no blending needed:
        *target_pixel = src_color.u32;
        continue;
      }

      const ColorInt* target_color = (const ColorInt*)target_pixel;
      *target_pixel = blend(*target_color, src_color);
    }

    bary_current_row += bary_dy;
  }
}

static void paint_draw_cmd(const PaintTarget &target,
  const ImDrawVert *vertices,
  const ImDrawIdx *idx_buffer,
  const ImDrawCmd &pcmd,
  const ImVec2& white_uv)
{
  const SWTexture* texture = (const SWTexture*)(pcmd.TextureId);
  IM_ASSERT(texture);

  for (unsigned int i = 0; i + 3 <= pcmd.ElemCount;) {
    ImDrawVert v0 = vertices[idx_buffer[i + 0]];
    ImDrawVert v1 = vertices[idx_buffer[i + 1]];
    ImDrawVert v2 = vertices[idx_buffer[i + 2]];

    // Text is common, and is made of textured rectangles. So let's optimize for it.
    // This assumes the ImGui way to layout text does not change.
    if (i + 6 <= pcmd.ElemCount && idx_buffer[i + 3] == idx_buffer[i + 0]
        && idx_buffer[i + 4] == idx_buffer[i + 2]) {
      ImDrawVert v3 = vertices[idx_buffer[i + 5]];

      if (v0.pos.x == v3.pos.x && v1.pos.x == v2.pos.x && v0.pos.y == v1.pos.y && v2.pos.y == v3.pos.y
          && v0.uv.x == v3.uv.x && v1.uv.x == v2.uv.x && v0.uv.y == v1.uv.y && v2.uv.y == v3.uv.y) {
        const bool has_uniform_color = v0.col == v1.col && v0.col == v2.col && v0.col == v3.col;

        const bool has_texture = v0.uv != white_uv || v1.uv != white_uv || v2.uv != white_uv || v3.uv != white_uv;

        if (has_uniform_color && has_texture) {
          paint_uniform_textured_rectangle(target, *texture, pcmd.ClipRect, v0, v2);
          i += 6;
          continue;
        }
      }
    }

    // A lot of the big stuff are uniformly colored rectangles,
    // so we can save a lot of CPU by detecting them:
    if (i + 6 <= pcmd.ElemCount) {
      ImDrawVert v3 = vertices[idx_buffer[i + 3]];
      ImDrawVert v4 = vertices[idx_buffer[i + 4]];
      ImDrawVert v5 = vertices[idx_buffer[i + 5]];

      ImVec2 min, max;
      min.x = min3(v0.pos.x, v1.pos.x, v2.pos.x);
      min.y = min3(v0.pos.y, v1.pos.y, v2.pos.y);
      max.x = max3(v0.pos.x, v1.pos.x, v2.pos.x);
      max.y = max3(v0.pos.y, v1.pos.y, v2.pos.y);

      // Not the prettiest way to do this, but it catches all cases
      // of a rectangle split into two triangles.
      // TODO: Stop it from also assuming duplicate triangles is one rectangle.
      if ((v0.pos.x == min.x || v0.pos.x == max.x) && (v0.pos.y == min.y || v0.pos.y == max.y)
          && (v1.pos.x == min.x || v1.pos.x == max.x) && (v1.pos.y == min.y || v1.pos.y == max.y)
          && (v2.pos.x == min.x || v2.pos.x == max.x) && (v2.pos.y == min.y || v2.pos.y == max.y)
          && (v3.pos.x == min.x || v3.pos.x == max.x) && (v3.pos.y == min.y || v3.pos.y == max.y)
          && (v4.pos.x == min.x || v4.pos.x == max.x) && (v4.pos.y == min.y || v4.pos.y == max.y)
          && (v5.pos.x == min.x || v5.pos.x == max.x) && (v5.pos.y == min.y || v5.pos.y == max.y)) {
        const bool has_uniform_color =
          v0.col == v1.col && v0.col == v2.col && v0.col == v3.col && v0.col == v4.col && v0.col == v5.col;

        min.x = std::max(min.x, pcmd.ClipRect.x);
        min.y = std::max(min.y, pcmd.ClipRect.y);
        max.x = std::min(max.x, pcmd.ClipRect.z - 0.5f);
        max.y = std::min(max.y, pcmd.ClipRect.w - 0.5f);

        if (max.x < min.x || max.y < min.y) {
          i += 6;
          continue;
        }// Completely clipped

        if (has_uniform_color) {
          const ColorInt colorRef = ColorInt::bgra(v0.col);
          paint_uniform_rectangle(target, min, max, colorRef);
          i += 6;
          continue;
        }
      }
    }

    const bool has_texture = (v0.uv != white_uv || v1.uv != white_uv || v2.uv != white_uv);
    paint_triangle(target, has_texture ? texture : nullptr, pcmd.ClipRect, v0, v1, v2);
    i += 3;
  }
}

static void paint_draw_list(const PaintTarget &target, const ImDrawList *cmd_list)
{
  const ImDrawIdx *idx_buffer = &cmd_list->IdxBuffer[0];
  const ImDrawVert *vertices = cmd_list->VtxBuffer.Data;
  const ImVec2 white_uv = cmd_list->_Data->TexUvWhitePixel;

  for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++) {
    const ImDrawCmd &pcmd = cmd_list->CmdBuffer[cmd_i];
    if (pcmd.UserCallback) {
      pcmd.UserCallback(cmd_list, &pcmd);
    } else {
      paint_draw_cmd(target, vertices, idx_buffer, pcmd, white_uv);
    }
    idx_buffer += pcmd.ElemCount;
  }
  }

static void paint_imgui(uint32_t *pixels, ImDrawData *drawData, int fb_width, int fb_height)
{
  if (fb_width <= 0 || fb_height <= 0) return;

  PaintTarget target{ pixels, fb_width, fb_height };

  for (int i = 0; i < drawData->CmdListsCount; ++i) {
    paint_draw_list(target, drawData->CmdLists[i]);
  }
}

/// NEW STUFF

bool ImGui_ImplSW_Init(SDL_Window* win) {
  ImGuiIO& io = ImGui::GetIO();
  ImGuiIO& platform_io = ImGui::GetPlatformIO();
  IM_ASSERT(io.BackendRendererUserData == nullptr);

  if (SDL_HasWindowSurface(win)==SDL_FALSE) {
    return false;
  }

  ImGui_ImplSW_Data* bd = IM_NEW(ImGui_ImplSW_Data)();
  bd->Window = win;
  io.BackendRendererUserData = (void*)bd;
  io.BackendRendererName = "imgui_sw";
  io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

  platform_io.Renderer_TextureMaxWidth = platform_io.Renderer_TextureMaxHeight = (int)4096;

  return true;
}

void ImGui_ImplSW_Shutdown() {
  ImGui_ImplSW_Data* bd = ImGui_ImplSW_GetBackendData();
  IM_ASSERT(bd != nullptr);
  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplSW_DestroyDeviceObjects();
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
  IM_DELETE(bd);
}

bool ImGui_ImplSW_NewFrame() {
  ImGui_ImplSW_Data* bd = ImGui_ImplSW_GetBackendData();
  IM_ASSERT(bd != nullptr);

  return true;
}

void ImGui_ImplSW_RenderDrawData(ImDrawData* draw_data) {
  ImGui_ImplSW_Data* bd = ImGui_ImplSW_GetBackendData();
  IM_ASSERT(bd != nullptr);

  SDL_Surface* surf = SDL_GetWindowSurface(bd->Window);
  if (!surf) return;

  bool mustLock=SDL_MUSTLOCK(surf);
  if (mustLock) {
    if (SDL_LockSurface(surf)!=0) return;
  }
  paint_imgui((uint32_t*)surf->pixels,draw_data,surf->w,surf->h);
  // 0xAARRGGBB
  if (mustLock) {
    SDL_UnlockSurface(surf);
  }
}

/// CREATE OBJECTS

bool ImGui_ImplSW_CreateFontsTexture() {
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSW_Data* bd = ImGui_ImplSW_GetBackendData();

  // Load default font (embedded in code):
  uint8_t *tex_data;
  int font_width, font_height;
  io.Fonts->GetTexDataAsAlpha8(&tex_data, &font_width, &font_height);
  SWTexture* texture = new SWTexture((uint32_t*)tex_data,font_width,font_height,true);
  io.Fonts->SetTexID((ImTextureID)texture);
  bd->FontTexture = texture;

  return true;
}

void ImGui_ImplSW_DestroyFontsTexture() {
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplSW_Data* bd = ImGui_ImplSW_GetBackendData();
  if (bd->FontTexture)
  {
      delete bd->FontTexture;
      io.Fonts->SetTexID(0);
      bd->FontTexture = 0;
  }
}

bool ImGui_ImplSW_CreateDeviceObjects() {
  return true;
}

void ImGui_ImplSW_DestroyDeviceObjects() {
  ImGui_ImplSW_DestroyFontsTexture();
}

#endif // #ifndef IMGUI_DISABLE
