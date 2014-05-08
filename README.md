Apexi Power FC Interface for RX-7 FD3S (Raspexi Viewer)
=======================================================

Summary
-------

Digital Gauge Display for Mazda RX-7 FD3S and Apexi Power FC

This program is based on the source code of Gauges and Dashboard adapted from:

David J. Andruczyk https://github.com/djandruczyk/MegaTunix

Apexi Serial interface adapted from:

Hitoshi Kashima http://kaele.com/~kashima/car/pfcadp/FCLoggerFD3S.xls

HISTORY
-------

2014/05/07 r4
- Revising and refactoring for public release (Google Code)

2014/04/18 r3
- Implement multiple dash board (up to 4), can be switch by key 1/2/3/4
- Full screen on start
- Data save to CSV file
 
2014/04/05 r2
- Implement PowerFC RS-232 protocol (based on fclogger.py)
- Add configuration file (raspexi.cfg)
- Fix issue Gauges data location

2014/03/31 r1
- Initial release

PREREQUISITE
------------
Rasberry Pi with Raspbian OS (Tested)
- 2013-05-25-wheezy-raspbian
- 2013-09-25-wheezy-raspbian
- 2014-01-07-wheezy-raspbian


DIRECTORIES & FILES
-------------------
```
├── Dashboards                          ==> Dashboards directory
│ ├── FD3S_Raspexi.xml                ==> Raspexi Dashboard #1
│ ├── FD3S_Raspexi_2.xml              ==> Raspexi Dashboard #12
│ ├── ... 
│
├── Gauges                              ==> Gauges used by Dashboards
│ ├── FD3S                            ==> Raspexi Gauges
│ ├── ... 
│
├── libs                                ==> GTK and MegaTunix libraries 
│ ├── libGLU.so.1
│ ├── libgtkgl-2.0.so.1
│ ├── libmtxcommon.so.0
│ └── libmtxgauge.so.0
├── raspexi                             ==> Main executable file
├── raspexi.cfg                         ==> Configuration file
├── README.md                           ==> This file
└── run.sh                              ==> Run script
```

INSTALLATION
------------
Extract the archive "raspexi-yyyymmdd.tar.gz" to "/home/pi"

```
$ cd /home/pi
$ tar xvzf raspexi-yyyymmdd.tar.gz
```

CONFIGURATION
-------------
There are a few configuration variables in this release :-

```
[default]
port = /dev/ttyUSB0
baud = 19200,8,e,1
interval = 200
dash1 = FD3S_Raspexi.xml
dash2 = FD3S_Raspexi_2.xml
dash3 = FD3S_Raspexi_3.xml
dash4 = FD3S_Raspexi_4.xml
csvfile = /tmp/raspexi.csv
```

port ==> Serial port device name

baud ==> Serial port baud rate

interval ==> Data refresh interval

dash1, dash2, dash3, dash4 ==> Dashboard XML file

csvfile ==> CSV output file

*NOTE THAT if we have set csvfile location to `/tmp` it will be remove by
system on (re)start*

HOW TO RUN
----------
You can run this release with the following command :-

```
$ cd /home/pi/raspexi
$ ./run.sh
```

GAUGES DATASOURCE
-----------------
The following are inputs and theirs name can use in Dashboard:

Input                                           -Name to be defined

-Engine Speed (rpm)                             -RPM

-Absolute Intake Pressure (Kg/cm2)              -Intakepress   

-Pressure Sensor Voltage (mv)                   -PressureV   

-Throttle Sensor Voltage (mv)                   -ThrottleV

-Primary Injector Pulse Width (mSec)            -Primaryinp

-Fuel correction                                -Fuelc

-Leading Ignition Angle (deg)                   -Leadingign

-Trailing Ignition Angle (deg)                  -Trailingign

-Fuel Temperature (deg.C)                       -Fueltemp

-Metering Oil Pump Duty (%)                     -Moilp

-Boost Duty (TP.%)                              -Boosttp

-Boost Duty (Wg,%)                              -Boostwg

-Water Temperature (deg. C)                     -Watertemp

-Intake Air Temperature (deg C)                 -Intaketemp

-Knocking Level                                 -Knock

-Battery Voltage (V)                            -BatteryV

-Vehicle Speed (Km/h)                           -Speed

-ISCV duty (%)                                  -Iscvduty

-O2 Sensor Voltage (mv)                         -O2volt

-Secondary Injector Pulse Width (mSec)          -Secinjpulse

DEVELOPMENT
-----------
- Download or clone the original MegaTunix source code from :-
    ```
	$ git clone https://github.com/djandruczyk/MegaTunix
    ```
- Setting up all dependencies and build environment for MegaTunix
  (See `README.md` in MegaTunix for more detail)

- Download or clone Raspexi source code from :-

	```
	$ cd MegaTunix
	$ git clone https://code.google.com/p/raspexi-viewer-linux/ raspexi
	```

- Add `raspexi/Makefile` to MegaTunix AutoConf configuration file (`configure.ac`)
  or apply the following patch.

    ```
    diff --git a/configure.ac b/configure.ac
    index e868b1b..9bec24f 100644
    --- a/configure.ac
    +++ b/configure.ac
    @@ -311,6 +311,7 @@ MegaTunix32_dbg.iss
     MegaTunix64_dbg.iss
     Doxyfile
     WIN_NOTES.txt
    +raspexi/Makefile
     ])
     AC_OUTPUT
    ```

- Execute `autogen.sh` to generate build configuration.

    ```
	$ ./autogen.sh CPPFLAGS="-UDATA_DIR -DDATA_DIR=\\\"./\\\""
    ```
    
  Command line option `CPPFLAGS="-UDATA_DIR -DDATA_DIR=\\\"./\\\""`
  is need to override data directory (ex. `Dashboards`, `Gauges`, ...)
  of MegaTunix to working directory.
  
- Build Raspexi

    ```
	$ cd raspexi
	$ make
    ```
    
- Binary package for deployment
    ```
	$ package.sh
    ```
  the binary package `raspexi-yyyymmdd.tar.gz` will be created.
