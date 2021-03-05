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

#include "rain-snow-attenuation.h"
#include <fstream>
#include <math.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>
#include <stdio.h>
#include <string>


NS_LOG_COMPONENT_DEFINE("RainSnowAttenuation");

namespace ns3 {

namespace millicar {

NS_OBJECT_ENSURE_REGISTERED(RainSnowAttenuation);

TypeId RainSnowAttenuation::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::RainSnowAttenuation")
          .SetParent<RainAttenuation>()
          .AddConstructor<RainSnowAttenuation>()
          .AddAttribute("altitude", "Altitude in meters above the sea level",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&RainSnowAttenuation::m_altitude),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "h0", "Mean annual 0C isotherm height above mean sea level",
              DoubleValue(0.0), MakeDoubleAccessor(&RainSnowAttenuation::m_h0),
              MakeDoubleChecker<double>());
  return tid;
}
RainSnowAttenuation::RainSnowAttenuation() { NS_LOG_FUNCTION(this); }
RainSnowAttenuation::~RainSnowAttenuation() { NS_LOG_FUNCTION(this); }

/**
 * Computes the mean annual rain height.
 * Output:
 * - meanRainHeight: the mean annual rain height above the mean sea level
 */
double RainSnowAttenuation::getMeanAnnualRainHeight(void) {
  double meanRainHeight = m_h0 + 360;
  return meanRainHeight;

  /**
   * Computes the rain height in the center of communication path in meters
   * above the sea level
   * Input:
   * - hTx: meters above the sea level of the first terminal
   * - hRx: meters above the sea level of the second terminal
   * - distance: distance between the transmitter and receiver
   * Output:
   * - linkHeight: rain height of the center of the link path
   */
}
double RainSnowAttenuation::getRainHeight(double hTx, double hRx,
                                          double distance) {

  double h1H = hTx + m_altitude;
  double h2H = hRx + m_altitude;

  double dist = distance / 1000;

  double linkHeight = (0.5 * (h1H + h2H)) - (pow(dist, 2) / 17);
  return linkHeight;
}

double RainSnowAttenuation::getAttenuationMultiplier(double deltaHeight) {
  double attenMultiplier = 0;
  if (deltaHeight > 0) {
    attenMultiplier = 0;
  } else if ((-1200 <= deltaHeight) && (deltaHeight <= 0)) {
    double denom = 0;
    double nom = 0;

    denom = 1 + pow(1 - exp(-(pow(deltaHeight / 600, 2))), 2) *
                    (4 * pow(1 - exp(deltaHeight / 70), 2) - 1);
    nom = 4 * pow(1 - exp(deltaHeight / 70), 2);
    attenMultiplier = nom / denom;
  } else {
    attenMultiplier = 1;
  }
  return attenMultiplier;
}

/**
 * Computes the multiplying factor
 * Input:
 * - linkHeight: rain height of the center of the
 * communication path in meters above the sea level
 * - meanRainHeight: the mean annual rain height above the mean sea level
 * Output:
 * - mult_factor: multiplying factor
 */

double RainSnowAttenuation::getSnowAttenFactor(double meanRainHeight,
                                               double linkHeight) {
  double x, y;
  int i;
  double prob[49];
  double multFactor = 0;

  /**
   * First, load the table of the propabilities given in
   * Table 1 of ITU-R P.530-15.
   * The rain height variability is modeled by taking 49
   * intervals of 100 m relative to the mean rain height.
   *
   */

  std::ifstream rainHeightProb;
  rainHeightProb.open("src/millicar/model/rainHeight_prob.txt");
  if (!rainHeightProb) {
    printf("\n Unable to open the file");
    exit(1);
  }
  i = 0;
  while (!rainHeightProb.eof()) {
    rainHeightProb >> x >> y;
    prob[i] = y;

    // Do the following calculation for each interval
    // Calculate the rain height -> rainHeight
    double rainHeight = meanRainHeight - 2400 + 100 * i;

    // Calculate the link height relative to the rain height
    double deltaHeight = linkHeight - rainHeight;

    // Calculate the addition to the multiplying factor
    double attenuationMultiplyingFact = getAttenuationMultiplier(deltaHeight);

    // Compute the multipying factor for each interval
    double deltaF = attenuationMultiplyingFact * prob[i];

    // Add the multiplying factor for each interval
    multFactor = multFactor + deltaF;
    i++;
  }

  rainHeightProb.close();

  return multFactor;
}
/**
 * Computes the attenuation from combined rain and wet snow
 * Input:
 * - distance: distance between the transmitter and receiver
 * - frequency
 * - hTx: meters above the sea level of the first terminal
 * - hRx: meters above the sea level of the second terminal
 * Output:
 * - attenSnow: combined rain and wet snow attenuation
 */

double RainSnowAttenuation::getSnowAttenuation(double distance,
                                               double frequency, double hTx,
                                               double hRx) {
  double attenSnow = 0;

  double linkHeight = getRainHeight(hTx, hRx, distance);
  double meanRainHeight = getMeanAnnualRainHeight();

  Ptr<RainAttenuation> rainAtten = CreateObject<RainAttenuation>();
  double rainAttenuation = rainAtten->getRainAttenuation(distance, frequency);

  if (linkHeight <= (meanRainHeight - 3600)) {
    printf("The location is not affected by the wet snow.");
    attenSnow = rainAttenuation;
  } else {
    printf("The location it is affected by the wet snow.");
    double attenFactor = getSnowAttenFactor(meanRainHeight, linkHeight);
    attenSnow = rainAttenuation * attenFactor;
  }
  
  return attenSnow;
}
} // namespace millicar
}; // namespace ns3
