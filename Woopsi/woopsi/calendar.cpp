// TODO: resize()
// TODO: Still seems to be a rounding error in vertical 

#include <stdio.h>
#include <stdlib.h>
#include "calendar.h"
#include "date.h"
#include "button.h"
#include "graphicsport.h"
#include "debug.h"
#include "label.h"

using namespace WoopsiUI;

Calendar::Calendar(s16 x, s16 y, u16 width, u16 height, u8 day, u8 month, u16 year, u32 flags, FontBase* font) : Gadget(x, y, width, height, flags, font) {
	_date = NULL;
	_visibleDate = NULL;

	setOutlineType(OUTLINE_OUT);
	_selectedDayButton = NULL;

	buildGUI();

	setDate(day, month, year);
}

Calendar::~Calendar() {
	delete _date;
}

void Calendar::draw(Rect clipRect) {

	GraphicsPort* port = newInternalGraphicsPort(clipRect);

	port->drawFilledRect(0, 0, _width, _height, _backColour);

	// Draw outline
	port->drawBevelledRect(0, 0, _width, _height);

	delete port;
}

bool Calendar::handleEvent(const EventArgs& e) {

	if (e.gadget != NULL) {
		if (e.type == EVENT_RELEASE) {

			// Handle left arrow release
			if (e.gadget == _leftArrow) {
				_visibleDate->addMonths(-1);
				populateGUI();
				return true;
			}

			// Handle right arrow release
			if (e.gadget == _rightArrow) {
				_visibleDate->addMonths(1);
				populateGUI();
				return true;
			}

			// Handle day button release
			if (e.gadget->getRefcon() > 0) {

				// Catch clicks on the already-selected button
				if (_selectedDayButton == e.gadget) return true;

				// Select the new gadget and deselect the old
				e.gadget->setOutlineType(OUTLINE_IN);
				if (_selectedDayButton != NULL) {
					_selectedDayButton->setOutlineType(OUTLINE_CLICK_DEPENDENT);
					_selectedDayButton->draw();
				}

				_selectedDayButton = (Button*)e.gadget;

				// Update the date
				u8 day = atoi(_selectedDayButton->getText());
				_date->setDate(day, _visibleDate->getMonth(), _visibleDate->getYear());

				// Raise an action event
				raiseActionEvent(0, 0, 0, 0, KEY_CODE_NONE);
				return true;
			}
		}
	}

	return false;
}

void Calendar::setDate(u8 day, u8 month, u16 year) {
	
	// Initialise date class
	if (_date == NULL) {
		_date = new Date(day, month, year);
		_visibleDate = new Date(day, month, year);
	} else {
		_date->setDate(day, month, year);
		_visibleDate->setDate(day, month, year);
	}

	populateGUI();
}

void Calendar::populateGUI() {

	// Buffer for int-to-string conversions
	char buffer[6];

	// Set month label
	_monthLabel->disableDrawing();
	_monthLabel->setText(_visibleDate->getMonthName());

	// Append year to month name label
	sprintf(buffer, " %d", _visibleDate->getYear());
	_monthLabel->enableDrawing();
	_monthLabel->appendText(buffer);

	u8 buttonIndex = 0;
	Button* button;

	// Locate the first button in the gadget array
	for (u8 i = _decorationCount; i < _gadgets.size(); i++) {
		if (_gadgets[i]->getRefcon() > 0) {
			buttonIndex = i;
			break;
		}
	}

	// Get date object for start of this month
	Date* thisMonth = new Date(1, _visibleDate->getMonth(), _visibleDate->getYear());

	// Prepare to update grid of days
	u8 updatedDays = 0;
	u8 maxDays = CALENDAR_ROWS * CALENDAR_COLS;
	u8 startDay = 1;

	if (thisMonth->getWeekDay() != 0) {

		// Get start date of previous month
		Date* previousMonth = new Date(1, _visibleDate->getMonth(), _visibleDate->getYear());
		previousMonth->addMonths(-1);

		// Calculate calendar start date
		startDay = (previousMonth->getMonthDays() - thisMonth->getWeekDay()) + 1;

		// Set buttons
		while (startDay <= previousMonth->getMonthDays()) {
		
			sprintf(buffer, "%d", startDay);

			button = (Button*)_gadgets[buttonIndex];
			button->disable();
			button->setOutlineType(OUTLINE_CLICK_DEPENDENT);
			button->setText(buffer);

			startDay++;
			updatedDays++;
			buttonIndex++;
		}

		// Reset start day
		startDay = 1;

		delete previousMonth;
	}

	// Set all buttons for this month
	while (startDay <= thisMonth->getMonthDays()) {
		sprintf(buffer, "%d", startDay);

		button = (Button*)_gadgets[buttonIndex];
		button->enable();
		button->setOutlineType(OUTLINE_CLICK_DEPENDENT);

		// Select the day if necessary
		if (_visibleDate->getYear() == _date->getYear()) {
			if (_visibleDate->getMonth() == _date->getMonth()) {
				if (_date->getDay() == startDay) {
					button->setOutlineType(OUTLINE_IN);
					_selectedDayButton = button;
				}
			}
		}

		button->setText(buffer);

		startDay++;
		updatedDays++;
		buttonIndex++;
	}

	// Set all buttons for following month
	startDay = 1;
	while (updatedDays < maxDays) {
		sprintf(buffer, "%d", startDay);

		button = (Button*)_gadgets[buttonIndex];
		button->disable();
		button->setOutlineType(OUTLINE_CLICK_DEPENDENT);
		button->setText(buffer);

		startDay++;
		updatedDays++;
		buttonIndex++;
	}

	delete thisMonth;
}

void Calendar::buildGUI() {

	Rect rect;
	getClientRect(rect);

	// Define basic button properties
	u8 buttonWidth = rect.width / CALENDAR_COLS;
	u8 buttonHeight = rect.height / (CALENDAR_ROWS + 2);
	u16 gridY = (rect.height - (buttonHeight * CALENDAR_ROWS)) + 1;
	Button* button;

	// Add arrows and month label
	_leftArrow = new Button(rect.x, rect.y, buttonWidth, buttonHeight, GLYPH_ARROW_LEFT);
	_leftArrow->setEventHandler(this);
	addGadget(_leftArrow);

	_rightArrow = new Button((rect.width - buttonWidth) + 1, rect.y, buttonWidth, buttonHeight, GLYPH_ARROW_RIGHT);
	_rightArrow->setEventHandler(this);
	addGadget(_rightArrow);

	// Month name
	_monthLabel = new Label(rect.x + buttonWidth, rect.y, rect.width - (buttonWidth << 1), buttonHeight, "");
	_monthLabel->setBorderless(true);
	addGadget(_monthLabel);

	// Add day labels
	Label* label;
	
	label = new Label(rect.x, rect.y + buttonHeight, buttonWidth, buttonHeight, "S");
	label->setBorderless(true);
	addGadget(label);

	label = new Label(rect.x + buttonWidth, rect.y + buttonHeight, buttonWidth, buttonHeight, "M");
	label->setBorderless(true);
	addGadget(label);

	label = new Label(rect.x + (buttonWidth * 2), rect.y + buttonHeight, buttonWidth, buttonHeight, "T");
	label->setBorderless(true);
	addGadget(label);

	label = new Label(rect.x + (buttonWidth * 3), rect.y + buttonHeight, buttonWidth, buttonHeight, "W");
	label->setBorderless(true);
	addGadget(label);

	label = new Label(rect.x + (buttonWidth * 4), rect.y + buttonHeight, buttonWidth, buttonHeight, "T");
	label->setBorderless(true);
	addGadget(label);

	label = new Label(rect.x + (buttonWidth * 5), rect.y + buttonHeight, buttonWidth, buttonHeight, "F");
	label->setBorderless(true);
	addGadget(label);

	label = new Label(rect.x + (buttonWidth * 6), rect.y + buttonHeight, buttonWidth, buttonHeight, "S");
	label->setBorderless(true);
	addGadget(label);

	// Prepare to build grid of days
	u8 allocatedDays = 0;
	u8 maxDays = CALENDAR_ROWS * CALENDAR_COLS;

	// Create all boxes for this month
	while (allocatedDays < maxDays) {
		button = new Button(rect.x + ((allocatedDays % CALENDAR_COLS) * buttonWidth), gridY + ((allocatedDays / CALENDAR_COLS) * buttonHeight), buttonWidth, buttonHeight, "");
		button->setEventHandler(this);
		button->setRefcon(allocatedDays + 1);

		addGadget(button);
		allocatedDays++;
	}
}

const u8 Calendar::getDay() const { return _date->getDay(); }

const u8 Calendar::getMonth() const { return _date->getMonth(); }

const u16 Calendar::getYear() const { return _date->getYear(); }