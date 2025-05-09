#include "drr.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "traffic-class.h" // Required for Ptr<TrafficClass>
#include <fstream>         // Required for std::ifstream

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DRR");
NS_OBJECT_ENSURE_REGISTERED (DRR);

TypeId DRR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DRR")
    .SetParent<DiffServ> ()
    .SetGroupName ("Network")
    .AddConstructor<DRR> ()
    // If you want m_configFile to be an attribute settable at construction:
    // .AddAttribute ("ConfigFile", "The configuration file for DRR.",
    //                StringValue (""),
    //                MakeStringAccessor (&DRR::m_configFile), // Needs DRR::SetConfigFile to be called if set this way
    //                MakeStringChecker ())
    ;
  return tid;
}

DRR::DRR () : m_lastQueueServed (0) // Default, will be properly set in SetConfigFile
{
  NS_LOG_FUNCTION (this);
  // m_deficits and m_quantums are initialized/resized in SetConfigFile.
  // m_lastQueueServed will be set to (numQueues - 1) in SetConfigFile
  // so that the first scan properly starts from queue 0.
}

DRR::~DRR ()
{
  NS_LOG_FUNCTION (this);
}

void DRR::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_deficits.clear ();
  m_quantums.clear ();
  // Call base class DoDispose
  DiffServ::DoDispose ();
}

bool DRR::SetConfigFile (std::string filename)
{
  NS_LOG_FUNCTION (this << filename);
  m_configFile = filename; // Store filename
  std::ifstream configFileStream (filename.c_str ());
  if (!configFileStream.is_open ())
    {
      NS_LOG_ERROR ("DRR: Can't open DRR config file: " << filename);
      return false;
    }

  uint32_t numQueuesFromFile;
  configFileStream >> numQueuesFromFile;
  if (configFileStream.fail() || numQueuesFromFile == 0)
    {
      NS_LOG_ERROR ("DRR: Invalid number of queues in DRR config file: " << filename << ". Read: " << numQueuesFromFile);
      configFileStream.close();
      return false;
    }

  // Ensure the base DiffServ object has the correct number of TrafficClass objects.
  // If GetNTrafficClasses() is 0, we are setting up for the first time.
  // If GetNTrafficClasses() > 0 but different from numQueuesFromFile, it's a reconfiguration.
  // The simulation script calls GetTrafficClass(i) after SetConfigFile, so TCs must exist.
  
  // Clear existing TrafficClass objects if reconfiguring with a different number of queues.
  // This part depends heavily on how DiffServ manages its m_trafficClasses.
  // A robust DiffServ would have a ClearTrafficClasses() or SetNQueues() method.
  // Assuming AddTrafficClass appends and GetTrafficClass accesses an internal vector.
  if (GetNTrafficClasses() != 0 && GetNTrafficClasses() != numQueuesFromFile) {
      NS_LOG_WARN("DRR: Reconfiguring with a different number of queues (" << numQueuesFromFile << " vs " << GetNTrafficClasses() << "). "
                  "Behavior depends on DiffServ base class's management of TrafficClasses.");
      // Ideally, clear existing m_trafficClasses in DiffServ here.
      // For now, we proceed, DRR will operate on the first 'numQueuesFromFile' TCs.
  }

  // Ensure DiffServ has at least numQueuesFromFile TrafficClass objects.
  // Create and add new TrafficClass objects if needed.
  for (uint32_t i = GetNTrafficClasses (); i < numQueuesFromFile; ++i) {
      AddTrafficClass (CreateObject<TrafficClass> ());
  }
  
  // After this, GetNTrafficClasses() should be >= numQueuesFromFile.
  // DRR will only manage the first 'numQueuesFromFile' queues.
  if (GetNTrafficClasses() < numQueuesFromFile) {
      NS_LOG_ERROR("DRR: Failed to ensure enough TrafficClass objects in base DiffServ. Expected " << numQueuesFromFile << ", but base has " << GetNTrafficClasses());
      configFileStream.close();
      return false;
  }

  m_quantums.resize (numQueuesFromFile);
  m_deficits.resize (numQueuesFromFile);

  NS_LOG_INFO("DRR: Configuring " << numQueuesFromFile << " queues.");
  for (uint32_t i = 0; i < numQueuesFromFile; ++i)
    {
      configFileStream >> m_quantums[i];
      if (configFileStream.fail() || m_quantums[i] == 0) // Quantum must be positive
        {
          NS_LOG_ERROR ("DRR: Invalid quantum for queue " << i << " in DRR config file: " << filename);
          configFileStream.close();
          return false;
        }
      m_deficits[i] = 0; // Initialize deficit counter to 0 as per image
      NS_LOG_INFO("DRR: Queue " << i << " - Quantum: " << m_quantums[i] << ", Initial Deficit: " << m_deficits[i]);
    }
  configFileStream.close ();

  // Initialize m_lastQueueServed so that the first queue to be checked by Schedule() is queue 0.
  // If numQueuesFromFile is 1, (1-1)=0. Next is (0+1+0)%1 = 0. Correct.
  // If numQueuesFromFile > 1, (numQueuesFromFile-1). Next is ( (N-1)+1+0 )%N = 0. Correct.
  m_lastQueueServed = (numQueuesFromFile > 0) ? (numQueuesFromFile - 1) : 0;

  NS_LOG_INFO ("DRR: Configuration loaded successfully from " << filename);
  return true;
}

Ptr<Packet> DRR::Schedule (void)
{
  NS_LOG_FUNCTION (this);
  // Number of queues DRR is configured to manage
  uint32_t numManagedQueues = m_quantums.size(); 
  
  if (numManagedQueues == 0) {
      NS_LOG_LOGIC("DRR: No queues managed. Nothing to schedule.");
      return nullptr;
  }
  // Ensure base DiffServ has enough TCs for DRR to operate on.
  if (GetNTrafficClasses() < numManagedQueues) {
      NS_LOG_WARN("DRR: Mismatch between configured queues (" << numManagedQueues 
                  << ") and available TrafficClasses in base DiffServ (" << GetNTrafficClasses() << "). Cannot schedule.");
      return nullptr;
  }

  // Iterate up to numManagedQueues times to find a queue to service in this round-robin pass
  for (uint32_t i = 0; i < numManagedQueues; ++i)
    {
      // Determine current queue index to check, using round-robin
      // Starts from the queue after m_lastQueueServed
      uint32_t currentQueueIndex = (m_lastQueueServed + 1 + i) % numManagedQueues;

      Ptr<TrafficClass> tc = GetTrafficClass (currentQueueIndex); 
      if (!tc) { // Should not happen if SetConfigFile was successful
          NS_LOG_WARN("DRR: TrafficClass for queue " << currentQueueIndex << " is unexpectedly null. Skipping.");
          continue;
      }

      // If queue is empty, it cannot be served in this round. Skip it.
      // Its deficit should be 0 if it emptied correctly following the image's logic.
      if (tc->IsEmpty ())
        {
          NS_LOG_LOGIC ("DRR: Queue " << currentQueueIndex << " is empty. Skipping.");
          continue;
        }

      // This queue (currentQueueIndex) is non-empty. This corresponds to "flow i" from the image.
      // This is its "turn". Add its quantum to its deficit.
      // This matches: "DeficitCounter_i = Quantum_i + DeficitCounter_i"
      m_deficits[currentQueueIndex] += m_quantums[currentQueueIndex];
      NS_LOG_DEBUG ("DRR: Queue " << currentQueueIndex << " gets turn. Prior Deficit: " 
                    << (m_deficits[currentQueueIndex] - m_quantums[currentQueueIndex]) // Deficit before adding quantum
                    << ", Quantum: " << m_quantums[currentQueueIndex] 
                    << ". Total Deficit for round: " << m_deficits[currentQueueIndex]);

      // Image: "while ((Queue_i not empty) and (DeficitCounter_i > 0)) do"
      //        "  if (PacketSize <= DeficitCounter_i) then Send()"
      //        "  Else break"
      // Since Schedule() in ns-3 typically sends one packet per call, we check once here.
      // If a packet is sent, we update m_lastQueueServed and return.
      // If cannot send (packet too large), its accumulated deficit (OldDeficit + Quantum) is carried over.
      // The 'for' loop continues to check other queues in this *same* Schedule() invocation.

      // Check if queue is still not empty (it shouldn't have changed) and if deficit is positive
      if (!tc->IsEmpty() && m_deficits[currentQueueIndex] > 0) 
        {
          Ptr<Packet> packetToPeek = tc->Peek ();
          NS_ASSERT (packetToPeek); // Should be valid if queue is not empty
          uint32_t packetSize = packetToPeek->GetSize ();

          NS_LOG_DEBUG("DRR: Queue " << currentQueueIndex << " Peeked packet size: " << packetSize << "B. Deficit: " << m_deficits[currentQueueIndex]);

          if (packetSize <= m_deficits[currentQueueIndex])
            {
              // Sufficient deficit to send this packet
              Ptr<Packet> packetToSend = tc->Dequeue (); // Dequeue the packet
              m_deficits[currentQueueIndex] -= packetSize; // Reduce deficit

              NS_LOG_INFO ("DRR: Dequeued packet (size " << packetSize << "B) from queue " << currentQueueIndex 
                           << ". Deficit remaining: " << m_deficits[currentQueueIndex]);

              m_lastQueueServed = currentQueueIndex; // Update to the queue that successfully sent a packet

              // Image: "If Empty(Queue_i) then DeficitCounter_i = 0"
              if (tc->IsEmpty ())
                {
                  NS_LOG_DEBUG ("DRR: Queue " << currentQueueIndex << " is now empty. Resetting deficit to 0.");
                  m_deficits[currentQueueIndex] = 0;
                }
              return packetToSend; // Return the sent packet
            }
          else // PacketSize > m_deficits[currentQueueIndex]
            {
              // Packet too large for current deficit. Deficit (OldDeficit + Quantum) is carried over.
              // Image: "Else break (*skip while loop*)" for this queue's turn.
              // This queue has had its chance for this quantum, couldn't use it for *this* packet.
              // Its deficit (which includes the just-added quantum) is preserved.
              NS_LOG_DEBUG ("DRR: Queue " << currentQueueIndex << " head packet (size " << packetSize 
                            << "B) > deficit (" << m_deficits[currentQueueIndex] 
                            << "). No packet sent from this queue. Deficit carried over.");
              // Continue the 'for' loop to try other queues in this Schedule() call.
              // m_lastQueueServed is NOT updated here, as this queue didn't successfully send.
              // It will be updated only if a queue sends, or after a full unsuccessful scan.
            }
        }
        else // Queue became empty unexpectedly, or deficit was non-positive after adding quantum (unlikely with uint32_t)
        {
             NS_LOG_DEBUG ("DRR: Queue " << currentQueueIndex << " could not send (e.g. became empty or non-positive deficit " 
                           << m_deficits[currentQueueIndex] << " before sending).");
        }
    } // End of for loop iterating through queues

  // If the loop completes, no packet was sent from any queue in this pass.
  // This means a full round-robin scan was completed without sending.
  // m_lastQueueServed should reflect the end of this full scan.
  // The last `currentQueueIndex` checked in the loop would be `(m_lastQueueServed_at_entry + 1 + (N-1)) % N`
  // which simplifies to `(m_lastQueueServed_at_entry + N) % N = m_lastQueueServed_at_entry`.
  // So, m_lastQueueServed remains unchanged if a full scan yields no packet, which is correct.
  NS_LOG_LOGIC ("DRR: No packet could be scheduled in this full scan of " << numManagedQueues << " queues.");
  return nullptr;
}

} // namespace ns3
