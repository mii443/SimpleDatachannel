
#include <rtc.hpp>
#include <unordered_map>
#include <future>
#include <random>

using namespace rtc;
using namespace std;

#ifndef H_SIMPLEDATACHANNEL
#define H_SIMPLEDATACHANNEL



	class SimpleDatachannel {
	public:
	    unordered_map<string, shared_ptr<PeerConnection>> peer_connection_map;
	    unordered_map<string, shared_ptr<DataChannel>> data_channel_map;

		string local_id;
		Configuration config;
		shared_ptr<WebSocket> ws;

		function<void(string id, string data)> on_message;

		shared_ptr<PeerConnection> create_peer_connection(weak_ptr<WebSocket> wws, string id);
	    string random_id(size_t length);

		void connect(string id, shared_ptr<WebSocket>, string label);
	    SimpleDatachannel(string, string);

		~SimpleDatachannel();
	private:
		
	};



#endif
