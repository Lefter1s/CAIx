CAIx-qt: Qt5 GUI for CAIx
===============================

Build instructions
===================
You can find build instructions for Windows, OS X and Unix in the documents build-windows.md, build-osx.md and build-unix.md.


Build configuration options
============================

UPNnP port forwarding
---------------------

To use UPnP for port forwarding behind a NAT router (recommended, as more connections overall allow for a faster and more stable caix experience), pass the following argument to qmake:

    qmake "USE_UPNP=1"

(in **Qt Creator**, you can find the setting for additional qmake arguments under "Projects" -> "Build Settings" -> "Build Steps", then click "Details" next to **qmake**)

UPnP support is not compiled in by default.  
Set USE_UPNP to a different value to control this:

| **Argument**| **Info**
|-------------|------------------------------------------------------------------------------
| USE_UPNP=-  | no UPnP support, miniupnpc not required; 
| USE_UPNP=0  | (default) built with UPnP, support turned off by default at runtime;
| USE_UPNP=1  | build with UPnP support turned on by default at runtime.  

Generation of QR codes
-----------------------

qrencode may be used to generate QRCode images for payment requests. 
Pass the USE_QRCODE lag to qmake to control this:

| **Argument**  | **Info**
|---------------|------------------------------------------------------------------------------
| USE_QRCODE=0  | No QRCode support - qrencode not required 
| USE_QRCODE=1  | (default) QRCode support enabled

Notification support for recent (k)ubuntu versions
---------------------------------------------------

To see desktop notifications on (k)ubuntu versions starting from 10.04, enable usage of the
FreeDesktop notification interface through DBUS using the following qmake option:

    qmake "USE_DBUS=1"

Berkely DB version warning
==========================

A warning for people using the *static binary* version of CAIx on a Linux/UNIX-ish system (tl;dr: **Berkely DB databases are not forward compatible**).  
The static binary version of CAIx is linked against libdb 5.0 (see also [this Debian issue](http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=621425)).  
Now the nasty thing is that databases from 5.X are not compatible with 4.X.  
If the globally installed development package of Berkely DB installed on your system is 5.X, any source you
build yourself will be linked against that. The first time you run with a 5.X version the database will be upgraded,
and 4.X cannot open the new format. This means that you cannot go back to the old statically linked version without
significant hassle!