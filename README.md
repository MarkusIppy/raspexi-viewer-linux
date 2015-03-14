Apexi Power FC Interface for Raspberry Pi (Raspexi Viewer)
=======================================================

Summary
-------

Digital Gauge Display for Apexi Power FC which runs on Raspberry Pi

This program is based on the source code of Gauges and Dashboard adapted from:

David J. Andruczyk https://github.com/djandruczyk/MegaTunix

Apexi Serial interface adapted from:

Hitoshi Kashima http://kaele.com/~kashima/car/pfcadp/FCLoggerFD3S.xls

History
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

Pre-Requisite
------------
Rasberry Pi with Raspbian OS (Tested)
- 2013-05-25-wheezy-raspbian
- 2013-09-25-wheezy-raspbian
- 2014-01-07-wheezy-raspbian


Directories & Files
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

Installation
------------
Extract the archive `raspexi-yyyymmdd.tar.gz` to `/home/pi`

```
$ cd /home/pi
$ tar xvzf raspexi-yyyymmdd.tar.gz
```

Configuration
-------------
There are a few configuration variables in `raspexi.cfg` :-

```
[default]
port = /dev/ttyUSB0
baud = 57600,8,n,1
interval = 35
model = Mazda
dash1 = Analogue_Dash_1280_720.xml
dash2 = Commander_1280_720.xml
dash3 = Digital_Dash_1280_720.xml
dash4 = Race_1280_720.xml
csvfile = /home/pi/raspexi/log/raspexi.csv
```

* port ==> Serial port device name

* baud ==> Serial port baud rate

* interval ==> Data refresh interval

* model ==> Mazda, Nissan, Subaru, Toyota

* dash1, dash2, dash3, dash4 ==> Dashboard XML file

* csvfile ==> CSV output file

*NOTE: If we have set csvfile location to `/tmp` it will be remove by
system on (re)start*

*NOTE: The 'model' configuration value is in testing and is not fully supported yet (9/March/2015)*

How To Run
----------
You can run this release with the following command :-

```
$ cd /home/pi/raspexi
$ ./run.sh
```

Gauges Datasource
-----------------
The following are inputs and theirs name can use in Dashboard:

*(NOTE: the current version is in testing for support for multiple vehicles (March/2015), the datasources below are for Mazda only.
Please refer to powerfc.h file for data sources for other vehicles, these should match the datasource given in the dashboard XML file)*

Value											|Datasource Name	|Mazda	|Nissan	|Subaru	|Toyota
------------------------------------------------|-------------------|-------|-------|-------|------
Engine Speed (rpm)								|RPM				|✓		|✓		|✓		|✓		
Absolute Intake Pressure (Kg/cm2)				|Intakepress		|✓		|		|		|✓
Pressure Sensor Voltage (mv)					|PressureV			|✓		|		|		|✓
Engine Load (N)									|EngLoad			|		|✓		|✓		|
Mass Flow Sensor #1 (mv)						|MAF1V				|		|✓		|✓		|
Mass Flow Sensor #2 (mv)						|MAF2V				|		|✓		|✓		|
Throttle Sensor Voltage (mv)					|ThrottleV			|✓		|✓		|✓		|✓
Throttle Sensor #2 Voltage(mv)					|ThrottleV_2		|		|		|		|✓
Primary Injector Pulse Width (mSec)            	|Primaryinp			|✓		|✓		|✓		|✓
Fuel correction                                	|Fuelc				|✓		|✓		|✓		|✓
Leading Ignition Angle (deg)                   	|Leadingign			|✓		|✓		|✓		|✓
Trailing Ignition Angle (deg)                  	|Trailingign		|✓		|✓		|✓		|✓
Fuel Temperature (deg.C)                       	|Fueltemp			|✓		|		|		|
Metering Oil Pump Duty (%)                     	|Moilp				|✓		|		|		|
Boost Duty (TP.%)                              	|Boosttp			|✓		|		|		|	|These are
Boost Duty (Wg,%)                              	|Boostwg			|✓		|		|		|	|to be combined
Boost Pressure (PSI)							|BoostPres			|		|✓		|✓		|✓	|in a future
Boost Duty (%)									|BoostDuty			|		|✓		|✓		|✓	|release.
Water Temperature (deg. C)                     	|Watertemp			|✓		|✓		|✓		|✓
Intake Air Temperature (deg C)                 	|Intaketemp			|✓		|✓		|✓		|✓
Knocking Level                                 	|Knock				|✓		|✓		|✓		|✓
Battery Voltage (V)                            	|BatteryV			|✓		|✓		|✓		|✓
Vehicle Speed (Km/h)                           	|Speed				|✓		|✓		|✓		|✓
ISCV duty (%)                                  	|Iscvduty			|✓		|		|		|✓
Mass Air Flow sensor activity ratio (%)			|MAFactivity		|		|✓		|✓		|		
O2 Sensor Voltage (mv)                         	|O2volt				|✓		|✓		|✓		|✓
O2 Sensor #2 Voltage (mV),"						|O2volt_2			|		|✓		|✓		|		
Secondary Injector Pulse Width (mSec)          	|Secinjpulse		|✓		|		|		|
Suction In Air Temperature (mV)					|SuctionAirTemp		|		|		|		|✓

Development
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
  $ ./compile.sh
  ```
    
- Binary package for deployment
  ```
  $ ./package.sh
  ```
  the binary package `raspexi-yyyymmdd.tar.gz` will be created.
