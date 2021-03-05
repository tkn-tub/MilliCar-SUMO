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

#ifndef RAIN_ATTENUATION_H_
#define RAIN_ATTENUATION_H_

#include "ns3/mmwave-vehicular-propagation-loss-model.h"
#include "ns3/object.h"

namespace ns3 {

namespace millicar {

class RainAttenuation : public Object {
public:
  static TypeId GetTypeId(void);
  RainAttenuation();
  virtual ~RainAttenuation();

  /**
   * Get the specific rain attenuation using the coefficients
   * given in ITU-R P.838-3
   */
  double getSpecificAttenuation(void);

  /**
   * Computes the distance factor
   * which consideres the inhomogeneity of rain
   * in the transmission link
   */
  double getDistanceFactor(double frequeny, double distance);

  /**
   * Computes the rain atenuation
   */
  double getRainAttenuation(double distance, double frequency);

private:
  double m_k;
  double m_alpha;
  // Rain intensity in mm/h
  uint32_t m_rainRate;
};
} // namespace millicar

} // namespace ns3

#endif
