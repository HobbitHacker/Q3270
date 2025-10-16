# Q3270 - A Qt-based 3270 Emulator

Q3270 is a Qt-based 3270 terminal emulator with colour themes and keyboard mapping dialogs built in. 

"Emulator" is an interesting term. Some refer to these kinds of programs as Simulators, some as
TN3270 Clients, and others use the term Emulator. I use the term Emulator.

Aimed at the IBM mainframe (MVS, OS/390, z/OS, VM, zVM, call it what you will), this
program is intended to be used to access computers running those operating systems, 
via the TN3270 protocol.

It's not (yet) as feature rich as other emulators, but it is (in my limited cases) usuable for
things like MVS 3.8j under Hercules. 

I've used it for MVS 3.8j and VM/370, and for IBM's zXplore at 204.90.115.200:623.

Documentation is unfortunately sparse presently. 

---

## Features
- Standard 3270 model sizes
- IBM-DYNAMIC user-selectable screen size
- Keyboard mapping dialog
- Colour mapping dialog
- Session management (save, load, auto‑start lists)
- SSL support (not heavily tested)
- Dynamic font scaling according to window size

---

## Getting Started

### Prerequisites
- **Qt**: 5.15 (baseline, known‑good)  
- Qt 6.x should work, but is currently experimental.  
- **g++** C++ compiler
- **CMake**
- **QtSvg** module (required for SVG rendering)

### Dependencies

On Linux, install Qt development libraries and build tools via your package manager.  
For example (these package names may be incorrect, check your distro) - :

- Ubuntu/Debian:
  `sudo apt install build-essential qtbase5-dev qttools5-dev-tools libqt5svg5-dev`

- Fedora:
  `sudo dnf install qt5-qtbase-devel qt5-qtsvg-devel`

- openSUSE:
  `sudo zypper install libqt5-qtbase-devel libqt5-qtsvg-devel`

- Arch Linux:
  `sudo pacman -S base-devel qt5-base qt5-svg`

### Building
`git clone https://github.com/HobbitHacker/Q3270.git`

`cd Q3270`

`cmake .`

`make`

This will build a Q3270 executable, which you can then run with:

`./Q3270`

## Packages

Unstable testing packages may be found under

https://github.com/HobbitHacker/Q3270/actions

for a given distribution. 

## Backlog

See BACKLOG.md for a list of things still to be done.
