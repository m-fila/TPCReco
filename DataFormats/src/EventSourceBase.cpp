#include<cstdlib>
#include <iostream>

#include "EventSourceBase.h"
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
EventSourceBase::EventSourceBase() {

	myCurrentEvent = std::make_shared<EventCharges>();
	myCurrentEntry = 0;
	nEvents = 0;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void EventSourceBase::loadDataFile(const std::string& fileName) {

	currentFilePath = fileName;

}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void EventSourceBase::loadGeometry(const std::string& fileName) {

	Geometry(fileName.c_str());
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void EventSourceBase::loadEventId(unsigned long int iEvent) {
	for (uint64_t iEntry = 0; currentEventNumber() != iEvent && iEntry < nEvents; ++iEntry) {
		loadFileEntry(iEntry);
	}
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::shared_ptr<EventCharges> EventSourceBase::getCurrentEvent() const {

	return myCurrentEvent;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::shared_ptr<EventCharges> EventSourceBase::getNextEvent() {

	std::cout << "nEvents: " << nEvents
		<< " myCurrentEntry: " << myCurrentEntry
		<< std::endl;

	if (nEvents > 0 && myCurrentEntry < nEvents - 1) {
		loadFileEntry(++myCurrentEntry);
	}
	return myCurrentEvent;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::shared_ptr<EventCharges> EventSourceBase::getPreviousEvent() {

	if (myCurrentEntry > 0 && nEvents > 0) {
		loadFileEntry(--myCurrentEntry);
	}
	return myCurrentEvent;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::shared_ptr<EventCharges> EventSourceBase::getLastEvent() {

	if (nEvents > 0) {
		loadFileEntry(nEvents - 1);
		myCurrentEntry = nEvents - 1;
	}

	return myCurrentEvent;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
uint64_t EventSourceBase::currentEventNumber() const {

	if (myCurrentEvent) {
		return myCurrentEvent->Info().EventId();
	}
	return 0;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::string EventSourceBase::getCurrentPath() const {

	return currentFilePath;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////