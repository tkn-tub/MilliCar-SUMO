#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mmwave-vehicular-helper.h"
#include "ns3/mmwave-vehicular-net-device.h"
#include "ns3/mmwave-vehicular-propagation-loss-model.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/rain-snow-attenuation.h"
#include "ns3/system-wall-clock-ms.h"
#include "ns3/traci-applications-module.h"
#include "ns3/traci-module.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using namespace ns3;
using namespace millicar;

Ptr<TraciClient> sumoClient = CreateObject<TraciClient>();
Ptr<MmWaveVehicularHelper> helper = CreateObject<MmWaveVehicularHelper>();

void computePathLoss(NetDeviceContainer, double);

int main(int argc, char *argv[]) {

  double frequency = 60e9;  // frequency in Hz
  double bandwidth = 2.5e8; // bandwidth in Hz
  uint32_t simulationRun;   // number of simulation
  uint32_t intensityOfRain; // rain intensity in mm/h
  double k = 0.0; // regression coffiecient for computing specific rain attenuation
           //             // (horizontal/vertical polarization)
  double alpha = 0.0; // regression coffiecient for computing specific rain
  //                    // attenuation (horizontal/vertical polarization)
  double altitude = 0.0; // altitude in meters above the sea level of Paderborn
  double h0 = 0.0;       // mean annual 0C isotherm height above mean sea level
  std::string channel_condition;
  std::string scenario;
  bool combined_rain_snow;

  ns3::Time simulationTime(ns3::Seconds(200));
  double stepTime = 1.0;

  CommandLine cmd;

  cmd.AddValue("intensityOfRain", "Specifies the value of rain intensity",
               intensityOfRain);
  cmd.AddValue("combined_rain_snow",
               "Defines weather the attenuation from combined rain and wet "
               "snow should be computed",
               combined_rain_snow);
  cmd.AddValue("channel_condition", "Defines the channel condition",
               channel_condition);
  cmd.AddValue("scenario",
               "Defines the scenario where the communication takes place",
               scenario);
  cmd.AddValue("simulationRun", "Defines the number of simulations",
               simulationRun);
  cmd.AddValue("k", "Regression coefficient k", k);
  cmd.AddValue("alpha", "Regression coefficient alpha", alpha);
  cmd.AddValue("altitude", "Altitude in meters above the sea level", altitude);
  cmd.AddValue("h0", "Mean annual 0C isotherm height above mean sea level", h0);

  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::MmWavePhyMacCommon::CenterFreq",
                     DoubleValue(frequency));
  Config::SetDefault(
      "ns3::MmWaveVehicularPropagationLossModel::ChannelCondition",
      StringValue(channel_condition));
  Config::SetDefault("ns3::MmWaveVehicularPropagationLossModel::Scenario",
                     StringValue(scenario));
  Config::SetDefault("ns3::MmWaveVehicularHelper::Bandwidth",
                     DoubleValue(bandwidth));
  Config::SetDefault("ns3::RainAttenuation::RainRate",
                     UintegerValue(intensityOfRain));
  Config::SetDefault("ns3::RainAttenuation::k", DoubleValue(k));
  Config::SetDefault("ns3::RainAttenuation::alpha", DoubleValue(alpha));
  Config::SetDefault("ns3::RainSnowAttenuation::altitude",
                     DoubleValue(altitude));
  Config::SetDefault("ns3::RainSnowAttenuation::h0", DoubleValue(h0));
  Config::SetDefault("ns3::MmWaveVehicularPropagationLossModel::Shadowing",
                     BooleanValue(true));
  Config::SetDefault("ns3::MmWaveVehicularPropagationLossModel::SnowEffect",
                     BooleanValue(combined_rain_snow));
  Config::SetDefault(
      "ns3::MmWaveVehicularSpectrumPropagationLossModel::UpdatePeriod",
      TimeValue(MilliSeconds(1)));

  // SUMO Configuration
  sumoClient->SetAttribute(
      "SumoConfigPath",
      StringValue("scratch/simple_example/sumo_ns3_example.sumocfg"));
  sumoClient->SetAttribute("SumoBinaryPath",
                           StringValue("")); // use system installation of sumo
  sumoClient->SetAttribute("SynchInterval", TimeValue(Seconds(1.0)));
  sumoClient->SetAttribute("StartTime", TimeValue(Seconds(0.0)));
  sumoClient->SetAttribute("SumoPort", UintegerValue(3400));
  sumoClient->SetAttribute(
      "PenetrationRate",
      DoubleValue(1.0)); // portion of vehicles equipped with wifi
  sumoClient->SetAttribute("SumoLogFile", BooleanValue(true));
  sumoClient->SetAttribute("SumoStepLog", BooleanValue(false));
  sumoClient->SetAttribute("SumoSeed", IntegerValue(10));
  sumoClient->SetAttribute("SumoAdditionalCmdOptions",
                           StringValue("--fcd-output sumoTrace.xml"));
  sumoClient->SetAttribute("SumoWaitForSocket", TimeValue(Seconds(1.0)));
  sumoClient->SetAttribute("SumoGUI", BooleanValue(true));

  NodeContainer n;
  n.Create(2);
  uint32_t nodeCounter(0);

  // create the mobility nodes
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(n);

  // Create and configure the helper
  Ptr<MmWaveVehicularHelper> helper = CreateObject<MmWaveVehicularHelper>();
  helper->SetPropagationLossModelType(
      "ns3::MmWaveVehicularPropagationLossModel");
  helper->SetSpectrumPropagationLossModelType(
      "ns3::MmWaveVehicularSpectrumPropagationLossModel");
  helper->SetNumerology(3);
  NetDeviceContainer devs = helper->InstallMmWaveVehicularNetDevices(n);

  InternetStackHelper internet;
  internet.Install(n);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devs);

  helper->PairDevices(devs);

  VehicleSpeedControlHelper vehicleSpeedControlHelper(9);
  vehicleSpeedControlHelper.SetAttribute("Client", (PointerValue)sumoClient);

  // a callback function to setup the nodes
  std::function<Ptr<Node>()> setupNewWifiNode = [&]() -> Ptr<Node> {
    if (nodeCounter >= n.GetN())
      NS_FATAL_ERROR("Node Container empty!: " << nodeCounter
                                               << " nodes created.");

    // don't create and install the protocol stack of the node at simulation
    // time -> take from "node pool"
    Ptr<Node> includedNode = n.Get(nodeCounter);
    ++nodeCounter; // increment counter for next node

    // Install Application
    ApplicationContainer vehicleSpeedControlApps =
        vehicleSpeedControlHelper.Install(includedNode);
    vehicleSpeedControlApps.Start(Seconds(0.0));
    vehicleSpeedControlApps.Stop(simulationTime);

    return includedNode;
  };
  // a callback function for node shutdown
  std::function<void(Ptr<Node>)> shutdownWifiNode = [](Ptr<Node> exNode) {
    // stop all applications
    Ptr<VehicleSpeedControl> vehicleSpeedControl =
        exNode->GetApplication(0)->GetObject<VehicleSpeedControl>();
    if (vehicleSpeedControl)
      vehicleSpeedControl->StopApplicationNow();

    // set position outside communication range in SUMO
    Ptr<ConstantPositionMobilityModel> mob =
        exNode->GetObject<ConstantPositionMobilityModel>();
    mob->SetPosition(
        Vector(-100.0 + (rand() % 25), 320.0 + (rand() % 25), 250.0));
  };

  // start traci client
  sumoClient->SumoSetup(setupNewWifiNode, shutdownWifiNode);
  computePathLoss(devs, stepTime);

  Simulator::Stop(simulationTime);
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

void computePathLoss(NetDeviceContainer devs, double stepTime) {
  auto channel = DynamicCast<MmWaveVehicularNetDevice>(devs.Get(0))
                     ->GetPhy()
                     ->GetSpectrumPhy()
                     ->GetSpectrumChannel();
  PointerValue plm;
  channel->GetAttribute("PropagationLossModel", plm);
  Ptr<MmWaveVehicularPropagationLossModel> pathloss =
      DynamicCast<MmWaveVehicularPropagationLossModel>(
          plm.Get<PropagationLossModel>());

  Ptr<MobilityModel> mobileNode0 =
      devs.Get(0)->GetNode()->GetObject<MobilityModel>();
  Ptr<MobilityModel> mobileNode1 =
      devs.Get(1)->GetNode()->GetObject<MobilityModel>();
  Vector3D pos = mobileNode0->GetPosition();
  Vector3D pos1 = mobileNode1->GetPosition();

  double pathLossVal = pathloss->GetLoss(mobileNode1, mobileNode0);
  std::cout << "\n The value of the path loss is: " << pathLossVal << std::endl;
  double distance3D = mobileNode1->GetDistanceFrom(mobileNode0);
  double weatherAtten =
      pathloss->GetWeatherAttenuation(distance3D, pos.z, pos1.z);
  std::cout << "\n The additional value of the weather attenuation is: "
            << weatherAtten << std::endl;
  Simulator::Schedule(Seconds(stepTime), &computePathLoss, devs, stepTime);
}
