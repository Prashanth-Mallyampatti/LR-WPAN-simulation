#include <ns3/test.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/lr-wpan-error-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/lr-wpan-net-device.h>
#include <ns3/spectrum-value.h>
#include <ns3/lr-wpan-spectrum-value-helper.h>
#include <ns3/lr-wpan-mac.h>
#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/mac16-address.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/uinteger.h>
#include <ns3/nstime.h>
#include <ns3/abort.h>
#include <ns3/command-line.h>
#include <ns3/gnuplot.h>
#include <bits/stdc++.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;
using namespace std;
static uint32_t g_received = 0;

NS_LOG_COMPONENT_DEFINE ("LrWpanErrorDistancePlot");

static void LrWpanErrorDistanceCallback (McpsDataIndicationParams params, Ptr<Packet> p)
{
  g_received++;
}

static void plotPacketSuccessRate(int sender, int g_received, int maxPackets, int nSenders, Gnuplot psrplot, Gnuplot2dDataset psrdataset)
{
  std::ostringstream os;
  std::ofstream berfile ("802.15.4-psr-distance.plt");
  psrplot.AddDataset (psrdataset);

	psrdataset.SetStyle (Gnuplot2dDataset::POINTS); 
  psrplot.SetTitle (os.str ());
  psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  psrplot.SetLegend ("distance (m)", "Packet Success Rate (PSR)");
  psrplot.SetExtra  ("set xrange [0:200]\n\
 set yrange [0:1]\n\
 set grid\n\
 set style line 1 linewidth 5\n\
 set style increment user");
  psrplot.GenerateOutput (berfile);
  berfile.close ();
}

static void plotSendersPosition(int x[], int y[], int z[], int nSenders)
{
	string fileNameWithNoExtension = "plot-3d-senderPosition";
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
	plot.AppendExtra ("set xrange [0:+200]");
	plot.AppendExtra ("set yrange [0:+200]");
	plot.AppendExtra ("set zrange [0:+200]");
	
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
  std::ostringstream os;
  Gnuplot psrplot = Gnuplot ("802.15.4-psr-distance.eps");
  Gnuplot2dDataset psrdataset ("802.15.4-psr-vs-distance");

  int minDistance = 1;
  int maxDistance = 200;  // meters
  int maxPackets = 100;
  int packetSize = 20;		// bytes
  double txPower = 0;
  uint32_t channelNumber = 11;
	int nSenders = 5;

  CommandLine cmd;

  cmd.AddValue ("txPower", "transmit power (dBm)", txPower);
  cmd.AddValue ("packetSize", "packet (MSDU) size (bytes)", packetSize);
  cmd.AddValue ("channelNumber", "channel number", channelNumber);
	cmd.AddValue ("nSenders", "number of senders", nSenders);
	cmd.AddValue ("maxDistance", "maximum distance between sender and receiver (in meters)", maxDistance);
	cmd.AddValue ("minDistance", "minimum distance between sender and receiver (in meters)", minDistance);
	cmd.AddValue ("packetSize", "size of packet to be sent", packetSize);

  cmd.Parse (argc, argv);

  os << "Packet (MSDU) size = " << packetSize << " bytes; tx power = " << txPower << " dBm; channel = " << channelNumber;

// -------------------------------------- //
	Ptr<Node> node_obj[nSenders + 1];
	for(int i = 0; i < nSenders + 1; i++)
	{
  	node_obj[i] = CreateObject <Node> ();
	}
  
// -------------------------------------- //
	Ptr<LrWpanNetDevice> net_dev[nSenders + 1];
	for(int i = 0; i < nSenders+1; i++)
	{
		net_dev[i] = CreateObject<LrWpanNetDevice> ();
	}

// -------------------------------------- //
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
				net_dev[k]->SetAddress (Mac16Address (macAddress));
			k = k + 1;
		}
	}

// -------------------------------------- //
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();
  channel->AddPropagationLossModel (model);
	for(int i = 0 ; i < nSenders + 1; i++)
	{	
		net_dev[i]->SetChannel (channel);
  	node_obj[i]->AddDevice (net_dev[i]);
	}

// -------------------------------------- //
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

// -------------------------------------- //
  LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd[nSenders+1];
	for(int i = 0; i < nSenders+1; i++)
	{
		psd[i] = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
 		net_dev[i]->GetPhy ()->SetTxPowerSpectralDensity (psd[i]);
	}

// -------------------------------------- //
	McpsDataIndicationCallback cbe;
	cbe = MakeCallback (&LrWpanErrorDistanceCallback);
	net_dev[0]->GetMac ()->SetMcpsDataIndicationCallback (cbe);

// -------------------------------------- //
	// Destination: Node 0
	int i = 0, j = 0;
	char* dstMacAddress = new char[to_string(i).length() + to_string(j).length()];
	sprintf(dstMacAddress, "%02X:%02X", i, j);

// -------------------------------------- //
  McpsDataRequestParams params;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address ("00:00");
  params.m_msduHandle = 0;
  params.m_txOptions = 0;

// -------------------------------------- //
  Ptr<Packet> p1;
	int last_g_received = 0;
	for(int i = 1; i < nSenders + 1; i++)
	{
	 	for(int j = 0; j < maxPackets; j++)
		{
			p1 = Create<Packet> (packetSize);
  		Simulator::Schedule (Seconds (0.0),
                                  &LrWpanMac::McpsDataRequest,
                                  net_dev[i]->GetMac (), params, p1);
		}
		last_g_received = g_received;
    Simulator::Run ();
		psrdataset.Add (i, (g_received - last_g_received) / (1.0 * maxPackets));
	}

// -------------------------------------- //
	float packetsSent = maxPackets * nSenders;
	NS_LOG_UNCOND("Packets sent (all senders): " << packetsSent);
	NS_LOG_UNCOND("Packets received: " << g_received);
	float packetLoss = ((packetsSent - g_received) * 100.00) / packetsSent;
	NS_LOG_UNCOND("Packet Loss: " << packetLoss << "%");

// -------------------------------------- //
	plotSendersPosition(x, y, z, nSenders);
	plotPacketSuccessRate(i, g_received, maxPackets, nSenders, psrplot, psrdataset);
  system("gnuplot 802.15.4-psr-distance.plt");
  system("gnuplot plot-3d-senderPosition.plt");
	Simulator::Destroy ();
  return 0;
}
