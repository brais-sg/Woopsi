#include "slidervertical.h"
#include <stdlib.h>

SliderVertical::SliderVertical(s16 x, s16 y, u16 width, u16 height) : Gadget(x, y, width, height, GADGET_DRAGGABLE) {
	_outline = OUTLINE_IN;
	_minimumValue = 0;
	_maximumValue = 0;
	_minimumGripHeight = 5;
	_pageSize = 1;

	// Create grip
	Rect rect;
	getClientRect(rect);

	_grip = new SliderVerticalGrip(rect.x, rect.y, rect.width, rect.height);
	_grip->setEventHandler(this);
	addGadget(_grip);
}

SliderVertical::~SliderVertical() {
	delete _grip;
}

const s16 SliderVertical::getMinimumValue() const {
	return _minimumValue;
}

const s16 SliderVertical::getMaximumValue() const {
	return _maximumValue;
}

const s16 SliderVertical::getValue() const {
	// Calculate the current value represented by the top of the grip
	Rect rect;
	getClientRect(rect);

	return ((_maximumValue - _minimumValue) * (_grip->getY() - _parent->getY())) / ((rect.height - _grip->getHeight()) + 1);
}

const s16 SliderVertical::getPageSize() const {
	return _pageSize;
}

void SliderVertical::setMinimumValue(const s16 value) {
	_minimumValue = value;
}

void SliderVertical::setMaximumValue(const s16 value) {
	_maximumValue = value;
}

void SliderVertical::setValue(const s16 value) {

	// Convert the value to co-ordinates using fixed-point fractional values
	// for accuracy
	Rect rect;
	getClientRect(rect);

	s32 pixelValue = ((_maximumValue - _minimumValue) << 8) / ((rect.height - _grip->getHeight()) + 1);
	
	if (pixelValue > 0) {
		s16 newGripY = (value << 8) / pixelValue;
	
		if (newGripY + _grip->getHeight() > rect.y + rect.height) {
			newGripY = (rect.y + rect.height) - _grip->getHeight();
		}

		// Move the grip
		_grip->moveTo(0, abs(newGripY));
	}
}

void SliderVertical::setPageSize(s16 pageSize) {
	_pageSize = pageSize;
}

void SliderVertical::draw() {
	Gadget::draw();
}

void SliderVertical::draw(Rect clipRect) {
	GraphicsPort* port = newInternalGraphicsPort(clipRect);

	// Draw background
	port->drawFilledRect(0, 0, _width, _height, _darkColour);

	// Draw outline
	port->drawBevelledRect(0, 0, _width, _height);

	delete port;
}

bool SliderVertical::click(s16 x, s16 y) {
	if (_flags.enabled) {
		if (checkCollision(x, y)) {
			_clickedGadget = NULL;

			// Work out which gadget was clicked
			for (s16 i = _gadgets.size() - 1; i > -1; i--) {
				if (_gadgets[i]->click(x, y)) {
					break;
				}
			}

			// Did we click a gadget?
			if (_clickedGadget == NULL) {

				// Move the grip
				s16 newGripY;

				// Which way should the grip move?
				if (y > _grip->getY()) {
					// Move grip down
					newGripY = (_grip->getY() - getY()) + _grip->getHeight();
				} else {
					// Move grip up
					newGripY = (_grip->getY() - getY()) - _grip->getHeight();
				}

				// Get client rect for this gadget
				Rect rect;
				getClientRect(rect);

				// Adjust y value so that it does not exceed boundaries of gutter
				if (newGripY < rect.y) {
					newGripY = rect.y;
				} else if (newGripY + _grip->getHeight() > rect.y + rect.height) {
					newGripY = (rect.height - _grip->getHeight()) + 1;
				}

				// Move the grip
				_grip->moveTo(0, newGripY);

				// Handle click on gutter
				Gadget::click(x, y);
			}

			return true;
		}
	}

	return false;
}

bool SliderVertical::release(s16 x, s16 y) {
	if (_clickedGadget != NULL) {

		// Release clicked gadget
		_clickedGadget->release(x, y);

		return true;
	} else if (_flags.clicked) {

		// Handle release on window
		Gadget::release(x, y);

		return true;
	}

	return false;
}

bool SliderVertical::drag(s16 x, s16 y, s16 vX, s16 vY) {
	// Handle child dragging
	if (_clickedGadget != NULL) {
		return _clickedGadget->drag(x, y, vX, vY);
	}

	return false;
}

bool SliderVertical::handleEvent(const EventArgs& e) {

	// Handle grip events
	if ((e.gadget == _grip) && (e.gadget != NULL)) {
		if ((e.type == EVENT_DRAG) || (e.type == EVENT_MOVE)) {

			// Grip has moved
			raiseValueChangeEvent();

			return true;
		}
	}

	return false;
}

void SliderVertical::recalculate() {
	resizeGrip();
	
	// Reposition grip
	setValue(getValue());
}

void SliderVertical::resizeGrip() {

	// Get available size
	Rect rect;
	getClientRect(rect);

	// Calculate new height
	s32 newHeight = rect.height;
	
	// Calculate the height of the content that has overflowed the viewport
	s32 overspill = (abs(_maximumValue - _minimumValue) - _pageSize << 8);
	
	// Is there any overflow?
	if (overspill > 0) {
	
		// Calculate the ratio of content to gutter
		u32 ratio = (abs(_maximumValue - _minimumValue) << 8) / rect.height;
		
		// New height is equivalent to the height of the gutter minus
		// the ratio-converted overflow height
		newHeight = rect.height - (overspill / ratio);
		

		// Ensure height is within acceptable boundaries
		if (newHeight < _minimumGripHeight) {
			newHeight = _minimumGripHeight;
		}
		if (newHeight > rect.height) newHeight = rect.height;

		// Perform resize
		_grip->resize(rect.width, newHeight);
	}
}