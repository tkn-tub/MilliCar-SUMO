# MilliCar ns-3 module - ITU fading prediction methods - SUMO #

This is an extension of the [Millicar](https://github.com/signetlabdei/millicar) module by the International Telecommunication Union (ITU) fading prediction methods, for estimating the attenuation from pure rain and combined rain and wet snow. 

MilliCar is an [ns-3](https://www.nsnam.org "ns-3 Website") module that enables full stack end-to-end simulations in the Vehicle-to-Vehicle (V2V) networks operating in the millimeter wave (mmWave) frequency band.  

The ITU methods for estimating the attenuation from pure rain and combined rain and wet snow are described in ITU-R P.530-15 [1], using the regression coeficients specified in ITU-R P838-3 [2].

To investigate the impact of the weather conditions in a realistic vehicular scenario, is used the coupling implementation of ns-3 with the traffic simulator [SUMO](https://www.eclipse.org/sumo/) given in [here](https://github.com/vodafone-chair/ns3-sumo-coupling) and the scenario for the city of [Paderborn](https://zenodo.org/record/4522059) [3], Germany.

The Millicar model has been extended by two new classes:
 - `RainAttenuation` - for estimating the attenuation from pure rain
 - `RainSnowAttenuation` - for estimating the attenuation from the combined rain and wet snow effect
 The attenuation from the weather conditions is added to the path loss computed in the `MmWaveVehicularPropagationLossModel` class using the function `GetWeatherAttenuation`. 

## Examples
A simple scenario consisting of two vehicles following each other is presented under `scratch/simple_example/sumo_ns3_scenario.cc`. The user can start the scenario by running the bash script `example_sumo_ns3.sh` from the terminal. The bash script can be used to run several simulations, with different seeds and to set several parameters such as: the rain intensity, the regression coefficients for estimating the attenuation from rain and the altitude for the specific location to compute the attenuation from combined rain and wet snow.

A simple scenario for the Paderborn city can be found inside `scratch/sumo_ns3_paderborn/sumo_ns3_paderborn_scenario.cc`. The use can start the scenario by running the bash script `sumo_ns3_paderborn_scenario.sh`. 


## System specification:
The MilliCar extension and the two examples were tested on the following system:
 - ns-3: ns-3.30
 - SUMO: 1.7.0
 - Ubuntu 20.04.2 LTS

## LICENSE
The extension of MilliCar module with the fading prediction developed by the ITU, is licensed under the terms of the GNU GPLv2, similar to Millicar module and ns-3. See the LICENSE file for more details.
All the components that are used such as SUMO simulator, the Paderborn scenario and the coupling of SUMO with ns-3, come with the corresponding LICENCE.

## References
[1] International Telecommunication Union, “Propagation data and prediction methods required for the design of terrestrial line-of-sight systems,” ITU-R,Recommendation P.530-15, Sep. 2013 \
[2] International Telecommunication Union, “Specific attenuation model for rain for use in prediction methods,” ITU-R, Recommendation P.838-3, Mar. 2005, pp. 1–8 \
[3] D. S. Buse, “Paderborn Traffic Scenario,” Zenodo, Software Package, Feb. 2021 
