#!/bin/bash
###############################################################################
#
# Copyright (C) 2014 <COPYRIGHT>
#
# Digital Gauges for Apexi Power FC for RX7 on Raspberry Pi 
# 
# 
# This software comes under the GPL (GNU Public License)
# You may freely copy,distribute etc. this as long as the source code
# is made available for FREE.
# 
# No warranty is made or implied. You use this program at your own risk.
#
###############################################################################

cd /home/pi/raspexi
export LD_LIBRARY_PATH=./libs:$LD_LIBRARY_PATH
./raspexi
