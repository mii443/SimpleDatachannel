#include "simpleDatachannel.h"

#include <rtc.hpp>

#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <unordered_map>

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

#include <nlohmann/json.hpp>

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }


SimpleDatachannel::SimpleDatachannel(string id, string ws_url) {
	//rtc::InitLogger(LogLevel::Error);
	
	if (id.empty()) {
		id = random_id(4);
	}
	local_id = id;

	config.iceServers.emplace_back("stun.l.google.com:19302");

	cout << "The local ID is: " << local_id << endl;
	ws = make_shared<WebSocket>();
	promise<void> ws_promise;
	future<void> ws_future = ws_promise.get_future();

	ws->onOpen([&ws_promise]() {
		cout << "WebSocket connected, signaling ready" << endl;
		ws_promise.set_value();
	});

	ws->onError([&ws_promise](string s) {
		cout << "WebSocket error" << endl;
		ws_promise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
	});

	ws->onClosed([]() { 
		cout << "WebSocket closed" << endl; 
	  
	});

	ws->onMessage(nullptr, [&](string data) {
		json message = json::parse(data);

		auto it = message.find("id");
		if (it == message.end())
			return;
		auto id = it->get<string>();

		it = message.find("type");
		if (it == message.end())
			return;
		const auto type = it->get<string>();

		shared_ptr<PeerConnection> pc;
		const auto jt = peer_connection_map.find(id);
		if (jt != peer_connection_map.end()) {
			pc = jt->second;
		} else if (type == "offer") {
			cout << "Answering to " + id << endl;
			pc = create_peer_connection(ws, id);
		} else {
			return;
		}

		if (type == "offer" || type == "answer") {
			const auto sdp = message["description"].get<string>();
			pc->setRemoteDescription(Description(sdp, type));
		} else if (type == "candidate") {
			const auto sdp = message["candidate"].get<string>();
			const auto mid = message["mid"].get<string>();
			pc->addRemoteCandidate(Candidate(sdp, mid));
		}
	});

	ws->open(ws_url + local_id);
	ws_future.get();
}

void SimpleDatachannel::connect(string id, shared_ptr<WebSocket> ws, string label) {
	if (id.empty())
		return;
	if (id == local_id)
		return;

	auto pc = create_peer_connection(ws, id);
	auto dc = pc->createDataChannel(label);

	dc->onMessage(nullptr, [this, id, wdc = make_weak_ptr(dc)](string data) {
		on_message(id, data);
	});

	data_channel_map.emplace(id, dc);
}

SimpleDatachannel::~SimpleDatachannel() {
	data_channel_map.clear();
	peer_connection_map.clear();
}

shared_ptr<PeerConnection> SimpleDatachannel::create_peer_connection(weak_ptr<WebSocket> wws, string id) {
	auto pc = make_shared<PeerConnection>(config);

	pc->onStateChange([](PeerConnection::State state) { 
	  cout << "State: " << state << endl;
	});

	pc->onGatheringStateChange(
	    [](PeerConnection::GatheringState state) { cout << "Gathering State: " << state << endl; });

	pc->onLocalDescription([wws, id](Description description) {
		const json message = {
		    {"id", id}, {"type", description.typeString()}, {"description", string(description)}};

		if (auto wsl = wws.lock())
			wsl->send(message.dump());
	});

	pc->onLocalCandidate([wws, id](Candidate candidate) {
		const json message = {{"id", id},
		                {"type", "candidate"},
		                {"candidate", string(candidate)},
		                {"mid", candidate.mid()}};

		if (auto wsl = wws.lock())
			wsl->send(message.dump());
	});

	pc->onDataChannel([this, id](shared_ptr<DataChannel> dcl) {
		cout << "DataChannel from " << id << " received with label \"" << dcl->label() << "\""
		     << endl;
		/*
		dcl->onOpen([this, wdc = make_weak_ptr(dcl)]() {
			if (auto dc = wdc.lock())
				dc->send("Hello from " + local_id);
		});
		

		dcl->onClosed([id]() { cout << "DataChannel from " << id << " closed" << endl; });
    */

		dcl->onMessage(nullptr, [this, id](string data) {
			on_message(id, data);
		});

		data_channel_map.emplace(id, dcl);
	});

	peer_connection_map.emplace(id, pc);
	return pc;
};

string SimpleDatachannel::random_id(size_t length) {
	static const string characters(
	    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	string id(length, '0');
	default_random_engine rng(random_device{}());
	uniform_int_distribution<int> dist(0, int(characters.size() - 1));
	generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
	return id;
}


