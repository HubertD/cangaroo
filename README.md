# cangaroo
open source can bus analyzer

written by Hubert Denkmair <hubert@denkmair.de>

## changelog


### v0.1.3 unreleased
* some performance improvements when receiving lots of messages 
* bugfix with time-delta view: timestamps not shown when no previous message avail
* new logging subsystem, do not use QDebug any more
* new can interface configuration GUI (only GUI, no backend functionality yet)

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
* define driver features like detailed bittimings, bus status, statistics
* support non-message frames in traces (e.g. markers)
* implement plugin API
* embed python for scripting
* implement muxed signals

### can drivers
* redo socketcan driver (clean qt/c++ implementation, suid binary for interface configuration)
* socketcan: use hardware timestamps (SIOCSHWTSTAMP) if possible
* cannelloni support
* windows peak can driver
* windows vector driver

### import / export
* export to other file formats (e.g. Vector ASC, BLF, MDF)
* import CAN-Traces

### general ui
* maybe switch to docking window system

### logging subsystem
* show timestamps, log level etc. in log window

### interface status window
* show status (error mode, tx/rx counts, ...) of all can interfaces

### trace window
* message filtering
* assign colors to can interfaces / messages
* limit displayed number of messages
* show error frames and other non-message frames
* hide unused muxed signals in linear trace view
* handle muxed signals properly in aggregated trace view
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
* make project build on windows
* add windows installer

