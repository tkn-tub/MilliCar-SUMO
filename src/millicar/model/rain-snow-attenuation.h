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

#ifndef RAIN_SNOW_ATTENUATION_H_
#define RAIN_SNOW_ATTENUATION_H_

#include "ns3/mmwave-vehicular-propagation-loss-model.h"
#include "ns3/object.h"
#include "ns3/rain-attenuation.h"

namespace ns3 {

namespace millicar {

class RainSnowAttenuation : public Object {
public:
  static TypeId GetTypeId(void);
  RainSnowAttenuation();
  virtual ~RainSnowAttenuation();

  /**
   * Computes the mean annual rain height for a specific location
   */
  double getMeanAnnualRainHeight(void);

  /**
   * Computes the rain height of the center of the communication
   * path in meters above the sea level
   */
  double getRainHeight(double hTx, double hRx, double distance);

  /**
   * Computes the attenuation factor
   */
  double getSnowAttenFactor(double meanRainHeight, double linkHeight);

  /**
   * Computes the multiplying factor considering the corresponding
   * link height relative to the rain height
   */
  double getAttenuationMultiplier(double deltaHeight);

  /**
   * Computes the attenuation from combined rain and wet snow
   */

  double getSnowAttenuation(double distance, double frequency, double hTx,
                            double hRx);

private:
  double m_altitude; // altitude in meters above the sea level 
  double m_h0;       // mean annual 0C isotherm height above mean sea level
};

} // namespace millicar

} // namespace ns3

#endif
