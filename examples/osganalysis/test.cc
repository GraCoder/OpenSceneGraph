#pragma once

#include <stdio.h>
#include <osg/Vec3d>
#include <osg/Vec3>
#include "tvec.h"

using namespace tg;

vec3 rgb2YCoCg(vec3 rgb) {
  vec3 ycocg;
  ycocg.y() = (rgb.x() - rgb.z()) * 2;
  float tmp = rgb.x() + rgb.z();
  ycocg.z() = rgb.y() * 2 - tmp;
  ycocg.x() = ycocg.z() + tmp + tmp;
  return ycocg;
}

vec3 YCoCg2rgb(vec3 ycocg) {
  vec3 tmp = ycocg * 0.25;
  return vec3(tmp.x() + tmp.y() - tmp.z(), tmp.x() + tmp.z(), tmp.x() - tmp.y() - tmp.z());
}

float luminace(vec3 color) {
  return 0.25 * color.x() + 0.5 * color.y() + 0.25 * color.z();
}

vec3 tone_map(vec3 color) {
  return color / (1 + luminace(color));
}

vec3 untone_map(vec3 color) {
  return color / (1 - luminace(color));
}

void test() {
  auto v1 = rgb2YCoCg(vec3(0.7, 0.5, 0.3));
  auto v3 = tone_map(v1);
  auto v4 = untone_map(v3);
  auto v2 = YCoCg2rgb(v1);
  printf("");
}