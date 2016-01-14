
#include "Kernel.h"
#include "Float.h"

namespace domoaster {

class frame_ydle : public IFrame
{
  public:
    uint8_t type;
    uint8_t crc;
    void Dump (const char * msg)
    {
      char sztmp[255];

      if (msg)
        DOMOASTER_DEBUG << "\t" << msg ;

      DOMOASTER_DEBUG << "Emetteur : " << (int) sender;
      DOMOASTER_DEBUG << "Recepteur : " << (int) receptor;
      DOMOASTER_DEBUG << "Type : " << (int) type;
      DOMOASTER_DEBUG << "Taille : " << (int) taille;
      DOMOASTER_DEBUG << "Crc : " << (int) crc;

      sprintf(sztmp, "Data Hex : ");
      for (int a=0; a < taille - 1; a++)
        sprintf(sztmp,"%s 0x%02X", sztmp, data[a]);
      DOMOASTER_DEBUG << sztmp;

      sprintf(sztmp, "Data Dec : ");
      for (int a=0; a < taille - 1; a++)
        sprintf(sztmp,"%s %d", sztmp, data[a]);
      DOMOASTER_DEBUG << sztmp;
    }
  private:
    std::string frameBits;
} ;

} ; // namespace domoaster