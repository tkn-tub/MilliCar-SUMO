Paderborn Traffic Scenario
==========================

Release v0.1

Contact: Dominik S. Buse <buse@ccs-labs.org>

A traffic scenario for [SUMO (Simulation of Urban Mobility)](https://sumo.dlr.de) modeling the city of Paderborn, Germany.
Contains the whole administrative region of the city of Paderborn and traffic representing a 24-hour weekday.

This Scenario has been tested with SUMO v1.6.0.

This project is licensed under the terms of the GPLv3 license.

Citing
------

Please cite this scenario as:

D. S. Buse, “Paderborn Traffic Scenario,” School of Electrical Engineering and Computer Science, TU Berlin, Berlin, Germany, Traffic Simulation Scenario, Feb. 2021, doi: 10.5281/zenodo.4522059.

Configurations
--------------

There are two ready-to-use configurations for this scenario:

- `two-hours-ad-hoc.sumo.cfg` -- a quick scenario with two hours of traffic, yielding up to around 2000 simultaneous vehicles.
	Uses ad-hoc routing to read trips directly from a trips file.
- `full-day.sumo.cfg` -- the full scenario with 24 hour of traffic (see figure below).
	Uses routes pre-computed using [Dynamic User Assignment](https://sumo.dlr.de/docs/Demand/Dynamic_User_Assignment.html) to avoid traffic jams.

Statistics
----------

Here are some cornerstone statistics for the full scenario:

- Total number of trips: 203387
- Simulated time steps: 89015 or 24:43:35
- Performance - total runtime on i7 7700k: ca 1500 seconds or 42 minutes
- Teleports: 9

![Number of vehicles running over the simulated day](doc/images/running-vehicles-full-day.png "Number of vehiles running over the simulated day")

Included Files
--------------

Files used for simulation with SUMO:

- `network.net.xml` -- road network description, including base (non-actuated) traffic lights.
- `trafficlightlogic.tll.xml` -- actuated traffic light logic definitions for the network.
- `polygons.poly.xml` -- buildings and other shapes and POIs, mostly relevant for use in the GUI and V2X simulation with Veins.
- `vtypes.add.xml` -- vehicle types used in the simulation, required by all other `trp.xml` and `rou.xml` files.
- `trips.trp.xml` -- small traffic demand set, contains about two hours of spawning vehicles.
- `alltrips.trp.xml` -- full traffic demand set, contains about an entire day of spawning vehicles. Only trips without pre-computed routes. If you use this directly, it will give you traffic jams -- use the `allroutes.rou.xml` instead unless you know what your are doing.
- `allroutes.rou.xml` -- pre-computed routes derived from `alltrips.trp.xml` using Dynamic User Assignment. Use this for a whole day of running traffic.
