
#include <cstdint>
#include"serialib.h"
#include "titan_tube.h"
#include "dmx_controls.h"



int open_dmx_port(serialib *serial, const char *port) {
  if (serial->openDevice(port, 57600) != 1) {
    //printf("Erreur d'ouverture du port DMX\n");
    return -1;
  }
  return 0;
}

// 0 0 0 0 0 0 
// HHHH aaaa bbbb c ccc d  ddd ... 0xE7
// 0123 4567 8    12    16
#pragma pack(push, 1) 
struct Packet {
  unsigned char SOM = 0x7E;
  unsigned char label = 0x06;
  uint16_t size;
  unsigned char data[MAX_CHANNEL] = {25};  // Recommended 25 by API
  unsigned char EOM = 0xE7;
};
#pragma pack(pop) 

void send_dmx_data(serialib *serial, TitanTube *tubes, size_t num_tubes) {
  Packet packet;
  size_t current_tube = 0;
  packet.data[0] = 0; // First dmx byte should always be 0 (RDM)
  for (size_t i = 1; i < num_tubes * 4; i+=4) {
    packet.data[i + 0] = tubes[current_tube].color.r; // Canal 1 - Rouge
    packet.data[i + 1] = tubes[current_tube].color.g; // Canal 2 - Vert
    packet.data[i + 2] = tubes[current_tube].color.b; // Canal 3 - Bleu
    packet.data[i + 3] = 0; // tubes[current_tube].color.r; // Canal 4 - White
    //printf("%zu : %d, %d, %d \n", current_tube, packet.data[i], packet.data[i+1], packet.data[i+2]);
    current_tube++;
  }
  packet.size = MAX_CHANNEL;
  //printf("%d \n",  packet.size);

  uint8_t buff[sizeof(packet)];
  memcpy(buff, &packet, sizeof(packet));

  for(int i = 0 ; i < sizeof(packet) ; i++){
    //printf("%d\n", buff[i]);
  }

  // Envoi des données DMX
  serial->writeBytes((unsigned char*)&packet, sizeof(packet));
}

// Ferme le port série après l'envoi des données
void close_dmx_port(serialib *serial) {
  serial->closeDevice();
}






