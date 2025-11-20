#include<iostream>
#include <chrono>
#include"Petrinet.h"
#include"read_json.h"
#include"Process.h"

int main() {
	Petrinet petrinet;
	read_place_json(petrinet);
	read_trans_json(petrinet);
	read_tokens_json(petrinet);
	petrinet.dijskstra_search();
	petrinet.bestpath = petrinet.Createbestpath();
	return 0;
}