#<big>__Raspexi__</big>		<img align="right" src="https://www.dropbox.com/s/uhp53ih4mcmsfmi/raspexi-logo_v2.png?dl=1" width="407" height="101" />

### _A'PEXi Power FC Interface for Raspberry Pi _


Summary
-------

Digital Gauge Display for A'PEXi Power FC which runs on Raspberry Pi. Raspexi is capable of communicating with the Power FC to retrieve diagnostic information and display the information in the form of aesthetic guages and dashboards. Raspexi has been tested for Mazda Power FC's and is currently in beta testing for Nissan, Subaru and Toyota Power FC's.

The Gauges and Dashboards used in this software are adapted from:

* <a href="https://github.com/djandruczyk/MegaTunix">__MegaTunix__</a> by _David J. Andruczyk_

The A'PEXi Serial interface is adapted from:

* <a href="http://kaele.com/~kashima/car/pfcadp/FCLoggerFD3S.xls">__FCLogger__</a> by _Hitoshi Kashima_


Installation
------------
#### Pre-Requisite

Rasberry Pi with Raspbian OS. Tested versions:

* 2013-05-25-wheezy-raspbian
* 2013-09-25-wheezy-raspbian
* 2014-01-07-wheezy-raspbian

#### Install
Extract the archive `raspexi-yyyymmdd.tar.gz` to `/home/pi`:

```
$ cd /home/pi
$ tar xvzf raspexi-yyyymmdd.tar.gz
```


Configuration
-------------
There are a few configurable settings in `raspexi.cfg`:

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

* csvfile ==> CSV log output file

*NOTE: If the 'csvfile' location is set to the `/tmp` directory then it will be removed by
the system on (re)start*

*NOTE: The 'model' setting is in testing and has only been confirmed for Mazda (March/2015)*


How To Run
----------
You can run this release with the following command:

```
$ cd /home/pi/raspexi
$ ./run.sh
```


Gauge Datasources
-----------------
The following _Descriptions_ are for information which the Power FC will return for a given model. The _Datasource Name_ can be used in the dashboard XML files to define what information a particular gauge will display:

*NOTE: The current version is in testing for support for vehicles other than Mazda (March/2015) * 

<p>Description</p>								|<p>Datasource Name</p>	|<p>Mazda</p>	|<p>Nissan</p>	|<p>Subaru</p>	|<p>Toyota</p>
------------------------------------------------|:---------------------:|:-------------:|:-------------:|:-------------:|:-------------:
_Engine Speed (rpm)							_	|__RPM				__	|<big>__✓__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__		
_Absolute Intake Pressure (Kg/cm2)			_	|__Intakepress		__	|<big>__✓__		|				|				|<big>__✓*__
_Pressure Sensor Voltage (mv)				_	|__PressureV		__	|<big>__✓__		|				|				|<big>__✓*__
_Engine Load (N)							_	|__EngLoad			__	|				|<big>__✓*__	|<big>__✓*__	|
_Mass Flow Sensor #1 (mv)					_	|__MAF1V			__	|				|<big>__✓*__	|<big>__✓*__	|
_Mass Flow Sensor #2 (mv)					_	|__MAF2V			__	|				|<big>__✓*__	|<big>__✓*__	|
_Throttle Sensor Voltage (mv)				_	|__ThrottleV		__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Throttle Sensor #2 Voltage(mv)				_	|__ThrottleV_2		__	|				|				|				|<big>__✓*__
_Primary Injector Pulse Width (mSec)        _  	|__Primaryinp		__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Fuel correction                            _  	|__Fuelc			__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Leading Ignition Angle (deg)               _  	|__Leadingign		__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Trailing Ignition Angle (deg)              _  	|__Trailingign		__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Fuel Temperature (deg.C)                   _  	|__Fueltemp			__	|<big>__✓__		|				|				|
_Metering Oil Pump Duty (%)                 _   |__Moilp			__	|<big>__✓__		|				|				|
_Boost Duty (TP.%)                          _   |__Boosttp			__	|<big>__✓__		|				|				|				|<font color="red">These are</font>
_Boost Duty (Wg,%)                          _   |__Boostwg			__	|<big>__✓__		|				|				|				|<font color="red">to be combined</font>
_Boost Pressure (PSI)						_	|__BoostPres		__	|				|<big>__✓*__	|<big>__✓*__	|<big>__✓*__	|<font color="red">in a future</font>
_Boost Duty (%)								_	|__BoostDuty		__	|				|<big>__✓*__	|<big>__✓*__	|<big>__✓*__	|<font color="red">release.</font>
_Water Temperature (deg. C)                 _   |__Watertemp		__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Intake Air Temperature (deg C)             _   |__Intaketemp		__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Knocking Level                             _   |__Knock			__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Battery Voltage (V)                        _   |__BatteryV			__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Vehicle Speed (Km/h)                       _   |__Speed			__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_ISCV duty (%)                              _   |__Iscvduty			__	|<big>__✓__		|				|				|<big>__✓*__
_Mass Air Flow sensor activity ratio (%)	_	|__MAFactivity		__	|				|<big>__✓*__	|<big>__✓*__	|		
_O2 Sensor Voltage (mv)                     _  	|__O2volt			__	|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_O2 Sensor #2 Voltage (mV),"				_	|__O2volt_2			__	|				|<big>__✓*__	|<big>__✓*__	|		
_Secondary Injector Pulse Width (mSec)      _  	|__Secinjpulse		__	|<big>__✓__		|				|				|
_Suction In Air Temperature (mV)			_	|__SuctionAirTemp	__	|				|				|				|<big>__✓*__
_Analog Auxilary Input #1                   _   |__AUX1				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #2                   _   |__AUX2				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #3                   _   |__AUX3				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #4                   _	|__AUX4				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #5                   _  	|__AUX5				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #6                   _	|__AUX6				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #7                   _  	|__AUX7				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxilary Input #8                   _	|__AUX8				__	|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
<big>__*__</big> Implemented but untested

Development
-----------
1. Download or clone the original MegaTunix source code using:
  ```
  $ git clone https://github.com/djandruczyk/MegaTunix
  ```
2. Set up all dependencies and build MegaTunix 
  (See [`README.md`](https://github.com/djandruczyk/MegaTunix/blob/master/README.md) in MegaTunix for more detail)

3. Download or clone Raspexi source code from :-

  ```
  $ cd MegaTunix
  $ git clone https://github.com/MarkusIppy/raspexi-viewer-linux raspexi
  ```

4. Add `raspexi/Makefile` to MegaTunix AutoConf configuration file (`configure.ac`)
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

5. Execute `autogen.sh` to generate build configuration.

  ```
  $ ./autogen.sh CPPFLAGS="-UDATA_DIR -DDATA_DIR=\\\"./\\\""
  ```
    
  Command line option `CPPFLAGS="-UDATA_DIR -DDATA_DIR=\\\"./\\\""`
  is need to override data directory (ex. `/Dashboards`, `/Gauges`, ...)
  of MegaTunix to working directory.
  
6. Build Raspexi

  ```
  $ cd raspexi
  $ ./compile.sh
  ```
    
7. To build a binary package for deployment:
  ```
  $ ./package.sh
  ```
  The binary package `raspexi-yyyymmdd.tar.gz` will then be created.


History
-------
Revision	|Date (d/m/y)	|Notes
:----------:|:-------------:|------
__R6__		|09/03/2015		|<li>Added support for Nissan, Subaru and Toyota (by JacobD10)</li><li>Added CSV log file error handling (by JacobD10)</li><li>Added XML dashboard file incorrect '_datasource_' error handling (by JacobD10)</li>
__R5__		|07/07/2014		|<li>Implementation of Auxillary inputs AUX1-AUX8 (by SonicRaT)</li>
__R4__		|07/05/2014		|<li>Revising and refactoring for public release (Google Code)</li>
__R3__		|18/04/2014 	|<li>Implement multiple dash board (up to 4), can be switch by key 1/2/3/4</li><li>Full screen on start</li><li>Data save to CSV file</li>
__R2__		|05/04/2014		|<li>Implement PowerFC RS-232 protocol (based on fclogger.py)</li><li>Add configuration file (raspexi.cfg)</li><li>Fix issue Gauges data location</li>
__R1__		|31/03/2014		|<li>Initial release</li>
