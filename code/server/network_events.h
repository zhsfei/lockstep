#ifndef NETWORK_EVENTS_H
#define NETWORK_EVENTS_H

#include "network.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum network_event_type {
  network_event_type_connect,
  network_event_type_disconnect
};

struct connect_network_event {
  client_id ClientID;
};

struct disconnect_network_event {
  client_id ClientID;
};

memsize SerializeDisconnectNetworkEvent(client_id ID, buffer Out);
memsize SerializeConnectNetworkEvent(client_id ID, buffer Out);
network_event_type UnserializeNetworkEventType(buffer Input);
connect_network_event UnserializeConnectNetworkEvent(buffer Input);
disconnect_network_event UnserializeDisconnectNetworkEvent(buffer Input);

#endif
