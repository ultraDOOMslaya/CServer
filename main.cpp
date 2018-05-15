#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>


ENetAddress			address;
ENetHost*			server;
ENetHost*			client;
ENetPeer*			peer;
ENetEvent			event;
ENetPacket*			packet;

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

int main(void)
{
	char str[512];
	bool isServer;
	bool sender = false;
	int connected=0;

	if (enet_initialize() != 0) {
		printf("Could not initialize enet.\n");
		return 0;
	}

	address.host = ENET_HOST_ANY;
	address.port = SERVER_PORT;

	printf("(C), (R) or (S)erver?\n");
	gets_s(str);

	if ((str[0] == 'c') || (str[0] == 'C'))
	{
		client = enet_host_create(NULL, 1, 2, 5760 / 8, 1440 / 8);
		enet_address_set_host(&address, ENET_HOST_ANY);
		address.port = SERVER_PORT;
		peer = enet_host_connect(client, &address, 2, 0);
		isServer = false;
		connected = 1;
	}
	else if ((str[0] == 'r') || (str[0] == 'R') ) {
		client = enet_host_create(NULL, 1, 2, 5760 / 8, 1440 / 8);
		enet_address_set_host(&address, ENET_HOST_ANY);
		address.port = SERVER_PORT;
		peer = enet_host_connect(client, &address, 2, 0);
		isServer = false;
		sender = true;
		connected = 1;
	}
	else {
		server = enet_host_create(&address, 4, 2, 0, 0);
		isServer = true;
	}

	while (1) {
		if (isServer) {
			while (enet_host_service(server, &event, 1000) > 0) {
				switch (event.type) {

				case ENET_EVENT_TYPE_CONNECT:
					printf("A new client connected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);

					event.peer->data = "player";
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					printf("A packet of length %u containing %s was received from %s on channel %u.\n",
						event.packet->dataLength,
						event.packet->data,
						event.peer->data,
						event.channelID);

					/*sprintf_s(str, "%s: %s", (char*)event.peer->data, (char*)event.packet->data);
					packet = enet_packet_create(str, strlen(str) + 1,
						ENET_PACKET_FLAG_RELIABLE);

					printf("Number of peers: %u\n", connections.size());
					for (ENetPeer e : connections) {
						int sent = enet_peer_send(&e, 0, packet);
						if (sent < 0) {
							printf("Failed to send packet to peer...");
						}
					}*/
					for (int i = 0; i < server->peerCount; i++) {
						if (server->peers[i].data != NULL) {
							printf("peer data is: %s\n", server->peers[i].data);
							int sent = enet_peer_send(&server->peers[i], 0, event.packet);
							if (sent < 0) {
								printf("Failed to send packet to peer...");
							}
						}
					}
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					printf("%s disconnected.\n", event.peer->data);
					event.peer->data = NULL;

				default:
					printf("Tick tock.\n");
					break;
				}
			}
		}
		else {
			while (enet_host_service(client, &event, 1000) > 0) {
				switch (event.type)
				{
				case ENET_EVENT_TYPE_NONE:
					break;
				case ENET_EVENT_TYPE_CONNECT:
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					printf("%s \n", (char*)event.packet->data);
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					connected = 0;
					printf("You have been disconnected.\n");
					return 2;
				default:
					break;
				}
			}

			if (connected > 0) {
				if (sender) {
					printf("Input: ");
					gets_s(str);

					if (strlen(str) == 0) { continue; }

					if (strncmp("q", str, 512) == 0) {
						connected = 0;
						enet_peer_disconnect(peer, 0);
						continue;
					}

					packet = enet_packet_create(str, strlen(str) + 1,
						ENET_PACKET_FLAG_RELIABLE);
					int sent = enet_peer_send(peer, 0, packet);
					if (sent < 0) {
						printf("Failed to send packet to host...");
					}
				}
			}
		}
	}

	if (isServer) {
		enet_host_destroy(server);
	}
	else {
		enet_host_destroy(client);
	}
	enet_deinitialize();
}