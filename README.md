Hierarchical Task Network Planner for UE4
==========================================

Overview
--------

This repository contains a plugin implemented for the [Unreal Engine 4](https://www.unrealengine.com/what-is-unreal-engine-4) (UE4) Game Engine. It implements a Hierarchical Task Network (HTN) Planner that can be used with C++ to define the behavior of NPCs. There is also (limited) support for Blueprint.

The HTN Planner was mainly implemented for research into an approach for Plan Reuse. This research was done during a Research Internship for the Master Artificial Intelligence program at Maastricht University, under the supervision of dr. Mark H. M. Winands. A paper focused on this research has been submitted to a conference for review. The report handed in for the Research Internship at Maastricht University, which is longer and contains more implementation details, can be found in the Report folder. 

Requirements
------------

The plugin requires UE4 to be of any use (it does not have any functionality outside of the engine), and Visual Studio to compile the source code (binaries are not included). The required version of Visual Studio depends on the version of UE4 that is used.

Supported Unreal Engine 4 versions
----------------------------------

The current version of the plugin was implemented for versions 4.9.X of UE4, and has only been tested on those versions.

Supported Operating Systems
---------------------------

The plugin has only been tested on the Windows 7 operating system. It is expected to work fine on other operating systems compatible with UE4 as well, but this has not been tested.