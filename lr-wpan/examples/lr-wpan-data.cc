/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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
 * Author:  Tom Henderson <thomas.r.henderson@boeing.com>
 */

/*
 * Try to send data end-to-end through a LrWpanMac <-> LrWpanPhy <->
 * SpectrumChannel <-> LrWpanPhy <-> LrWpanMac chain
 *
 * Trace Phy state changes, and Mac DataIndication and DataConfirm events
 * to stdout
 */
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

static void plotSendersPosition(int x[], int y[], int z[])
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
plot.AppendExtra ("set xrange [0:+200]");
plot.AppendExtra ("set yrange [0:+200]");
plot.AppendExtra ("set zrange [0:+200]");

// Instantiate the dataset, set its title, and make the points be
// connected by lines.
Gnuplot3dDataset dataset;
dataset.SetTitle (dataTitle);
dataset.SetStyle ("with lines");
for(size_t i = 0; i < sizeof(x)/sizeof(x[0]); i++)
{
	for(size_t j = 0; j < sizeof(y)/sizeof(y[0]); j++)
	{
		for(size_t k = 0; k < sizeof(z)/sizeof(z[0]); k++)
		{
			dataset.Add (x[i], y[j], z[k]);
			dataset.AddEmptyLine ();
		}
	}
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

void static plotErrorDistance(int i, std::ostringstream& os, std::ofstream& berfile, Gnuplot psrplot, Gnuplot2dDataset psrdataset, int mP, uint32_t g_Received)
{
		NS_LOG_UNCOND ("Received " << g_Received << " packets for sender " << i);

		double maxPackets = 1.0 * mP;
		psrdataset.Add(i, g_Received / maxPackets);
		g_Received = 0;
	  
		psrplot.AddDataset (psrdataset);
 
//   	psrplot.SetTitle (os.str ());
//   	psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
//   	psrplot.SetLegend ("distance (m)", "Packet Success Rate (PSR)");
//   	psrplot.SetExtra  ("set xrange [0:200]\n\
// 	 	set yrange [0:1]\n\
//		set grid\n\
// 		set style line 1 linewidth 5\n\
// 		set style increment user");
//   	psrplot.GenerateOutput (berfile);
//   	berfile.close ();	
}

int main (int argc, char *argv[])
{
  std::ostringstream os;
  std::ofstream berfile ("802.15.4-psr-distance.plt");
  bool verbose = false;
  bool extended = false;
	int nSenders = 5;
	int maxDistance = 100;	// meters
	int minDistance = 1;		// meters
	int packetSize = 50;
	int maxPackets = 10;
	double txPower = 0;
	uint32_t channelNumber = 11;
	
  CommandLine cmd;

  cmd.AddValue ("verbose", "turn on all log components", verbose);
  cmd.AddValue ("extended", "use extended addressing", extended);
	cmd.AddValue ("nSenders", "Number of Senders", nSenders);
	cmd.AddValue ("txPower", "transmit power (dbm)", txPower);
	cmd.AddValue ("packetSize", "packet (MSDU) size (bytes)", packetSize);
	cmd.AddValue ("channelNumber", "channel number", channelNumber);

  cmd.Parse (argc, argv);
	
	os << "Packet (MSDU) size = " << packetSize << " bytes; tx power = " << txPower << " dBm; channel = " << channelNumber;
	Gnuplot psrplot = Gnuplot ("802.15.4-psr-distance.eps");
	Gnuplot2dDataset psrdataset ("802.15.4-psr-distance");
	
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
  //channel->SetPropagationDelayModel (delayModel);

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
  	//net_dev[i]->GetPhy ()->TraceConnect ("TrxState", std::string (phy), MakeCallback (&StateChangeNotification));
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
	McpsDataConfirmParams params1[nSenders];
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
  	params[i].m_txOptions = 0;//TX_OPTION_ACK;
	}
//  dev0->GetMac ()->McpsDataRequest (params, p0);

  Ptr<Packet> p1;
  //uint32_t g_received[nSenders] = {0};
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
		NS_LOG_UNCOND(g_received << " received");
//		plotErrorDistance(i, os, berfile, psrplot, psrdataset, maxPackets, g_received[i-1]);
		
	}
//	Simulator::Run();
//	system("gnuplot plot-3d.plt");
  Simulator::Destroy ();
  return 0;
}
