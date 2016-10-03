GTA V Real Time Handling Editor
===========

This .asi plugin will allow realtime editing of handling.meta values in-memory. Editing this way will eliminate the need to restart the game every time something has been edited, for testing purposes.

## Features
* Realtime handling editing
* Correct front/rear bias setting

## Requirements
* Grand Theft Auto V
* ScriptHookV

The offsets for the values have only been tested in b791_2

## Installation
* Drop RTHandlingEditor.asi in the main GTA V folder
* Drop RTHandlingEditor.ini in the main GTA V folder

## Usage
* Press the `read` hotkey to read `RTHandlingEditor.ini` values into memory
* Press the `log` hotkey to write memory values into `RTHandlingEditor.log`
* Use `-1337` in the `[handling]` section to ignore the value

## Limitations
* Some values do not apply while in-game and need restarting still. For example, nInitialDriveGears.
* Some values using radians internally have small rounding errors

------------------------------

## Building

### Requirements
* [ScriptHookV SDK by Alexander Blade](http://www.dev-c.com/gtav/scripthookv/)

Download the [ScriptHookV SDK](http://www.dev-c.com/gtav/scripthookv/) and extract it's contents to ScriptHookV_SDK. 
Clone this repository to the same folder ScriptHookV_SDK was extracted so you have ScriptHookV_SDK and GTAVManualTransmission in the same folder. If you get build errors about missing functions, update your [natives.h](http://www.dev-c.com/nativedb/natives.h).
