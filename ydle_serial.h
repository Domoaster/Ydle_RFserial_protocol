#include "IFrame.h"
#include "INode.h"

#include "Kernel.h"
#include "Float.h"

#include "node.h"
#include "frame.h"
#include "Crc.h"

#define YDLE_MAX_SIZE_FRAME 36
#define YDLE_START 0x06559
#define YDLE_START_BITS 0x42

#define YDLE_TYPE_ETAT 		1 // Node send data
#define YDLE_TYPE_CMD  		2 // ON/OFF sortie etc...
#define YDLE_TYPE_ACK  		3 // Acquit last command
#define YDLE_TYPE_ETAT_ACK 	4 // Node send data and whant ACK

#define YDLE_DATA_BOOL					0 // true / false (1 bit / 8 bits data)
#define YDLE_DATA_UINT8				1 // (8 bits / 16 bits data)
#define YDLE_DATA_UINT16			2 // (16 bits / 24 bits data)
#define YDLE_DATA_UINT24			3 // (24 bits / 32 bits data)

#define YDLE_CMD_LINK				0 // Link a node to the master
#define YDLE_CMD_ON					1 // Send a ON command to node data = N° output
#define YDLE_CMD_OFF				2 // Send a OFF command to node data = N° output
#define YDLE_CMD_RESET				3 // Ask a node to reset is configuration
#define YDLE_CMD_SET				4 // Set value
#define YDLE_CMD_GET				5 // Get value
#define YDLE_CMD_PING				6 // PING

namespace domoaster {

class ydle_serial : public IProtocol 
{
  public:
    std::string Name () { return "Ydle over serial protocol" ; }
    std::string Class () { return "ydle_serial" ; }
    std::string Connector () { return "RFserial" ; }
    void Init () {} ;
    void Start ()  ;
    
    void Receive (std::vector<uint16_t> &) ;
    void Receive (uint8_t){} ;
    
    void Init (IConnector *) ;
    
    std::string getDefaultNodeType () { return "node_ydle_serial" ; }
    Json::Value onIHMRequest (const WebServer::HTTPRequest *request) ;
    
        struct ACKCmd_t
    {
      frame_ydle Frame;
      int Time;
      int iCount;
    };
    std::list<ydle_serial::ACKCmd_t> mListACK;
    bool length_ok;
    bool rx_active;

    
    uint16_t rx_bits;

    uint8_t bit_count;
    int rx_bytes_count;
    
    static uint8_t start_bit = 0b01000010;
    uint8_t receptor;
    uint8_t sender;
    uint8_t type;
  
    uint8_t m_data[YDLE_MAX_SIZE_FRAME];
    
    std::vector<uint8_t> bytesToSend ;

    // Ajout d'une donnée
    void addData(frame_ydle *frame, int type, bool data) ;
    void addData(frame_ydle *frame, int type, int data) ;
    void addData(frame_ydle *frame, int type, long int data) ;
    void addData(frame_ydle *frame, int type, float data) ;

    
    void InitReception () ;
    void addBit(uint8_t) ;
    void onBitReceived (uint8_t) ;
    uint8_t computeCrc(frame_ydle *frame) ;
    void InitFrame (frame_ydle *frame, int, int, int) ;
    void onFrameReceived (frame_ydle *frame) ;
    void SendACK (frame_ydle *frame) ;
    void Send (frame_ydle *frame) ;
    void AddBytes(uint8_t byte_in);
    
} ;

} ; // namespace domoaster

extern "C" int LoadPlugins (domoaster::Kernel &) ;