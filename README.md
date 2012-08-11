DAWM
=========

DAWM is a tiling window manager written in C, that is partially based on dwm.

The long-term goals of DAWM are:

* To have as few dependencies as possible (Should be compilable with only POSIX libraries and Xlib)
* EWMH (NetWM) compliance
* (Optional) Xinerama support
* (Optional) Xft support

Installation
------------
DAWM requires Xlib development headers.

The software can be installed with the following command:

    cd src/ && make install

Documentation
------------
The documentation can be built with the following command:

    cd doc/ && make

Configuration
------------
DAWM's configuration file is $HOME/.config/dawm/config.

An example configuration file can be found in data/config.

Autorun
------------
DAWM can be made to autorun programs by executing the file
$HOME/.config/dawm/autorun. This file can be a shell script.
The file must have the executable flag set.

The file can be created with the following command:

    echo "#! /bin/sh" > ~/.config/dawm/autorun && chmod +x ~/.config/dawm/autorun

License
------------
See the LICENSE file.
