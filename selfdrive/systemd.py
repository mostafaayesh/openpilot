#!/usr/bin/env python3
# The MIT License
#
# Copyright (c) 2019-, Rick Lan, dragonpilot community, and a number of other of contributors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import threading
import time

import cereal.messaging as messaging
from common.realtime import Ratekeeper

from selfdrive.dashcamd.dashcamd import Dashcamd
from selfdrive.gpxd.gpx_uploader import gpx_uploader_thread

HERTZ = 1

def confd_thread():
  sm = messaging.SubMaster(['deviceState'])

  frame = 0
  started = False
  free_space = 1
  dashcamd = Dashcamd()

  rk = Ratekeeper(HERTZ, print_delay_threshold=None)  # Keeps rate at 2 hz
  uploader_thread = None

  while True:
    if uploader_thread is None:
      uploader_thread = threading.Thread(target=gpx_uploader_thread)
      uploader_thread.start()

    if frame % (HERTZ * 3) == 0:
      sm.update(0)
      if sm.updated['deviceState']:
        started = sm['deviceState'].started
        free_space = sm['deviceState'].freeSpacePercent
    
    if frame % HERTZ == 0:
      dashcamd.run(started, free_space)
    
    frame += 1
    rk.keep_time()

def main():
  confd_thread()

if __name__ == "__main__":
  main()