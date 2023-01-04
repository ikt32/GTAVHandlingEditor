# GTA V Real Time Handling Editor

A real-time vehicle handling editor for Grand Theft Auto V, intended for vehicle (handling) developers,
to easily adjust handling in-game and see the effects of the changes immediately.

## Features

* In-game menu for real-time handling entry editing
* Load and save XML/meta files, starting from the `<Item />` level
* Drawn COM, roll centre heights and more
* Reload vehicle with all its modifications
* Grabs additional parameter information from [GTAVHandlingInfo](https://github.com/E66666666/GTAVHandlingInfo)
  * Automatically updates (on launch) if a newer version is present
* Supports `SubHandlingData` (almost completely)
  * `CCarHandlingData` (including AdvancedData)
  * `CBikeHandlingData`
  * `CFlyingHandlingData`
  * `CSpecialFlightHandlingData`
  * `CBoatHandlingData`
  * `CSeaPlaneHandlingData`
  * `CSubmarineHandlingData`
  * `CTrailerHandlingData`

## Usage requirements

* Grand Theft Auto V (b1604+)
* ScriptHookV

Optional:

* [Add-On Vehicle Spawner](https://www.gta5-mods.com/scripts/add-on-vehicle-spawner)
(Uses its generated hash lookup, for automatic handling names)

## Installation

Extract `RTHandlingEditor.asi` and the folder `HandlingEditor` into the main GTA V folder.

Check the `settings_menu.ini` file in `HandlingEditor` to change menu hotkeys, if desired.

## Usage

Use the `rthe` cheat to open the menu.

The menu itself should be straightforward - most options have a description.

Inside the editing menus every parameter can be manually entered by pressing
Enter on that option.

Certain parameters do not apply instantly - the vehicle needs to be reloaded.
The main menu contains a trigger for this - the current vehicle will be reloaded
with all its tuning modifications.

## Building

* Visual Studio 2022 (C++20 bits and pieces)
* [ScriptHookV SDK by Alexander Blade](http://www.dev-c.com/gtav/scripthookv/)

Clone this repository recursively, to pull in all the submodules.
