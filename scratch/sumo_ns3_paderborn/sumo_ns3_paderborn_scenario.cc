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
#include <sys/stat.h>

using namespace ns3;
using namespace millicar;

Ptr<TraciClient> sumoClient = CreateObject<TraciClient>();
Ptr<MmWaveVehicularHelper> helper = CreateObject<MmWaveVehicularHelper>();
bool combined_rain_snow;

int main(int argc, char *argv[]) {

  double frequency = 60e9;  // frequency in Hz
  double bandwidth = 2.5e8; // bandwidth in Hz
  uint32_t simulationRun;   // number of simulation
  int intensityOfRain;      // rain intensity in mm/h
  double k; // regression coffiecient for computing specific rain attenuation
            // (horizontal/vertical polarization)
  double alpha;    // regression coffiecient for computing specific rain
                   // attenuation (horizontal/vertical polarization)
  double altitude; // altitude in meters above the sea level of Paderborn
  double h0;       // mean annual 0C isotherm height above mean sea level
  std::string channel_condition;
  std::string scenario;
  ns3::Time simulationTime(ns3::Seconds(200));

  CommandLine cmd;

  cmd.AddValue("simulationRun", "Defines the number of simulations",
               simulationRun);
  cmd.AddValue("intensityOfRain", "Specifies the value of rain intensity",
               intensityOfRain);
  cmd.AddValue("combined_rain_snow",
               "Defines weather the attenuation from combined rain and wet "
               "snow should be computed",
               combined_rain_snow);
  cmd.AddValue("channel_condition", "Defines the channel condition",
               channel_condition);
  std::cout << "\n Channel condition is " << channel_condition << std::endl;
  cmd.AddValue("scenario",
               "Defines the scenario where the communication takes place",
               scenario);
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
  Config::SetDefault("ns3::RainAttenuation::k", DoubleValue(k));
  Config::SetDefault("ns3::RainAttenuation::alpha", DoubleValue(alpha));
  Config::SetDefault("ns3::RainSnowAttenuation::altitude",
                     DoubleValue(altitude));
  Config::SetDefault("ns3::RainSnowAttenuation::h0", DoubleValue(h0));
  Config::SetDefault("ns3::MmWaveVehicularPropagationLossModel::Shadowing",
                     BooleanValue(true));
  Config::SetDefault(
      "ns3::MmWaveVehicularSpectrumPropagationLossModel::UpdatePeriod",
      TimeValue(MilliSeconds(1)));

  // SUMO Configuration

  sumoClient->SetAttribute(
      "SumoConfigPath",
      StringValue("scratch/sumo_ns3_paderborn/full-day.sumo.cfg"));
  sumoClient->SetAttribute("SumoBinaryPath",
                           StringValue("")); // use system installation of sumo
  sumoClient->SetAttribute("SynchInterval", TimeValue(Seconds(0.01)));
  sumoClient->SetAttribute("StartTime", TimeValue(Seconds(0.0)));
  sumoClient->SetAttribute("SumoPort", UintegerValue(3400));
  sumoClient->SetAttribute(
      "PenetrationRate",
      DoubleValue(1.0)); // portion of vehicles equipped with wifi
  sumoClient->SetAttribute("SumoLogFile", BooleanValue(true));
  sumoClient->SetAttribute("SumoStepLog", BooleanValue(false));
  sumoClient->SetAttribute("SumoSeed", IntegerValue(10));
  sumoClient->SetAttribute("SumoAdditionalCmdOptions",
                           StringValue("--fcd-output sumo_paderborn.xml"));
  sumoClient->SetAttribute("SumoWaitForSocket", TimeValue(Seconds(1.0)));
  sumoClient->SetAttribute("SumoGUI", BooleanValue(true));

  VehicleSpeedControlHelper vehicleSpeedControlHelper(9);
  vehicleSpeedControlHelper.SetAttribute("Client", (PointerValue)sumoClient);

  std::function<Ptr<Node>()> setupNewWifiNode = [&]() -> Ptr<Node> {
    NodeContainer n;
    Ptr<Node> newNode = CreateObject<Node>();
    n.Add(newNode);

    MobilityHelper mobility; // create the mobility nodes
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(n);

    helper->SetPropagationLossModelType(
        "ns3::MmWaveVehicularPropagationLossModel");
    helper->SetSpectrumPropagationLossModelType(
        "ns3::MmWaveVehicularSpectrumPropagationLossModel");
    helper->SetNumerology(3);

    NetDeviceContainer devs = helper->InstallMmWaveVehicularNetDevices(n);

    InternetStackHelper internet;
    internet.Install(n);

    ApplicationContainer vehicleSpeedControlApps =
        vehicleSpeedControlHelper.Install(n);
    vehicleSpeedControlApps.Start(Simulator::Now());
    vehicleSpeedControlApps.Stop(simulationTime);

    return newNode;
  };

  // callback function for node shutdown
  std::function<void(Ptr<Node>)> shutdownWifiNode = [](Ptr<Node> exNode) {
    std::cout << "\n Trying to shut down the application in the node "
              << sumoClient->GetVehicleId(exNode) << std::endl;

    // stop all applications
    Ptr<VehicleSpeedControl> vehicleSpeedControl =
        exNode->GetApplication(0)->GetObject<VehicleSpeedControl>();
    if (vehicleSpeedControl)
      vehicleSpeedControl->StopApplicationNow();
  };

  // start traci client with given function pointers
  sumoClient->SumoSetup(setupNewWifiNode, shutdownWifiNode);

  Simulator::Stop(simulationTime);
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
