#!/bin/bash
# This bash script is used to run the simulations for: sumo_ns3_simple_scenario
./waf configure --enable-examples

# MilliCar parameters

environment="l"
commScenario="V2V-Urban"


# Create a vector which stores the values of the rain intensities
declare -a rainIntensity
rainIntensity[0]=10

# Regression coefficients
k=0.8606 # horizontal polarization
alpha=0.7656  # horizontal polarization

# k = 0.8515  # vertical polarization
# alpha = 0.7486  # vertical polarization

# Snow Attenuation Parameters
snowAttenuation=true
altitude=122  # altitude in meters above the sea level of Paderborn
h0=2000  # mean annual 0C isotherm height above mean sea level


for intensity in ${rainIntensity[@]}
do

simRun=1

  ./waf --run "sumo_ns3_paderborn  --simulationRun=$simRun --intensityOfRain=$intensity --channel_condition=$environment --combined_rain_snow=$snowAttenuation --scenario=$commScenario --RngRun=$simRun --k=$k --alpha=$alpha --altitude=$altitude  --h0=$h0"

done  


