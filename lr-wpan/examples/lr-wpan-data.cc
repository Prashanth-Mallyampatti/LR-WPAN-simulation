#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/packet.h>

#include <iostream>
#include <string>

#include <ns3/test.h>
#include <ns3/callback.h>
#include <ns3/lr-wpan-error-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/lr-wpan-net-device.h>
#include <ns3/spectrum-value.h>
#include <ns3/lr-wpan-spectrum-value-helper.h>
#include <ns3/lr-wpan-mac.h>
#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/mac16-address.h>
#include <ns3/uinteger.h>
#include <ns3/nstime.h>
#include <ns3/abort.h>
#include <ns3/command-line.h>
#include <ns3/gnuplot.h>
#include <bits/stdc++.h> 

#include <fstream>
#include <vector>
using namespace ns3;
using namespace std;

static uint32_t g_received = 0;

NS_LOG_COMPONENT_DEFINE ("LrWpanErrorDistancePlot");

int sender = 5;
static void LrWpanErrorDistanceCallback (McpsDataIndicationParams params, Ptr<Packet> p)
{
	g_received++;
}

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p)
{
  NS_LOG_UNCOND ("Received packet of size " << p->GetSize ());
}

static void DataConfirm (McpsDataConfirmParams params1)
{
  NS_LOG_UNCOND ("LrWpanMcpsDataConfirmStatus = " << params1.m_status);
}

static void StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState)
{
  NS_LOG_UNCOND (context << " state change at " << now.GetSeconds ()
                         << " from " << LrWpanHelper::LrWpanPhyEnumerationPrinter (oldState)
                         << " to " << LrWpanHelper::LrWpanPhyEnumerationPrinter (newState));
}

static void plotSendersPosition(int x[], int y[], int z[], int nSenders)
{
string fileNameWithNoExtension = "plot-3d";
string graphicsFileName        = fileNameWithNoExtension + ".png";
string plotFileName            = fileNameWithNoExtension + ".plt";
string plotTitle               = "3-D Plot";
string dataTitle               = "3-D Data";
// Instantiate the plot and set its title.
Gnuplot plot (graphicsFileName);
plot.SetTitle (plotTitle);

// Make the graphics file, which the plot file will create when it
// is used with Gnuplot, be a PNG file.
plot.SetTerminal ("png");

// Rotate the plot 30 degrees around the x axis and then rotate the
// plot 120 degrees around the new z axis.
plot.AppendExtra ("set view 30, 120, 1.0, 1.0");

// Make the zero for the z-axis be in the x-axis and y-axis plane.
plot.AppendExtra ("set ticslevel 0");

// Set the labels for each axis.
plot.AppendExtra ("set xlabel 'X Values'");
plot.AppendExtra ("set ylabel 'Y Values'");
plot.AppendExtra ("set zlabel 'Z Values'");

// Set the ranges for the x and y axis.
plot.AppendExtra ("set xrange [0:+100]");
plot.AppendExtra ("set yrange [0:+100]");
plot.AppendExtra ("set zrange [0:+100]");

// Instantiate the dataset, set its title, and make the points be
// connected by lines.
Gnuplot3dDataset dataset;
dataset.SetTitle (dataTitle);
dataset.SetStyle ("with lines");
for(int i = 0; i < nSenders + 1; i++)
{
	dataset.Add (x[i], y[i], z[i]);
	dataset.AddEmptyLine ();
}
#pragma GCC diagnostic pop

// Add the dataset to the plot.
plot.AddDataset (dataset);

// Open the plot file.
ofstream plotFile (plotFileName.c_str());

// Write the plot file.
plot.GenerateOutput (plotFile);

// Close the plot file.
plotFile.close ();
}

int main (int argc, char *argv[])
{
  bool verbose = false;
  bool extended = false;
	int nSenders = 5;
	int maxDistance = 100;	// meters
	int minDistance = 1;		// meters
	int packetSize = 50;
	double txPower = 0;
	uint32_t channelNumber = 11;
	
  CommandLine cmd;

  cmd.AddValue ("verbose", "turn on all log components", verbose);
  cmd.AddValue ("extended", "use extended addressing", extended);
	cmd.AddValue ("nSenders", "Number of Senders", nSenders);
	cmd.AddValue ("txPower", "transmit power (dbm)", txPower);
	cmd.AddValue ("packetSize", "packet (MSDU) size (bytes)", packetSize);
	cmd.AddValue ("channelNumber", "channel number", channelNumber);
	cmd.AddValue ("minDistance", "minimum distance between sender and receiver", minDistance);
	cmd.AddValue ("maxDistance", "maximum distance between sender and receiver", maxDistance);

  cmd.Parse (argc, argv);
	
	Time::SetResolution (Time::NS);

  LrWpanHelper lrWpanHelper;
  if (verbose)
    {
      lrWpanHelper.EnableLogComponents ();
    }

  // Enable calculation of FCS in the trailers. Only necessary when interacting with real devices or wireshark.
  // GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  // Create n nodes, and NetDevice for each one
	// Make the first Node as Reciever
	Ptr<Node> node_obj[nSenders + 1];
	for(int i = 0; i < nSenders + 1; i++)
	{
  	node_obj[i] = CreateObject <Node> ();
	}
  
	Ptr<LrWpanNetDevice> net_dev[nSenders + 1];
	for(int i = 0; i < nSenders+1; i++)
	{
		net_dev[i] = CreateObject<LrWpanNetDevice> ();
	}

	int k = 0;
	for(int i = 0; i < ((nSenders+1) / 256) + 1; i++)
	{
		int n = 0;
		if (i == (nSenders+1) / 256)
			n = (nSenders+1) % 256;
		else
			n = 256;

		for(int j = 0; j < n; j++)
		{
			char* macAddress = new char[to_string(i).length() + to_string(j).length()];
			sprintf(macAddress, "%02X:%02X", i, j);
			if (!extended)
			{
				net_dev[k]->SetAddress (Mac16Address (macAddress));
			}
			else
			{
				cout << "64 Byte currently not supported." << endl;
				exit(0);
			}
			k = k + 1;
		}
	}
  // Each device must be attached to the same channel
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->AddPropagationLossModel (propModel);
  channel->SetPropagationDelayModel (delayModel);

	for(int i = 0 ; i < nSenders + 1; i++)
	{	
		net_dev[i]->SetChannel (channel);
	}
  // To complete configuration, a LrWpanNetDevice must be added to a node
	for(int i = 0 ; i < nSenders + 1; i++)
	{
  	node_obj[i]->AddDevice (net_dev[i]);
	}

  // Trace state changes in the phy
	for(int i = 0; i < nSenders + 1; i++)
	{
		std::string phy = "phy" + std::to_string(i);
  	net_dev[i]->GetPhy ()->TraceConnect ("TrxState", std::string (phy), MakeCallback (&StateChangeNotification));
  }
	// Set sender mobilities
	Ptr<ConstantPositionMobilityModel> senderMobility[nSenders + 1];
	int x[nSenders + 1];
	int y[nSenders + 1];
	int z[nSenders + 1];
	
	for(int i = 0; i < nSenders + 1; i++)
	{
		senderMobility[i] = CreateObject<ConstantPositionMobilityModel> ();
		x[i] = rand() % maxDistance + minDistance;
		y[i] = rand() % maxDistance + minDistance;
		z[i] = rand() % maxDistance + minDistance;
  	senderMobility[i]->SetPosition (Vector (x[i], y[i], z[i]));
   	net_dev[i]->GetPhy ()->SetMobility (senderMobility[i]);
	}

   LrWpanSpectrumValueHelper svh;
   Ptr<SpectrumValue> psd[nSenders];
		for(int i = 0; i < nSenders; i++)
		{
 			psd[i] = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
   		net_dev[i]->GetPhy ()->SetTxPowerSpectralDensity (psd[i]);
		}
	plotSendersPosition(x, y, z, nSenders);

	McpsDataConfirmCallback cbc[nSenders + 1];
	McpsDataIndicationCallback cbi[nSenders + 1];
	McpsDataIndicationCallback cbe;
	for(int i = 0; i < nSenders + 1; i++)
	{
		//sender = i;
  	cbc[i] = MakeCallback (&DataConfirm);
  	net_dev[i]->GetMac ()->SetMcpsDataConfirmCallback (cbc[i]);
		cbe = MakeCallback (&LrWpanErrorDistanceCallback);
  	net_dev[1]->GetMac ()->SetMcpsDataIndicationCallback (cbe);
  	cbi[i] = MakeCallback (&DataIndication);
  	net_dev[i]->GetMac ()->SetMcpsDataIndicationCallback (cbi[i]);
	}

	// Tracing
  lrWpanHelper.EnablePcapAll (std::string ("LR-wpan-trace/PCAPs/LR-wpan-data"), true);
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("LR-wpan-trace/lr-wpan-data.tr");
  lrWpanHelper.EnableAsciiAll (stream);

  // The below should trigger two callbacks when end-to-end data is working
  // 1) DataConfirm callback is called
  // 2) DataIndication callback is called with value of 50
	Ptr<Packet> p[nSenders];
	for(int i = 0 ; i < nSenders; i++)
	{
		p[i] = Create<Packet> (packetSize);			// Dummy data
	}
	
	McpsDataRequestParams params[nSenders];
	for(int i = 0; i < nSenders; i++)
	{
		params[i].m_dstPanId = 0;
	}

	// Destination setting to Node 0
	int i = 0, j = 0;
	char* dstMacAddress = new char[to_string(i).length() + to_string(j).length()];
	sprintf(dstMacAddress, "%02X:%02X", i, j);
	
	
  for(int i = 0 ; i < nSenders; i++)
	{
		if (!extended)
  	{
      params[i].m_srcAddrMode = SHORT_ADDR;
      params[i].m_dstAddrMode = SHORT_ADDR;
      params[i].m_dstAddr = Mac16Address (dstMacAddress);
			//params1[i].m_Sender = i;
  	}
  	else
 		{
			// Not supported
			// params0.m_srcAddrMode = EXT_ADDR;
      // params0.m_dstAddrMode = EXT_ADDR;
      // params0.m_dstExtAddr = Mac64Address ("00:00:00:00:00:00:00:02");
    }
  	params[i].m_msduHandle = 0;
  	params[i].m_txOptions = TX_OPTION_ACK;
	}
//  dev0->GetMac ()->McpsDataRequest (params, p0);

  Ptr<Packet> p1;
	for(int i = 1; i < nSenders + 1; i++)
	{
		sender = i;
	 	for(int j = 0; j < 2; j++)
		{
			p1 = Create<Packet> (packetSize);
  		Simulator::Schedule (Seconds (0.0),
                                  &LrWpanMac::McpsDataRequest,
                                  net_dev[i]->GetMac (), params[i-1], p1);
		}
  	Simulator::Run ();
	}
	system("gnuplot plot-3d.plt");
  Simulator::Destroy ();
  return 0;
}
