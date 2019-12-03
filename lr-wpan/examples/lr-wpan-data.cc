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
using namespace ns3;
using namespace std;

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p)
{
  NS_LOG_UNCOND ("Received packet of size " << p->GetSize ());
}

static void DataConfirm (McpsDataConfirmParams params)
{
  NS_LOG_UNCOND ("LrWpanMcpsDataConfirmStatus = " << params.m_status);
}

static void StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState)
{
  NS_LOG_UNCOND (context << " state change at " << now.GetSeconds ()
                         << " from " << LrWpanHelper::LrWpanPhyEnumerationPrinter (oldState)
                         << " to " << LrWpanHelper::LrWpanPhyEnumerationPrinter (newState));
}

int main (int argc, char *argv[])
{
  bool verbose = false;
  bool extended = false;
	int nSenders = 5;

  CommandLine cmd;

  cmd.AddValue ("verbose", "turn on all log components", verbose);
  cmd.AddValue ("extended", "use extended addressing", extended);
	cmd.AddValue ("nSenders", "Number of Senders", nSenders);

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
				net_dev[i]->SetAddress (Mac16Address (macAddress));
			}
			else
			{
				cout << "64 Byte currently not supported." << endl;
				exit(0);
			}
		}
	}
  // Each device must be attached to the same channel
  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
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
	for(int i = 0; i < nSenders + 1; i++)
	{
		senderMobility[i] = CreateObject<ConstantPositionMobilityModel> ();
		int x = rand() % 101 + 0;
		int y = rand() % 101 + 0;
		int z = rand() % 101 + 0;
  	senderMobility[i]->SetPosition (Vector (x, y, z));
   	net_dev[i]->GetPhy ()->SetMobility (senderMobility[i]);
	}

	 
	McpsDataConfirmCallback cbc[nSenders + 1];
	McpsDataIndicationCallback cbi[nSenders + 1];
	for(int i = 0; i < nSenders + 1; i++)
	{
  	cbc[i] = MakeCallback (&DataConfirm);
  	net_dev[i]->GetMac ()->SetMcpsDataConfirmCallback (cbc[i]);
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
		p[i] = Create<Packet> (50);			// Dummy data
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

	for(int i = 1; i < nSenders + 1; i++)
	{
  	Simulator::ScheduleWithContext (1, Seconds (0.0),
                                  &LrWpanMac::McpsDataRequest,
                                  net_dev[i]->GetMac (), params[i-1], p[i-1]);
	}

/*  // Send a packet back at time 2 seconds
  Ptr<Packet> p2 = Create<Packet> (60);  // 60 bytes of dummy data
  if (!extended)
    {
      params.m_dstAddr = Mac16Address ("00:01");
    }
  else
    {
      params.m_dstExtAddr = Mac64Address ("00:00:00:00:00:00:00:01");
    }
  Simulator::ScheduleWithContext (2, Seconds (2.0),
                                  &LrWpanMac::McpsDataRequest,
                                  dev1->GetMac (), params, p2);
*/
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
