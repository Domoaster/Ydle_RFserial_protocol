/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "ydle_serial.h"
#include "DataAccess.h"
#include "WebServer.h"
#include "../../src/WebServer.cpp"

using namespace domoaster;

void ydle_serial::Init(IConnector *connector) {
    _connector = connector;
    InitReception();
}

void ydle_serial::Start() {
  /*  while(1){
        TestSend();
        usleep(5000000);
    }*/
}

void ydle_serial::InitReception() {
    //first = true;
    rx_active = false;
    length_ok = false;
    rx_bits = 0;
    bit_count = 0;
    rx_bytes_count = 0;
    receptor = 0;
    sender = 0;
    type = 0;
    memset(m_data, 0, sizeof (m_data));
}

void ydle_serial::Receive(std::vector<uint16_t> & rx_values) {
    bool bit = true;
    for (int i(0); i < rx_values.size(); ++i) {
        if (rx_values[i] > 950 && rx_values[i] < 1050) {
            addBit(bit);
            bit = !bit;
        }
        if (rx_values[i] > 1950 && rx_values[i] < 2050) {
            addBit(bit);
            addBit(bit);
            bit = !bit;
        }
    }
}

void ydle_serial::addBit(uint8_t bit) {
    bit = bit ? 0x1 : 0x0;
    rx_bits <<= 1;
    rx_bits |= bit;
    onBitReceived(bit);
}

void ydle_serial::onBitReceived(uint8_t bit_value) {
    if (rx_active) {
        bit_count++;
        // On récupère les bits et on les places dans des variables
        // 1 bit sur 2 avec Manchester
        if (bit_count % 2 == 1) {
            if (bit_count < 16) {
                // Les 8 premiers bits de données
                receptor <<= 1;
                receptor |= bit_value;
            } else if (bit_count < 32) {
                // Les 8 bits suivants
                sender <<= 1;
                sender |= bit_value;
            } else if (bit_count < 38) {
                // Les 3 bits de type
                type <<= 1;
                type |= bit_value;
            } else if (bit_count < 48) {
                // Les 5 bits de longueur de trame
                rx_bytes_count <<= 1;
                rx_bytes_count |= bit_value;
            } else if ((bit_count - 48) < (rx_bytes_count * 16)) {
                length_ok = true;
                m_data[(bit_count - 48) / 16] <<= 1;
                m_data[(bit_count - 48) / 16] |= bit_value;
            }
        }

        // Quand on a reçu les 24 premiers bits, on connait la longueur de la trame
        // On vérifie alors que la longueur semble logique	
        if (bit_count >= 48) {
            // Les bits 19 à 24 informent de la taille de la trame
            // On les vérifie car leur valeur ne peuvent être < à 1 et > à 31
            if (rx_bytes_count < 1 || rx_bytes_count > 31) {
                // Mauvaise taille de message, on ré-initialise la lecture
                DOMOASTER_INFO << rx_bytes_count << " error !";
                InitReception();
                return;
            }
        }

        // On vérifie si l'on a reçu tout le message
        if ((bit_count - 48) >= (rx_bytes_count * 16) && (length_ok == true)) {
            DOMOASTER_INFO << "complete";
            frame_ydle frame;
            InitFrame(&frame, sender, receptor, type);
            frame.taille = rx_bytes_count;
            memcpy(frame.data, m_data, rx_bytes_count - 1); // copy data len - crc
            // crc calcul
            frame.crc = computeCrc(&frame);
            onFrameReceived(&frame);
            InitReception();
        }
    } else if (rx_bits == YDLE_START) {
        // Octet de start, on commence à collecter les données
        DOMOASTER_INFO << "start received";
        rx_active = true;
        rx_bytes_count = 0;
        bit_count = 0;
    }
}

void ydle_serial::InitFrame(frame_ydle *frame, int sender, int receptor, int type) {
    frame->sender = sender;
    frame->receptor = receptor;
    frame->type = type;
    frame->taille = 0;
    memset(frame->data, 0, sizeof (frame->data));
    frame->crc = 0;
}

void ydle_serial::onFrameReceived(frame_ydle *frame) {
    frame->Dump("onFrameReceived");
    // If it's a ACK then handle it
    if (frame->type == YDLE_TYPE_ACK) {
        std::list<ydle_serial::ACKCmd_t>::iterator i;
        for (i = mListACK.begin(); i != mListACK.end(); ++i) {
            if (frame->sender == i->Frame.receptor && frame->receptor == i->Frame.sender) {
                DOMOASTER_DEBUG << "Remove ACK from pending list";
                i = mListACK.erase(i);
                break; // remove only one ACK at a time.
            }
        }
    } else if (frame->type == YDLE_TYPE_ETAT_ACK) {
        DOMOASTER_DEBUG << "New State/ACK frame ready to be sent :";
        NotifyIHM(frame);
        SendACK(frame);
    } else if (frame->type == YDLE_TYPE_ETAT) {
        DOMOASTER_DEBUG << "New State frame ready to be sent :";
        NotifyIHM(frame);
    } else if (frame->type == YDLE_TYPE_CMD) {
        DOMOASTER_DEBUG << "New Command frame ready to be sent :";
        NotifyIHM(frame);
    } else {
        DOMOASTER_DEBUG << "Bad frame, trash it :";
    }
}

void ydle_serial::Send(frame_ydle *frame) {
    frame->taille++;
    frame->crc = computeCrc(frame);

    frame->Dump("Send");
    // Durée d'un bit
    bytesToSend.push_back((1000 & 0xFF00) >> 8);
    bytesToSend.push_back(1000 & 0xFF);
    // Trame 
    AddBytes(0xFF);
    AddBytes(0xFF);
    AddBytes(0xFF);
    AddBytes(0xFF);
    AddBytes(66);
    AddBytes(frame->receptor);
    AddBytes(frame->sender);
    AddBytes((frame->type << 5) + frame->taille);
    for (int j = 0; j < frame->taille - 1; j++) {
        AddBytes(frame->data[j]);
    }
    AddBytes(frame->crc);

    _connector->Send(bytesToSend);
    bytesToSend.clear();
    
}
void ydle_serial::TestSend() {
    
    // Durée d'un bit
    bytesToSend.push_back((1000 & 0xFF00) >> 8);
    bytesToSend.push_back(1000 & 0xFF);
    // Trame 
    AddBytes(0xFF);
    AddBytes(0xFF);
    AddBytes(0xFF);
    AddBytes(0xFF);
    AddBytes(start_bit);
    AddBytes(2);
    AddBytes(1);
    AddBytes((3 << 5) + 1);
    /*for (int j = 0; j < frame->taille - 1; j++) {
        AddBytes(frame->data[j]);
    }*/
    AddBytes(52);

    _connector->Send(bytesToSend);
    bytesToSend.clear();
    
}

void ydle_serial::SendACK(frame_ydle *frame) {
    InitFrame(frame, frame->receptor, frame->sender, YDLE_TYPE_ACK);
    Send(frame);
}

uint8_t ydle_serial::computeCrc(frame_ydle *frame) {
    uint8_t *buf, crc;
    int a, j;

    buf = (uint8_t*) malloc(frame->taille + 3);
    memset(buf, 0x0, frame->taille + 3);

    buf[0] = frame->sender;
    buf[1] = frame->receptor;
    buf[2] = frame->type;
    buf[2] = buf[2] << 5;
    buf[2] |= frame->taille;

    for (a = 3, j = 0; j < frame->taille - 1; a++, j++) {
        buf[a] = frame->data[j];
    }
    crc = crc8(buf, frame->taille + 2);
    free(buf);
    return crc;
}

Json::Value ydle_serial::onIHMRequest(const WebServer::HTTPRequest *request) {

}

void ydle_serial::AddBytes(uint8_t byte_in) {
    uint8_t byte_tmp = 0;
    uint8_t bit_pair = 0;
    uint8_t bit_impair = 0;

    for (int i = 3; i >= 0; --i) {
        bit_pair = (((byte_in & 0xF0) >> 4) & (1 << i)) >> i;
        bit_impair = !bit_pair;
        byte_tmp = (byte_tmp << 1) + bit_pair;
        byte_tmp = (byte_tmp << 1) + bit_impair;
    }
    bytesToSend.push_back(byte_tmp);
    
    byte_tmp = 0;
    bit_pair = 0;
    bit_impair = 0;
    for (int i = 3; i >= 0; --i) {
        bit_pair = ((byte_in & 0x0F) & (1 << i)) >> i;
        bit_impair = !bit_pair;
        byte_tmp = (byte_tmp << 1) + bit_pair;
        byte_tmp = (byte_tmp << 1) + bit_impair;
    }
    bytesToSend.push_back(byte_tmp);
}