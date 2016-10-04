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
* (Optional) Drop RTHandlingEditor.meta in the main GTA V folder

## Usage in-game
* Press the `read` hotkey during gameplay to read `RTHandlingEditor.ini` values into memory
* Press the `log` hotkey during gameplay to write memory values into `RTHandlingEditor.log`

## Usage .ini and .meta
* Use `-1337` in the `[handling]` section to ignore a value in `RTHandlingEditor.ini`
* Replace everything in `RTHandlingEditor.meta` with your handling entry, between ``
* An example with Rockstar's original Sultan entry is included. Replace at will

## Limitations
* Some values do not apply instantly. The game will need to reload the vehicle to apply these properly. Some examples:
  * vecCentreOfMassOffset
  * nInitialDriveGears
  * fDriveBiasFront
* Some values using radians internally have small rounding errors


------------------------------

## Building

### Requirements
* [ScriptHookV SDK by Alexander Blade](http://www.dev-c.com/gtav/scripthookv/)

Download the [ScriptHookV SDK](http://www.dev-c.com/gtav/scripthookv/) and extract it's contents to ScriptHookV_SDK. 
Clone this repository to the same folder ScriptHookV_SDK was extracted so you have ScriptHookV_SDK and GTAVManualTransmission in the same folder. If you get build errors about missing functions, update your [natives.h](http://www.dev-c.com/nativedb/natives.h).
