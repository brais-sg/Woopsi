#include "pacman.h"
#include "superbitmap.h"
#include "button.h"
#include "amigawindow.h"
#include "amigascreen.h"
#include "debug.h"
#include "pacsprite.h"
#include "pacmap.h"
#include "pacplayer.h"
#include "pacghost.h"
#include "pacghosts.h"

PacMan::PacMan(AmigaScreen* screen) : EventHandler() {
	_screen = screen;
	_width = 95;
	_height = 110;
	_gameOver = false;

	initGUI();

	_map = new PacMap(this);
	_ghosts = new PacGhosts(this);
	_player = new PacPlayer(this);
}

PacMan::~PacMan() {
	delete _map;
	delete _player;
	delete _ghosts;
}

PacMap* PacMan::getMap() {
	return _map;
}

PacGhosts* PacMan::getGhosts() {
	return _ghosts;
}

PacPlayer* PacMan::getPlayer() {
	return _player;
}

SuperBitmap* PacMan::getSuperBitmap() {
	return _superBitmap;
}

void PacMan::reset() {
	_map->reset();
	_ghosts->reset();
	_player->reset();
	_map->draw();
}

void PacMan::run() {
	if (!_gameOver) {
		_map->drawPills();
		_player->run();
		_ghosts->run();

		_superBitmap->draw();
	}
}

void PacMan::endGame() {
	_gameOver = true;

	_superBitmap->drawFilledRect(0, 0, _width, _height, _superBitmap->getBackColour());
	_superBitmap->drawText(10, 40, _superBitmap->getFont(), "Game Over");
}

bool PacMan::handleEvent(const EventArgs& e) {
	switch (e.type) {
		case EVENT_VBL:
			handleVBL(e);
			return true;
		case EVENT_KEY_PRESS:
			handleKeyPress(e);
			return true;
		case EVENT_RELEASE:
			handleRelease(e);
			return true;
		case EVENT_CONTEXT_MENU_SELECTION:
			switch (woopsiApplication->getContextMenuValue()) {
				case 1:
					_player->resetLives();
					_gameOver = false;
					reset();
					break;
				case 2:
					_window->close();
					break;
			}
			return true;

		default:
			return false;
	}
}

void PacMan::handleVBL(const EventArgs& e) {
	if (!_gameOver) {
		run();
	}
}

void PacMan::handleKeyPress(const EventArgs& e) {
	if (_window->hasFocus()) {
		if (e.gadget->getRefcon() == 1) {
			switch (e.keyCode) {
				case KEY_CODE_UP:
					Debug::printf("PacMan: Up pressed.");
					_player->setBufferedDirection(1);
					break;
				case KEY_CODE_DOWN:
					Debug::printf("PacMan: Down pressed.");
					_player->setBufferedDirection(2);
					break;
				case KEY_CODE_LEFT:
					Debug::printf("PacMan: Left pressed.");
					_player->setBufferedDirection(3);
					break;
				case KEY_CODE_RIGHT:
					Debug::printf("PacMan: Right pressed.");
					_player->setBufferedDirection(4);
					break;
				default:
					break;
			}
		}
	}
}

void PacMan::initGUI() {
	// Create window
	_window = new AmigaWindow(0, 13, 103, 150, "PacMan", Gadget::GADGET_CLOSABLE | Gadget::GADGET_DRAGGABLE);
	_screen->addGadget(_window);
	_window->setShiftClickChildren(false);
	_window->addContextMenuItem("Reset", 1);
	_window->addContextMenuItem("Quit", 2);

	_window->setEventHandler(this);
	_window->setRefcon(1);

	Gadget::Rect rect;
	_window->getClientRect(rect);

	_superBitmap = new SuperBitmap(rect.x, rect.y, _width, _height, _width, _height, false);
	_window->addGadget(_superBitmap);
	_superBitmap->setAllowStylusScroll(false);
	_superBitmap->setRefcon(2);

	_resetButton = new Button(rect.x + 18, rect.y + 114, 60, 14, "Reset");
	_window->addGadget(_resetButton);
	_resetButton->setEventHandler(this);
	_resetButton->setRefcon(3);
	
	// Register window for VBL events
	Woopsi::registerForVBL(_window);
}

void PacMan::handleRelease(const EventArgs& e) {
	if (e.gadget->getRefcon() == 3) {
		_player->resetLives();
		_gameOver = false;
		reset();
	}
}
