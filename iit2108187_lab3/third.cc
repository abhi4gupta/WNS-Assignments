/*
 * Author: Abhishek Kumar Gupta
 * roll no.: iit2018187
 * Wireless Networks Lab Assignment 3
 */


/* problem Statement
  
  Objective: Learning Queues, packet drops and their effect on congestion window size.
Experiment:
1. As in Assignment #2, Create a simple dumbbell topology, two client Node1 and Node2 on the left side of the dumbbell and server nodes Node3 and Node4 on the right side of the dumbbell. Let Node5 and Node6 form the bridge of the dumbbell. Use point to point links.
2. Add drop tail queues of size QueueSize5 and QueueSize6 to Node5 and Node6, respectively.
3. Install a TCP socket instance on Node1 that will connect to Node3.
4. Install a TCP socket instance on Node2 that will connect to Node3.
5. Install a TCP socket instance on Node2 that will connect to Node4.
6. Start Node1--Node3 flow at time 1s, then measure its throughput. How long does it take to fill link's entire capacity?
7. Start Node2--Node3 and Node2--Node4 flows at time 15s, measure their throughput.
8. Measure packet loss and cwnd size, and plot graphs throughput/time, cwnd/time and packet loss/time for each of the flows.
9. Plot graph throughput/cwnd and packet loss/cwnd for the first flow. Is there an optimal value for cwnd?
10.Vary QueueSize5 and QueueSize6. Which one has immediate effect on cwnd size of the first flow? Explain why.

*/




#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Lab4-3");


ofstream cwnd[5]; // Congestion Windows filestream
ofstream queueFile; // Queue size filestream
ofstream recvfile[5]; // Receiver Rates filesteam
int packetCount=0; //Current Packet Count
int totalLength=0; 
int packetSize[10];
int sumPacketSize,queueSize;
double initialTime[10],finalTime,diffTime;

/**
 * Class which will act as a tcp source. We will hook a congestion tracer with its tcp connection.
 */
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate);

private:
  virtual void StartApplication (void);  
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void 
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  ScheduleTx ();
}

void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}


/**
 * Following 5 functions are congestion windows hooks for each of the tcp connections
 */
static void
CwndTracer1(uint32_t oldval, uint32_t newval)
{
  cwnd[0]<<Simulator::Now().GetSeconds()<<" "<<newval<<endl;
}

static void
CwndTracer2(uint32_t oldval, uint32_t newval)
{
  cwnd[1]<<Simulator::Now().GetSeconds()<<" "<<newval<<endl;
}

static void
CwndTracer3(uint32_t oldval, uint32_t newval)
{
  cwnd[2]<<Simulator::Now().GetSeconds()<<" "<<newval<<endl;
}

static void
CwndTracer4(uint32_t oldval, uint32_t newval)
{
  cwnd[3]<<Simulator::Now().GetSeconds()<<" "<<newval<<endl;
}

static void
CwndTracer5(uint32_t oldval, uint32_t newval)
{
  cwnd[4]<<Simulator::Now().GetSeconds()<<" "<<newval<<endl;
}

/**
 * Node 0 Enque Hook
 */
static void
Enqueue(string context, Ptr<const Packet> p)
{
  queueSize++;
  queueFile<<Simulator::Now ().GetSeconds()<<"\t EQ \t"<<queueSize<<endl;
}

/**
 * Node 0 Deque Hook
 */
static void
Dequeue(string context, Ptr<const Packet> p)
{
  queueSize--;
  queueFile<<Simulator::Now ().GetSeconds()<<"\t DQ \t"<<queueSize<<endl;

}

static void
Drop(string context, Ptr<const Packet> p)
{
  queueFile<<Simulator::Now ().GetSeconds()<<"\t DR \t"<<queueSize<<endl;
}


/**
 * Node - 1 Receive Packet Hook
 */
static void
ReceivePacket (string context, Ptr<const Packet> p, const Address& addr)
{
        int currentPacketPort;        
        currentPacketPort = InetSocketAddress::ConvertFrom(addr).GetPort();        
        packetCount++;
        if(packetCount<10) // To handle the initial 10 packets
        {
                initialTime[packetCount]= Simulator::Now ().GetSeconds();
                packetSize[packetCount]= p->GetSize();        
        }
        else
        {                  
                finalTime = Simulator::Now ().GetSeconds();       
                diffTime = finalTime - initialTime[packetCount%10];                               
                sumPacketSize=0;                
                for(int i=0;i<10;i++) sumPacketSize+=packetSize[i];

                totalLength=sumPacketSize/diffTime;


              switch(currentPacketPort){
                case 49153:                
                  recvfile[0]<<finalTime<<"\t"<<totalLength<<endl;
                  break;
                case 49154:                
                  recvfile[1]<<finalTime<<"\t"<<totalLength<<endl;
                  break;
                 case 49155:                
                  recvfile[2]<<finalTime<<"\t"<<totalLength<<endl;
                  break;
                case 49156:                
                  recvfile[3]<<finalTime<<"\t"<<totalLength<<endl;
                  break;
                case 49157:                
                  recvfile[4]<<finalTime<<"\t"<<totalLength<<endl;
                  break;
              }
                        
                initialTime[packetCount%10]=finalTime;
                packetSize[packetCount%10]=p->GetSize();                
        }
}


int 
main (int argc, char *argv[])
{

  /**
   * File Input Output Code
   */
  string fileprefix = "Cwnd";
  string recvprefix = "Recv";
  for (int i = 0; i < 5; ++i)
  {
    stringstream ss;
    ss << i;
    string str = ss.str();
    string filename = fileprefix + str + ".dat";
    cwnd[i].open(filename.c_str());

    
     filename = recvprefix + str + ".dat";
    recvfile[i].open(filename.c_str());
  }
  queueFile.open("queue.dat");


  /**
   * Preparing the simulator
   */
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Set the size of the sending queue
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue(uint32_t(1000)));
  
  // Uncomment the below statement to enable logging
  //LogComponentEnable("Lab4-3", LOG_LEVEL_INFO); 
  
  std::string tcpType = "NewReno";

  // Command Line parsing
  CommandLine cmd;
  cmd.AddValue ("Tcp", "Tcp type: 'NewReno' or 'Tahoe'", tcpType);
  cmd.Parse (argc, argv);

  // Set the TCP Socket Type
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName ("ns3::Tcp" + tcpType)));

  NS_LOG_INFO ("Creating Topology");

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", DataRateValue(DataRate("10Mbps")));
  pointToPoint.SetChannelAttribute("Delay", TimeValue(Time("10ms")));
  pointToPoint.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(1000));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("172.16.24.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  uint16_t sinkPort = 9000;

  /**
   * Creating 5 tcp sinks at node 1
   */
  for (int i = 0; i < 5; ++i)
  {
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort+i));
      ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (50.));
  }
  
  // First tcp source
  Address sinkAddress (InetSocketAddress(interfaces.GetAddress (1), sinkPort));
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer1));
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 2000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (0.));
  app->SetStopTime (Seconds (50.));


  // Second tcp source
  Address sinkAddress1 (InetSocketAddress(interfaces.GetAddress (1), sinkPort+1));
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket2->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer2));
  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  app2->Setup (ns3TcpSocket2, sinkAddress1, 2000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app2);
  app2->SetStartTime (Seconds (5.));
  app2->SetStopTime (Seconds (45.));


  // third tcp source
  Address sinkAddress2 (InetSocketAddress(interfaces.GetAddress (1), sinkPort+2));
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket3->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer3));
  Ptr<MyApp> app3 = CreateObject<MyApp> ();
  app3->Setup (ns3TcpSocket3, sinkAddress2, 2000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app3);
  app3->SetStartTime (Seconds (10.));
  app3->SetStopTime (Seconds (40.));

  // fourth tcp source
  Address sinkAddress3 (InetSocketAddress(interfaces.GetAddress (1), sinkPort+3));
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket4->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer4));
  Ptr<MyApp> app4 = CreateObject<MyApp> ();
  app4->Setup (ns3TcpSocket4, sinkAddress3, 2000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app4);
  app4->SetStartTime (Seconds (15.));
  app4->SetStopTime (Seconds (35.));

  // fifth tcp source
  Address sinkAddress4 (InetSocketAddress(interfaces.GetAddress (1), sinkPort+4));
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket5->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer5));
  Ptr<MyApp> app5 = CreateObject<MyApp> ();
  app5->Setup (ns3TcpSocket5, sinkAddress4, 2000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app5);
  app5->SetStartTime (Seconds (20.));
  app5->SetStopTime (Seconds (30.));
  
  // Node 0 p2p device tx queue context
  std::string context = "/NodeList/0/DeviceList/0/$ns3::PointToPointNetDevice/TxQueue/";
  
  // Attaching hooks.
  Config::Connect (context + "Enqueue", MakeCallback (&Enqueue));
  Config::Connect (context + "Dequeue", MakeCallback (&Dequeue));
  Config::Connect (context + "Drop", MakeCallback (&Drop));
  
  // Node 1 all tcp receiver hook
  context = "/NodeList/1/ApplicationList/*/$ns3::PacketSink/Rx";
  Config::Connect (context, MakeCallback(&ReceivePacket));
  
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("lab4-3.tr"));
  pointToPoint.EnablePcapAll("lab4-3", false);

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();  

  Simulator::Stop (Seconds(50));
  Simulator::Run ();

  // Flowmonitor Analysis
  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
      {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        if ((t.sourceAddress=="172.16.24.1" && t.destinationAddress == "172.16.24.2"))
        {
            std::cout << "Flow " << (i->first)/2 + 1  << " (" << t.sourceAddress<<":"<<t.sourcePort << " -> " << t.destinationAddress <<":"<<t.destinationPort<< ")\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
        }
       }

  Simulator::Destroy ();


  // Closing Down files.
  for (int i = 0; i < 5; ++i)
  {
    cwnd[i].close();
    recvfile[i].close();
  }

  queueFile.close();
  return 0;
}