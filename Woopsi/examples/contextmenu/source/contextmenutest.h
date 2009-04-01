#ifndef _CONTEXT_MENU_TEST_H_
#define _CONTEXT_MENU_TEST_H_

#include "woopsi.h"
#include "eventhandler.h"
#include "multilinetextbox.h"

using namespace WoopsiUI;

class ContextMenuTest : public Woopsi, public GadgetEventHandler {
public:
	void startup();
	void shutdown();
	void handleContextMenuSelectionEvent(const GadgetEventArgs& e);
	
private:
	MultiLineTextBox* _output;
};

#endif
