
#include "simpleDatachannel.h"

#include <iostream>
#include <memory>
#include <random>
#include <unordered_map>

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

#include <nlohmann/json.hpp>

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

int main() {
	auto sdc = SimpleDatachannel("", "ws://mii.codes:3334/");
	sdc.on_message = [](string id, string data) { cout << id << ": " << data << endl; };

	cout << "ID: " << sdc.local_id << endl;
	
	cout << "dest id? ";
	string id;
	cin >> id;
	
	sdc.connect(id, sdc.ws, "test");

	while (sdc.data_channel_map.size() == 0) {}

	while (true) {
		cout << "text? " << endl;
		string txt;
		cin >> txt;

		cout << "dest id? " << endl;
		string id;
		cin >> id;

		sdc.data_channel_map[id]->send(txt);
	}
	return 0;
}