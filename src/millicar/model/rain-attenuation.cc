/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Telecommunication Networks (TKN), TU Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sigrid Dimce <dimce@ccs-labs.org>
 *
 */

#include "rain-attenuation.h"
#include <fstream>
#include <math.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>
#include <stdio.h>
#include <string>

NS_LOG_COMPONENT_DEFINE("RainAttenuation");



namespace ns3 {

namespace millicar {

NS_OBJECT_ENSURE_REGISTERED(RainAttenuation);

TypeId RainAttenuation::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::RainAttenuation")
          .SetParent<Object>()
          .AddConstructor<RainAttenuation>()
          .AddAttribute("RainRate", "Intensity of rain in mm/h",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RainAttenuation::m_rainRate),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("k", "Regression coefficient k", DoubleValue(0.0),
                        MakeDoubleAccessor(&RainAttenuation::m_k),
                        MakeDoubleChecker<double>())
          .AddAttribute("alpha", "Regression coefficient alpha",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&RainAttenuation::m_alpha),
                        MakeDoubleChecker<double>());
  return tid;
}
RainAttenuation::RainAttenuation() { NS_LOG_FUNCTION(this); }
RainAttenuation::~RainAttenuation() { NS_LOG_FUNCTION(this); }

/**
 * Get specific rain atteuation.
 * Output:
 * - specRainAtten: specific rain attenuation
 */
double RainAttenuation::getSpecificAttenuation(void) {
  double specRainAtten = m_k * pow(m_rainRate, m_alpha);
  return specRainAtten;
}

/**
 * Computes the distance factor
 * Inputs:
 * - frequency
 * - distance: distance between transmitter and receiver
 */
double RainAttenuation::getDistanceFactor(double frequeny, double distance) {

  double freq = frequeny / 10e8;
  double distanceFactor = 0;

  double denom = 0.477 * pow(distance, 0.633) * pow(m_rainRate, (0.073 * m_alpha)) *
              pow(freq, 0.123) -
          10.579 * (1 - exp(-0.024 * distance)); // Compute the denominator

  /**
   * If the value of the denominator is smaller than 0.4, the distance factor
   * takes the value 2.5, which is the maximum possible value according to ITU-R
   * P. 530-17
   */
  if (denom < 0.4) {
    distanceFactor = 2.5;
  } else {
    distanceFactor = 1 / denom;
  }

  return distanceFactor;
}

/**
 * Computes the attenuation from rain
 * Inputs:
 * - distance: distance between transmitter and receiver
 * - frequency: carrier frequency
 * Output:
 * - atten: the value of rain attenuation
 */
double RainAttenuation::getRainAttenuation(double distance, double frequency) {
  double dist = distance / 1000; // Convert the distance in km
  double distFactor = getDistanceFactor(frequency, dist);
  double specificAttenuation = getSpecificAttenuation();
  double atten = specificAttenuation * dist * distFactor;

  return atten;
}

} // namespace millicar
}; // namespace ns3
