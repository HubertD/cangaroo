# cangaroo
open source can bus analyzer

written by Hubert Denkmair <hubert@denkmair.de>

## changelog

### v0.1.4 unreleased
* handle muxed signals in backend and trace window
* can status window
* bugfixes in setup dialog
* do not try to extract signals from messages when DLC too short
* fix windows build
* windows PCAN-basic driver

### v0.1.3 released 2016-01-16
* new can interface configuration GUI (missing a suid binary to actually set the config)
* use libnl-route-3 for socketcan device config read
* query socketcan interfaces for supported config options
* new logging subsystem, do not use QDebug any more
* some performance improvements when receiving lots of messages 
* bugfix with time-delta view: timestamps not shown when no previous message avail

### v0.1.2 released 2016-01-12
* fix device re-scan ("could not bind" console message)
* fix some dbc parsing issues (signed signals, ...)
* implement big endian signals

### v0.1.1 released 2016-01-11
* change source structure to better fit debian packaging
* add debian packaging info

### v0.1 released 2016-01-10
initial release \o/



## todo

### backend
* allow for canfd frames
* support non-message frames in traces (e.g. markers)
* implement plugin API
* embed python for scripting

### can drivers
* allow socketcan interface config through suid binary
* socketcan: use hardware timestamps (SIOCSHWTSTAMP) if possible
* cannelloni support
* windows vector driver

### import / export
* export to other file formats (e.g. Vector ASC, BLF, MDF)
* import CAN-Traces

### general ui
* maybe switch to docking window system

### logging subsystem
* show timestamps, log level etc. in log window

### can status window
* display #warnings, #passive, #busoff, #restarts of socketcan devices

### trace window
* message filtering
* assign colors to can interfaces / messages
* limit displayed number of messages
* show error frames and other non-message frames
* sort signals by startbit, name or position in candb

### raw message generator
* provide a simple way to generate raw can messages

### CanDB based generator
* generate can messages from candbs
* set signals according to value tables etc.
* provide generator functions for signal values
* allow scripting of signal values

### replay window
* replay can traces
* map interfaces in traces to available networks

### graph window
* test QCustomPlot
* allow for graphing of interface stats, message stats and signals

### packaging / deployment
* provide clean debian package
* gentoo ebuild script
* provide static linked binary
* add windows installer

