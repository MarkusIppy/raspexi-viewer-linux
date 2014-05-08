#!/bin/sh
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

echo "Package Raspexi Viewer for deployment..."
rm -rf dist
mkdir dist
mkdir dist/libs
cp -f .libs/raspexi dist/raspexi
cp -rf Dashboards dist/
cp -rf Gauges dist/
cp -f README.md dist/
cp -f LICENSE dist/
cp -f AUTHORS dist/
cp -f raspexi.cfg dist/
cp -f run.sh dist/
cp -Lf ../mtxcommon/.libs/libmtxcommon.so.0 dist/libs
cp -Lf ../widgets/.libs/libmtxgauge.so.0 dist/libs
cp -Lf /usr/lib/arm-linux-gnueabihf/libGLU.so.1 dist/libs
cp -Lf /usr/lib/libgtkgl-2.0.so.1 dist/libs
tar -C dist -czf raspexi-`date +"%Y%m%d"`.tar.gz .
echo "DONE"