#!/bin/bash
# This bash script is used to run the simulations for: sumo_ns3_scenario.cc
./waf configure --enable-examples


# MilliCar parameters
environment="l" # the parameter defines the channel condition
commScenario="V2V-Urban" # communication environment


# Create a vector which stores the values of the rain intensities
declare -a rainIntensity
rainIntensity[0]=30 # mm/h

# The regression coefficients for computing the specific rain attenuation
k=0.8606  # horizontal polarization
alpha=0.7656  # horizontal polarization

# k = 0.8515  # vertical polarization
# alpha = 0.7486  # vertical polarization

# The input parameters for computing the attenuation from combined rain and wet snow
snowAttenuation=true
alt=122.0 # altitute
h0=2000; # Mean annual 0C isotherm height above mean sea level

for intensity in ${rainIntensity[@]}
do

simRun=1 
	
  ./waf --run "simple_example --simulationRun=$simRun --intensityOfRain=$intensity --combined_rain_snow=$snowAttenuation --channel_condition=$environment --scenario=$commScenario --RngRun=$simRun  --k=$k  --alpha=$alpha  --altitude=$alt  --h0=$h0"

done

