CAIx-qt: Qt5 GUI for CAIx
===============================
Mac OS X
--------

- Install XCode and the Command Line Developer tools
- Download and install [MacPorts](http://www.macports.org/install.php).  
- Execute the following commands in a terminal to get the dependencies:

	 sudo port selfupdate  
	 sudo port install boost db48@+no_java openssl miniupnpc autoconf pkgconfig automake libtool   
	 sudo port install qt5-mac qrencode protobuf-cpp

- Clone our latest github sources into a folder and navigate to that folder in your terminal by using cd  
- Execute the qmake and make commands in your terminal

Note: if u get a leveldb error run the following commands:

	 cd ./src/leveldb
	 sudo chmod +x build_detect_platforms
	 sudo make libleveldb.a liblevelmeme.a
	 cd ../..

And run the qmake and make commands again.