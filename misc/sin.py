#!/usr/bin/python3
# SPDX-License-Identifier: zlib-acknowledgement

import math

wave = [math.sin(2 * math.pi * 4 * (i/50)) for i in range(51)]

wave_w = 50

for i in wave:
  w = ((i + 1) / 2) * wave_w
  [print(" ", end="") for j in range(int(w))]
  print("*")

