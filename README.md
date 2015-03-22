<img align="right" src="https://github.com/MarkusIppy/raspexi-viewer-linux/blob/master/images/raspexi-logo_v2.png?raw=true" width="366" height="91" />
#<big>__Raspexi__<br></br></big>
<big>_A'PEXi Power FC Interface for Raspberry Pi_</big>




Summary
-------

Digital Gauge Display for A'PEXi Power FC which runs on Raspberry Pi. Raspexi is capable of communicating with the Power FC to retrieve diagnostic information and display the information in the form of aesthetic guages and dashboards. Raspexi has been tested for Mazda Power FC's and is currently in beta testing for Nissan, Subaru and Toyota Power FC's.

The Gauges and Dashboards used in this software are adapted from <a href="https://github.com/djandruczyk/MegaTunix">__MegaTunix__</a> by _David J. Andruczyk_<br>
The A'PEXi Serial interface is adapted from <a href="http://kaele.com/~kashima/car/pfcadp/FCLoggerFD3S.xls">__FCLogger__</a> by _Hitoshi Kashima_


Installation
------------
#### Pre-Requisite

Rasberry Pi with Raspbian OS. Tested versions:<br>
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
analog_eq1 = 1.4 * (AUX1 - AUX2) + 9.0
analog_eq2 = 0.5*AUX3+2.5
csvfile = /home/pi/raspexi/log/raspexi.csv
```

* port ==> Serial port device name

* baud ==> Serial port baud rate

* interval ==> Data refresh interval

* model ==> Mazda, Nissan, Subaru, Toyota

* dash1, dash2, dash3, dash4 ==> Dashboard XML file

* analog_eq1, analog_eq2, analog_eq3, analog_eq4 ==> Analog equations for auxiliary input relations. (See section _'Analog Auxiliary Input Relationship Equations'_ for more details)

* csvfile ==> CSV log output file

*Note: If the 'csvfile' location is set to the `/tmp` directory then it will be removed by
the system on (re)start*

*Note: The 'model' setting is in testing and has only been confirmed for Mazda (March/2015)*


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

*Note: The current version is in testing for support for vehicles other than Mazda (March/2015)* 

Description										|Datasource Name		|Mazda			|Nissan			|Subaru			|Toyota
------------------------------------------------|:---------------------:|:-------------:|:-------------:|:-------------:|:-------------:
_Engine Speed (rpm)_							|__`RPM`__				|<big>__✓__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__		
_Absolute Intake Pressure (Kg/cm2)_				|__`Intakepress`__		|<big>__✓__		|				|				|<big>__✓*__
_Pressure Sensor Voltage (mv)_					|__`PressureV`__		|<big>__✓__		|				|				|<big>__✓*__
_Engine Load (N)_								|__`EngLoad`__			|				|<big>__✓*__	|<big>__✓*__	|
_Mass Flow Sensor #1 (mv)_						|__`MAF1V`__			|				|<big>__✓*__	|<big>__✓*__	|
_Mass Flow Sensor #2 (mv)_						|__`MAF2V`__			|				|<big>__✓*__	|<big>__✓*__	|
_Throttle Sensor Voltage (mv)_					|__`ThrottleV`__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Throttle Sensor #2 Voltage(mv)_				|__`ThrottleV_2`__		|				|				|				|<big>__✓*__
_Primary Injector Pulse Width (mSec)_          	|__`Primaryinp`__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Fuel correction_                              	|__`Fuelc`__			|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Leading Ignition Angle (deg)_                 	|__`Leadingign`__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Trailing Ignition Angle (deg)_                	|__`Trailingign`__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Fuel Temperature (deg.C)_                     	|__`Fueltemp`__			|<big>__✓__		|				|				|
_Metering Oil Pump Duty (%)_                    |__`Moilp`__			|<big>__✓__		|				|				|
_Boost Duty (TP.%)_                             |__`Boosttp`__			|<big>__✓__		|				|				|				|<font color="red">These are</font>
_Boost Duty (Wg,%)_                             |__`Boostwg`__			|<big>__✓__		|				|				|				|<font color="red">to be combined</font>
_Boost Pressure (PSI)_							|__`BoostPres`__		|				|<big>__✓*__	|<big>__✓*__	|<big>__✓*__	|<font color="red">in a future</font>
_Boost Duty (%)_								|__`BoostDuty`__		|				|<big>__✓*__	|<big>__✓*__	|<big>__✓*__	|<font color="red">release.</font>
_Water Temperature (deg. C)_                    |__`Watertemp`__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Intake Air Temperature (deg C)_                |__`Intaketemp`__		|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Knocking Level_                                |__`Knock`__			|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Battery Voltage (V)_                           |__`BatteryV`__			|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_Vehicle Speed (Km/h)_                          |__`Speed`__			|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_ISCV duty (%)_                                 |__`Iscvduty`__			|<big>__✓__		|				|				|<big>__✓*__
_Mass Air Flow sensor activity ratio (%)_		|__`MAFactivity`__		|				|<big>__✓*__	|<big>__✓*__	|		
_O2 Sensor Voltage (mv)_                       	|__`O2volt`__			|<big>__✓__		|<big>__✓*__	|<big>__✓*__	|<big>__✓*__
_O2 Sensor #2 Voltage (mV)_						|__`O2volt_2`__			|				|<big>__✓*__	|<big>__✓*__	|		
_Secondary Injector Pulse Width (mSec)_        	|__`Secinjpulse`__		|<big>__✓__		|				|				|
_Suction In Air Temperature (mV)_				|__`SuctionAirTemp`__	|				|				|				|<big>__✓*__
_Analog Auxiliary Input #1_                     |__`AUX1`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #2_                     |__`AUX2`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #3_                     |__`AUX3`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #4_                   	|__`AUX4`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #5_                    	|__`AUX5`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #6_                  	|__`AUX6`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #7_                    	|__`AUX7`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Analog Auxiliary Input #8_                   	|__`AUX8`__				|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Result of Analog Equation #1_                 	|__`Analog1`__			|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Result of Analog Equation #2_                 	|__`Analog2`__			|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Result of Analog Equation #3_                 	|__`Analog3`__			|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
_Result of Analog Equation #4_                 	|__`Analog4`__			|<big>__✓__		|<big>__✓__		|<big>__✓__		|<big>__✓__
__Coming soon ...__   			              	|
_Instantaneous vehicle power (kW)_				|__`Power`__			|<big>			|<big>			|<big>			|<big>
_Vehicle Acceleration (100km/h/s)_				|__`Accel`__			|<big>			|<big>			|<big>			|<big>
_Force exerted in Newtons (N)_					|__`ForceN`__			|<big>			|<big>			|<big>			|<big>
_Force exerted in G-force (G)_					|__`ForceG`__			|<big>			|<big>			|<big>			|<big>
<big>__*__</big> Implemented but untested


Analog Auxiliary Input Relationship Equations
----------------------------
It may not be convenient to view __`AUX1`__, __`AUX2`__, etc, in a gauge as is. Therefore we have implemented a simple linear equation parser for the `raspexi.cfg` file. The linear equations can be represented in the form _'y=m*x+b'_. Here, the result _'y'_ is stored in either __`Analog1`__, __`Analog2`__, __`Analog3`__ or __`Analog4`__ depending on the definition in `raspexi.cfg`, _'m'_ is a multiplier, _'x'_ is an auxiliary input (__`AUX1`__, __`AUX2`__, etc) and _'b'_ is an adder.

Two examples are as follows:

* For _'y'_ = __`Analog1`__, _'m'_ = `1.4`, _'x'_ = __`AUX1 - AUX2`__ and _'b'_ = `9.0`. The resulting line in `raspexi.cfg` could be `analog_eq1 = 1.4 * (AUX1 - AUX2) + 9.0`.

* For _'y'_ = __`Analog2`__, _'m'_ = `0.5`, _'x'_ = __`AUX3`__ and _'b'_ = `2.5`. The resulting line in `raspexi.cfg` could be `analog_eq2 = 0.5*AUX3+2.5`.

*Note: The multiplication, subtraction and addition cannot be changed and spaces in the equation are ignored.*

Custom Gauges and Dashboards
----------------------------
You can create your very own custom gauges and dashboards using __*MegaTunix*__ which you can download [__compiled here__](http://sourceforge.net/projects/megatunix/) or the [__source here__](https://github.com/djandruczyk/MegaTunix).

To create your own __Gauges__ you can use the __`MtxGaugeDesigner`__.<br>
To create your own __Dashboards__ (which can include custom gauges) you can use the __`MtxDashDesigner`__.

Once you have saved the custom Gauges and Dashboards in the XML file format you will need to make sure that the XML field element `<datasource>` contains the appropriate _'Datasource Name'_ from the table above for each `<gauge>` in the dashboard XML file.


Development
-----------
1. Download or clone the original MegaTunix source code using:
  ```
  $ git clone https://github.com/djandruczyk/MegaTunix
  ```
2. Run the following to download __MegaTunix__ and install its dependencies (it may take a while to download and install these (>10mins)):
   ```
   $ cd ~/
   $ sudo apt-get install pkg-config libtool intltool libgtkgl2.0-dev libgtkglext1-dev g++ gcc flex bison glade libglade2-dev make git-core gdb automake1.9
   $ git clone git://github.com/djandruczyk/MegaTunix.git MegaTunix
   ```
   A good step by step installation guide for MegaTunix can be found <a href="http://www.msextra.com/forums/viewtopic.php?t=23548">__here__</a>.
  (See [`README.md`](https://github.com/djandruczyk/MegaTunix/blob/master/README.md) in MegaTunix for more detail)

3. Download or clone __Raspexi__ source code with:

  ```
  $ cd ~/MegaTunix
  $ git clone https://github.com/MarkusIppy/raspexi-viewer-linux raspexi
  ```

4. Add the exact line `raspexi/Makefile` to MegaTunix AutoConf configuration file (`configure.ac`)
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
  
   If you would like to install MegaTunix so that you can create your own dashboards and gauges on the Raspberry Pi then run `sudo make install` and `sudo ldconfig`.
  
6. Build Raspexi

  ```
  $ cd ~/MegaTunix/raspexi
  $ sudo chmod +x compile.sh
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
__R6__		|15/03/2015		|<ul><li>Added support for Nissan, Subaru and Toyota (by JacobD)</li><li>Added CSV log file error handling (by JacobD)</li><li>Added XML dashboard file incorrect '_datasource_' error handling (by JacobD)</li><li>Added linear equations to define the auxiliary relationships from the config file (by JacobD)</li></ul>
__R5__		|07/07/2014		|<ul><li>Implementation of auxiliary inputs AUX1-AUX8 (by SonicRaT)</li></ul>
__R4__		|07/05/2014		|<ul><li>Revising and refactoring for public release (Google Code)</li></ul>
__R3__		|18/04/2014 	|<ul><li>Implement multiple dash board (up to 4), can be switch by key 1/2/3/4</li><li>Full screen on start</li><li>Data save to CSV file</li></ul>
__R2__		|05/04/2014		|<ul><li>Implement PowerFC RS-232 protocol (based on fclogger.py)</li><li>Add configuration file (raspexi.cfg)</li><li>Fix issue Gauges data location</li></ul>
__R1__		|31/03/2014		|<ul><li>Initial release</li></ul>
