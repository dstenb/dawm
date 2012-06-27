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

Configuration
------------
DAWM's configuration file is located in $HOME/.config/dawm/config.

An example configuration file can be found in data/config.

License
------------
See the LICENSE file.
