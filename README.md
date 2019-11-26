# GTA V Real Time Handling Editor

Using this script, you can edit the handling of vehicles in real-time.

## Features

* In-game menu where you can edit handling entries in real-time
* Loads XML/Meta files, starting from the `<Item />` level
* Saves XML files of whatever vehicle you're editing
* TODO: Handling dev tools:
  * G-Force meter with lateral and longitudinal axes
  * Acceleration timers

## Requirements

* Grand Theft Auto V b1604+
* ScriptHookV

## Installation

Extract `RTHandlingEditor.asi` and the folder `HandlingEditor` into the main GTA V folder.
TODO: ini/folder names subject to change, until v2.0.0 release.

## Usage in-game

Use the `rthe` cheat to open the menu. From there, follow the on-screen instructions.

## Limitations

* Some values do not apply instantly. The game will need to reload the vehicle to apply these properly. Some examples:
  * vecCentreOfMassOffset
  * nInitialDriveGears
  * fDriveBiasFront
* Some values using radians internally have small rounding errors

------------------------------

## Building

### Requirements <!-- Building -->

* [ScriptHookV SDK by Alexander Blade](http://www.dev-c.com/gtav/scripthookv/)

Clone this repository recursively, to pull in all the submodules.
