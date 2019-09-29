EV3 C++ toolset
===============
*Very WIP*

So far only USB connections are supported.

Structure
---------
* `libev3comm` - contains implementation of communication with EV3
* `libev3asm` - EV3 bytecode assembler library; uses `lmsasm`'s
                `ev3.yaml` bytecode definition
* `ev3tool` - command line utility built on top of libev3comm

ev3tool help
------------
```
EV3 CLI tool 0.1.0
Usage:
ev3tool [flags] <command> [flags]
  -h, --help            Show this help message.
  -V, --version         Show program version string.
  -v, --verbose         Increase output verbosity.
  -q, --quiet           Decrease output verbosity.
      --md5             Show MD5 hashes of files in directory listings.
  -c=<>, --connect=<>   Connect to EV3 at a specific address.
  -f=<>, --from=<>      Upload/download file from a location.
  -t=<>, --to=<>        Upload/download file to a location.

Commands:
 list          Prints all connected bricks.
 upload        Upload a file to the brick. Needs --from and --to.
 download      Download a file from the brick. Needs --from and --to.
 mkdir         Creates a directory on the brick. Needs --to.
 ls            Print directory listing from the brick. Needs --from.
 rm            Remove a file from the brick. Needs --from.
 mailbox       Write into EV3 mailbox. Needs --from (file) and --to (mailbox name).
 btpin         Set BT pin. Needs --from (client MAC) and --to (new PIN).
 listhnds      List all open handles.
 closehnds     Try to close all handles.
 closehnd      Close only one handle. Needs --from.
 fwenter       Reboot into bootloader/firmware update mode.
 fwexit        Reboot out of bootloader/firmware update mode.
 fwver         Get HW/FW version from bootloader mode.
 fwflash       Flash a firmware file to the brick. Needs --from.

Address syntax:
HID:<VID>:<PID>[:<SN>]
  VID = USB/BT hexadecimal vendor id
  PID = USB/BT hexadecimal product id
  SN  = USB/BT product serial number
```

Supported platforms
-------------------
Xubuntu 18.04 works.

Windows should work too, but the code is not yet tested.
