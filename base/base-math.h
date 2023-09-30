// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// TODO(Ryan): sse_mathfun.h

#include <math.h>

#define SQUARE(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))

#define F32_ROUND_U32(real32) (u32)roundf(real32)
#define F32_ROUND_S32(real32) (s32)roundf(real32)
#define F32_FLOOR_S32(real32) (s32)floorf(real32)
#define F32_CEIL_U32(real32) (u32)ceilf(real32)
#define F32_CEIL_S32(real32) (s32)ceilf(real32) 
#define F32_DEG_TO_RAD(v) (F32_PI_DIV_180 * (v))
#define F32_RAD_TO_DEG(v) (F32_180_DIV_PI * (v))
#define F32_TURNS_TO_DEG(v) ((v) * 360.0f)
#define F32_TURNS_TO_RAD(v) ((v) * (F32_TAU))
#define F32_DEG_TO_TURNS(v) ((v) / 360.0f)
#define F32_RAD_TO_TURNS(v) ((v) / (F32_TAU))
#define F32_SQRT(x) sqrtf(x)
#define F32_SIN(x) sinf(x)
#define F32_COS(x) cosf(x)
#define F32_TAN(x) tanf(x)
#define F32_ATAN2(x, y) atan2f(x, y)
#define F32_LN(x) logf(x)

#define F64_SQRT(x) sqrt(x)
#define F64_SIN(x) sin(x) 
#define F64_COS(x) cos(x) 
#define F64_TAN(x) tan(x)
#define F64_LN(x) log(x)
#define F64_DEG_TO_RAD(v) (F64_PI_DIV_180 * (v))
#define F64_RAD_TO_DEG(v) (F64_180_DIV_PI * (v))
#define F64_TURNS_TO_DEG(v) ((v) * 360.0)
#define F64_TURNS_TO_RAD(v) ((v) * F64_TAU)
#define F64_DEG_TO_TURNS(v) ((v) / 360.0)
#define F64_RAD_TO_TURNS(v) ((v) / F64_TAU)

#if defined(COMPILER_GCC) && defined(ARCH_X86_64)
  #include <x86intrin.h>
  INTERNAL f32 f32_pow(f32 x, f32 y) { return __builtin_powf(x, y); }
  INTERNAL f32 f32_fmodf(f32 x, f32 y) { return __builtin_fmodf(x, y); }
  INTERNAL f32 f32_acos(f32 x) { return __builtin_acosf(x); }
  INTERNAL f32 f32_powi(f32 x, s32 y) { return __builtin_powif(x, y); }
  INTERNAL u32 u32_count_bits_set(u32 val) { return (u32)__builtin_popcount(val); }
  INTERNAL u32 u32_count_leading_zeroes(u32 val) { return (u32)__builtin_clz(val); }
  INTERNAL u32 u32_count_trailing_zeroes(u32 val) { return (u32)__builtin_ctz(val); }
  INTERNAL u32 u32_get_parity(u32 val) { return (u32)__builtin_parity(val); }
  INTERNAL u16 u16_endianness_swap(u16 val) { return __builtin_bswap16(val); }
  INTERNAL u32 u32_endianness_swap(u32 val) { return __builtin_bswap32(val); }
  INTERNAL u64 u64_endianness_swap(u64 val) { return __builtin_bswap64(val); }
#endif

INTERNAL f32
f32_lerp(f32 a, f32 b, f32 t)
{
  f32 result = 0.0f;
  result = ((b - a) * t) + a; 

  return result;
}

INTERNAL f32
f32_move_toward(f32 current, f32 target, f32 dt, f32 rate)
{
  f32 result = current;

  if (current > target)
  {
    result -= dt * rate;
    if (result < target) result = target; 
  }
  else if (current < target)
  {
    result += dt * rate;
    if (result > target) result = target; 
  }

  return result;
}


INTERNAL memory_index
memory_index_round_to_nearest(memory_index val, memory_index near)
{
  memory_index result = val;

  result += near - 1;
  result -= result % near;
  
  return result;
}

INTERNAL u32 
u32_rand(u32 *seed)
{
  u32 x = *seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *seed = x;

  return *seed;
}

INTERNAL f32 
f32_rand_unilateral(u32 *seed)
{
  u32 exponent = 127;
  u32 mantissa = u32_rand(seed) >> 9;
  u32 bits = (exponent << 23) | mantissa;
  f32 result = *(f32 *)&bits - 1.0f;

  return result;
}

INTERNAL f32 
f32_rand_bilateral(u32 *seed)
{
  return -1.0f + (2.0f * f32_rand_unilateral(seed));
}

INTERNAL u32 
u32_rand_range(u32 *seed, u32 range)
{
  u32 result = u32_rand(seed) % range;

  return result;
}

INTERNAL s32
s32_rand_range(u32 *seed, s32 min, s32 max)
{
  if (max < min)
  {
    max = min;
  }

  s32 result = min + (s32)u32_rand(seed) % (max - min + 1);

  return result;
}

INTERNAL f32 
f32_rand_range(u32 *seed, f32 min, f32 max)
{
  f32 range = f32_rand_unilateral(seed);
  f32 result = min + range * (max - min);

  return result;
}

INTERNAL f32
f32_norm(f32 start, f32 a, f32 end)
{
  f32 result = 0.0f;

  a = CLAMP(start, a, end);

  result = (end - a) / (end - start);

  return result;
}

INTERNAL f32
f32_map_to_range(f32 x0, f32 x1, f32 a, f32 y0, f32 y1)
{
  f32 result = 0.0f;

  f32 norm = f32_norm(x0, a, x1);

  result = y0 + norm * (y1 - y0);

  return result;
}


// NOTE(Ryan): To allow for anonymous structs
IGNORE_WARNING_PEDANTIC()
typedef union Vec2F32 Vec2F32;
union Vec2F32
{
  struct
  {
    f32 x, y;
  };
  struct
  {
    f32 w, h;
  };

  f32 elements[2];
  f32 v[2];
};

typedef union Vec2S32 Vec2S32;
union Vec2S32
{
  struct
  {
    s32 x, y;
  };

  struct
  {
    s32 w, h;
  };

  s32 elements[2];
  s32 v[2];
};

typedef union Vec2S64 Vec2S64;
union Vec2S64
{
  struct
  {
    s64 x, y;
  };

  s64 elements[2];
  s64 v[2];
};

typedef union Vec3F32 Vec3F32;
union Vec3F32
{
  struct
  {
    f32 x, y, z;
  };

  struct
  {
    f32 r, g, b;
  };

  struct
  {
    Vec2F32 xy;
    f32 z1;
  };

  struct
  {
    f32 x1;
    Vec2F32 yz;
  };

  f32 elements[3];
  f32 v[3];
};

typedef union Vec3S32 Vec3S32;
union Vec3S32
{
  struct
  {
    s32 x, y, z;
  };

  struct
  {
    s32 r, g, b;
  };

  s32 elements[3];
  s32 v[3];
};

typedef union Vec3S64 Vec3S64;
union Vec3S64
{
  struct
  {
    s64 x, y, z;
  };

  struct
  {
    s64 r, g, b;
  };

  s64 elements[3];
  s64 v[3];
};


typedef union Vec4F32 Vec4F32;
union Vec4F32
{
  struct
  {
    f32 x, y, z, w;
  };

  struct
  {
    Vec2F32 xy, zw;
  };

  struct
  {
    Vec3F32 xyz;
    f32 w1;
  };

  struct
  {
    f32 x1;
    Vec3F32 yzw;
  };

  struct
  {
    f32 r, g, b, a;
  };

  struct
  {
    Vec3F32 rgb;
    f32 a1;
  };

  struct
  {
    f32 r1;
    Vec3F32 gba;
  };

  f32 elements[4];
  f32 v[4];
};

typedef union Vec4S32 Vec4S32;
union Vec4S32
{
  struct
  {
    s32 x, y, z, w;
  };

  struct
  {
    Vec2S32 xy, zw;
  };

  struct
  {
    Vec3S32 xyz;
    s32 w1;
  };

  struct
  {
    s32 x1;
    Vec3S32 yzw;
  };

  struct
  {
    s32 r, g, b, a;
  };

  struct
  {
    Vec3S32 rgb;
    s32 a1;
  };

  struct
  {
    s32 r1;
    Vec3S32 gba;
  };

  s32 elements[4];
  s32 v[4];
};

typedef union Vec4S64 Vec4S64;
union Vec4S64
{
  struct
  {
    s64 x, y, z, w;
  };

  struct
  {
    Vec2S64 xy, zw;
  };

  struct
  {
    Vec3S64 xyz;
    s64 w1;
  };

  struct
  {
    s64 x1;
    Vec3S64 yzw;
  };

  struct
  {
    s64 r, g, b, a;
  };

  struct
  {
    s64 r1;
    Vec3S64 gba;
  };

  struct
  {
    Vec3S64 rgb;
    s64 a1;
  };

  s64 elements[4];
  s64 v[4];
};
IGNORE_WARNING_POP()

#define vec2_f32_dup(a) vec2_f32((a), (a))
#define vec3_f32_dup(a) vec3_f32((a), (a), (a))

INTERNAL Vec2F32 vec2_f32(f32 x, f32 y) { return {x, y}; }
INTERNAL Vec2F32 vec2_f32_add(Vec2F32 a, Vec2F32 b) { return vec2_f32(a.x + b.x, a.y + b.y); }
INTERNAL Vec2F32 vec2_f32_sub(Vec2F32 a, Vec2F32 b) { return vec2_f32(a.x - b.x, a.y - b.y); }
INTERNAL Vec2F32 vec2_f32_neg(Vec2F32 a) { return vec2_f32(-a.x, -a.y); }
INTERNAL Vec2F32 vec2_f32_hadamard(Vec2F32 a, Vec2F32 b) { return vec2_f32(a.x * b.x, a.y * b.y); }
INTERNAL Vec2F32 vec2_f32_mul(Vec2F32 a, f32 b) { return vec2_f32(a.x * b, a.y * b); }
INTERNAL Vec2F32 vec2_f32_div(Vec2F32 a, Vec2F32 b) { return vec2_f32(a.x / b.x, a.y / b.y); }
INTERNAL Vec2F32 vec2_f32_arm(f32 angle) { return vec2_f32(F32_COS(angle), F32_SIN(angle)); }
INTERNAL Vec2F32 vec2_f32_perp(Vec2F32 a) { return vec2_f32(-a.y, a.x); }
INTERNAL f32 vec2_f32_angle(Vec2F32 a) { return F32_ATAN2(a.y, a.x); }
INTERNAL f32 vec2_f32_dot(Vec2F32 a, Vec2F32 b) { return (a.x * b.x + a.y * b.y); }
INTERNAL f32 vec2_f32_lengthsq(Vec2F32 v) { return vec2_f32_dot(v, v); }
INTERNAL f32 vec2_f32_length(Vec2F32 v) { return F32_SQRT(vec2_f32_lengthsq(v)); }
INTERNAL Vec2F32 vec2_f32_normalise(Vec2F32 v) { return vec2_f32_mul(v, 1.0f / vec2_f32_length(v)); }
INTERNAL Vec2F32 vec2_f32_lerp(Vec2F32 a, Vec2F32 b, f32 t) { return vec2_f32(a.x * (1 - t) + (b.x * t), a.y * (1 - t) + (b.y * t)); }

#if defined(LANG_CPP)
INTERNAL Vec2F32 operator*(f32 s, Vec2F32 a) { return vec2_f32_mul(a, s); }
INTERNAL Vec2F32 operator*(Vec2F32 a, f32 s) { return vec2_f32_mul(a, s); }
INTERNAL Vec2F32 & operator*=(Vec2F32 &a, f32 s) { a = a * s; return a; } 
INTERNAL Vec2F32 operator+(Vec2F32 a, Vec2F32 b) { return vec2_f32_add(a, b); }
INTERNAL Vec2F32 & operator+=(Vec2F32 &a, Vec2F32 b) { a = a + b; return a; }
INTERNAL Vec2F32 operator-(Vec2F32 a, Vec2F32 b) { return vec2_f32_sub(a, b); }
INTERNAL Vec2F32 & operator-=(Vec2F32 &a, Vec2F32 b) { a = a - b; return a; }
INTERNAL Vec2F32 operator-(Vec2F32 a) { return vec2_f32_neg(a); }
#endif
// TODO(Ryan): Add gcc vector extensions for C

INTERNAL Vec2F32
vec2_f32_reflect(Vec2F32 incident, Vec2F32 normal)
{
  Vec2F32 result = ZERO_STRUCT;

  Vec2F32 normal_unit = vec2_f32_normalise(normal);

  result = -2.0f * (vec2_f32_dot(incident, normal_unit)) * normal_unit;

  return result;
}

INTERNAL Vec3F32 vec3_f32(f32 x, f32 y, f32 z) { return {x, y, z}; }
INTERNAL Vec3F32 vec3_f32_add(Vec3F32 a, Vec3F32 b) { return vec3_f32(a.x + b.x, a.y + b.y, a.z + b.z); }
INTERNAL Vec3F32 vec3_f32_sub(Vec3F32 a, Vec3F32 b) { return vec3_f32(a.x - b.x, a.y - b.y, a.z - b.z); }
INTERNAL Vec3F32 vec3_f32_hadamard(Vec3F32 a, Vec3F32 b) { return vec3_f32(a.x * b.x, a.y * b.y, a.z * b.z); }
INTERNAL Vec3F32 vec3_f32_div(Vec3F32 a, Vec3F32 b) { return vec3_f32(a.x / b.x, a.y / b.y, a.z / b.z); }
INTERNAL Vec3F32 vec3_f32_mul(Vec3F32 a, f32 scale) { return vec3_f32(a.x * scale, a.y * scale, a.z * scale); }
INTERNAL f32 vec3_f32_dot(Vec3F32 a, Vec3F32 b) { return (a.x * b.x + a.y * b.y + a.z * b.z); }
INTERNAL f32 vec3_f32_lengthsq(Vec3F32 v) { return vec3_f32_dot(v, v); }
INTERNAL f32 vec3_f32_length(Vec3F32 v) { return F32_SQRT(vec3_f32_lengthsq(v)); }
INTERNAL Vec3F32 vec3_f32_normalise(Vec3F32 v) { return vec3_f32_mul(v, 1.0f / vec3_f32_length(v)); }
INTERNAL Vec3F32 vec3_f32_lerp(Vec3F32 a, Vec3F32 b, f32 t) { return vec3_f32(a.x * (1 - t) + (b.x * t), a.y * (1 - t) + (b.y * t), a.z * (1 - t) + (b.z * t)); }
INTERNAL Vec3F32 vec3_f32_cross(Vec3F32 a, Vec3F32 b) { return vec3_f32(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }


INTERNAL Vec4F32 vec4_f32(f32 x, f32 y, f32 z, f32 w) { return {x, y, z, w}; }
INTERNAL Vec4F32 vec4_f32_add(Vec4F32 a, Vec4F32 b) { return vec4_f32(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
INTERNAL Vec4F32 vec4_f32_sub(Vec4F32 a, Vec4F32 b) { return vec4_f32(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
INTERNAL Vec4F32 vec4_f32_hadamard(Vec4F32 a, Vec4F32 b) { return vec4_f32(a.x * b.x, a.y * b.y, a.z * b.z, a.z * b.z); }
INTERNAL Vec4F32 vec4_f32_div(Vec4F32 a, Vec4F32 b) { return vec4_f32(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
INTERNAL Vec4F32 vec4_f32_mul(Vec4F32 a, f32 scale) { return vec4_f32(a.x * scale, a.y * scale, a.z * scale, a.w * scale); }
INTERNAL Vec4F32 vec4_f32_neg(Vec4F32 a) { return vec4_f32(-a.x, -a.y, -a.z, -a.w); }
INTERNAL f32 vec4_f32_dot(Vec4F32 a, Vec4F32 b) { return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w); }
INTERNAL f32 vec4_f32_lengthsq(Vec4F32 v) { return vec4_f32_dot(v, v); }
INTERNAL f32 vec4_f32_length(Vec4F32 v) { return F32_SQRT(vec4_f32_lengthsq(v)); }
INTERNAL Vec4F32 vec4_f32_normalise(Vec4F32 v) { return vec4_f32_mul(v, 1.0f / vec4_f32_length(v)); }
INTERNAL Vec4F32 vec4_f32_lerp(Vec4F32 a, Vec4F32 b, f32 t) { return vec4_f32(a.x * (1 - t) + (b.x * t), a.y * (1 - t) + (b.y * t), a.z * (1 - t) + (b.z * t), a.w * (1 - t) + (b.w * t)); }

INTERNAL Vec4F32
vec4_f32_whiten(Vec4F32 colour, f32 amount)
{
  Vec4F32 result = ZERO_STRUCT;

  result.r = f32_lerp(colour.r, 1.0f, amount);
  result.g = f32_lerp(colour.g, 1.0f, amount);
  result.b = f32_lerp(colour.b, 1.0f, amount);

  result.a = 1.0f;

  return result;
}

INTERNAL Vec4F32
vec4_f32_darken(Vec4F32 colour, f32 amount)
{
  Vec4F32 result = ZERO_STRUCT;

  result.r = f32_lerp(colour.r, 0.0f, amount);
  result.g = f32_lerp(colour.g, 0.0f, amount);
  result.b = f32_lerp(colour.b, 0.0f, amount);

  result.a = 1.0f;

  return result;
}

#if defined(LANG_CPP)
INTERNAL Vec4F32 operator*(f32 s, Vec4F32 a) { return vec4_f32_mul(a, s); }
INTERNAL Vec4F32 operator*(Vec4F32 a, f32 s) { return vec4_f32_mul(a, s); }
INTERNAL Vec4F32 & operator*=(Vec4F32 &a, f32 s) { a = a * s; return a; } 
INTERNAL Vec4F32 operator+(Vec4F32 a, Vec4F32 b) { return vec4_f32_add(a, b); }
INTERNAL Vec4F32 & operator+=(Vec4F32 &a, Vec4F32 b) { a = a + b; return a; }
INTERNAL Vec4F32 operator-(Vec4F32 a, Vec4F32 b) { return vec4_f32_sub(a, b); }
INTERNAL Vec4F32 & operator-=(Vec4F32 &a, Vec4F32 b) { a = a - b; return a; } 
INTERNAL Vec4F32 operator-(Vec4F32 a) { return vec4_f32_neg(a); }
#endif
// TODO(Ryan): Add gcc vector extensions for C


INTERNAL Vec2S32 vec2_s32(s32 x, s32 y) { return {x, y}; }
INTERNAL Vec2S64 vec2_s64(s64 x, s64 y) { return {x, y}; }

INTERNAL u32
u32_pack_4x8(Vec4F32 val)
{
  u32 result = 0;

  result = (F32_ROUND_U32(val.x) << 24 |
            F32_ROUND_U32(val.y) << 16 & 0xFF0000 |
            F32_ROUND_U32(val.z) << 8 & 0xFF00 |
            F32_ROUND_U32(val.w) & 0xFF);

  return result;
}

// TODO(Ryan): Rename to rect
IGNORE_WARNING_PEDANTIC()
typedef union Range2F32 Range2F32;
union Range2F32
{
  struct
  {
    Vec2F32 min, max;
  };

  struct
  {
    Vec2F32 p0, p1;
  };

  struct
  {
    f32 x0, y0, x1, y1;
  };
};
IGNORE_WARNING_POP()

INTERNAL Range2F32
range2_f32(Vec2F32 min, Vec2F32 max)
{
  Range2F32 result = {min, max};
  return result;
}

INTERNAL Range2F32 
range2_f32_shift(Range2F32 r, Vec2F32 v) 
{ 
  r.x0 += v.x; 
  r.y0 += v.y; 
  r.x1 += v.x; 
  r.y1 += v.y; 

  return r; 
}

INTERNAL Range2F32 
range2_f32_pad(Range2F32 r, f32 x) 
{ 
  Vec2F32 min = vec2_f32_sub(r.min, vec2_f32(x, x));
  Vec2F32 max = vec2_f32_add(r.max, vec2_f32(x, x));

  return range2_f32(min, max); 
}

INTERNAL Vec2F32 
range2_f32_centre(Range2F32 r) 
{ 
  return vec2_f32((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2); 
}

INTERNAL b32 
range2_f32_contains(Range2F32 r, Vec2F32 v) 
{ 
  return (r.min.x <= v.x && v.x <= r.max.x) && (r.min.y <= v.y && v.y <= r.max.y); 
}

INTERNAL Vec2F32 
range2_f32_dim(Range2F32 r) 
{ 
  return vec2_f32(f32_abs(r.max.x - r.min.x), f32_abs(r.max.y - r.min.y)); 
}
