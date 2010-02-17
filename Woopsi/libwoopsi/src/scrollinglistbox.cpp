#include "scrollinglistbox.h"
#include "scrollbarvertical.h"
#include "graphicsport.h"

using namespace WoopsiUI;

ScrollingListBox::ScrollingListBox(s16 x, s16 y, u16 width, u16 height, GadgetStyle* style) : Gadget(x, y, width, height, 0, style) {
	_scrollbarWidth = 10;

	setBorderless(true);

	_listbox = new ListBox(0, 0, width - _scrollbarWidth, height, &_style);
	_listbox->addGadgetEventHandler(this);

	// Create scrollbar
	Rect rect;
	_listbox->getClientRect(rect);
	_scrollbar = new ScrollbarVertical(width - _scrollbarWidth, 0, _scrollbarWidth, height, &_style);
	_scrollbar->setMinimumValue(0);
	_scrollbar->setMaximumValue(0);
	_scrollbar->setPageSize(rect.height / _listbox->getOptionHeight());
	_scrollbar->addGadgetEventHandler(this);

	// Add children to child array
	addGadget(_listbox);
	addGadget(_scrollbar);
}

void ScrollingListBox::drawContents(GraphicsPort* port) {
	port->drawFilledRect(0, 0, _width, _height, getBackColour());
}

void ScrollingListBox::handleValueChangeEvent(const GadgetEventArgs& e) {

	if (e.getSource() != NULL) {
		if (e.getSource() == _scrollbar) {

			if (_listbox != NULL) {
				_listbox->setRaisesEvents(false);
				_listbox->jump(0, 0 - (_scrollbar->getValue() * _listbox->getOptionHeight()));
				_listbox->setRaisesEvents(true);
			}
		}
	}
}

void ScrollingListBox::handleScrollEvent(const GadgetEventArgs& e) {

	if (e.getSource() != NULL) {
		if (e.getSource() == _listbox) {

			if (_scrollbar != NULL) {
				_scrollbar->setRaisesEvents(false);

				s32 value = ((0 - _listbox->getCanvasY()) << 16) / _listbox->getOptionHeight();

				// Round up
				value += value & 0x8000;
				
				value >>= 16;

				_scrollbar->setValue(value);
				_scrollbar->setRaisesEvents(true);
			}
		}
	}
}

void ScrollingListBox::handleDoubleClickEvent(const GadgetEventArgs& e) {

	if (e.getSource() != NULL) {
		if (e.getSource() == _listbox) {

			// Raise double-click events from list box to event handler
			_gadgetEventHandlers->raiseDoubleClickEvent(e.getX(), e.getY());
		}
	}
}

void ScrollingListBox::onResize(u16 width, u16 height) {

	// Resize the children
	_listbox->resize(width - _scrollbarWidth, height);
	_scrollbar->resize(_scrollbarWidth, height);

	// Adjust scrollbar page size
	Rect rect;
	getClientRect(rect);
	_scrollbar->setPageSize(rect.height / _listbox->getOptionHeight());

	// Move the scrollbar
	_scrollbar->moveTo(width - _scrollbarWidth, 0);
}

void ScrollingListBox::setFont(FontBase* font) {
	_style.font = font;
	_listbox->setFont(font);
	_scrollbar->setFont(font);
}

void ScrollingListBox::addOption(ListBoxDataItem* item) {
	_listbox->addOption(item);
	_scrollbar->setMaximumValue(_listbox->getOptionCount());
}

void ScrollingListBox::addOption(const WoopsiString& text, const u32 value) {
	_listbox->addOption(text, value);
	_scrollbar->setMaximumValue(_listbox->getOptionCount());
}

void ScrollingListBox::addOption(const WoopsiString& text, const u32 value, const u16 normalTextColour, const u16 normalBackColour, const u16 selectedTextColour, const u16 selectedBackColour) {
	_listbox->addOption(text, value, normalTextColour, normalBackColour, selectedTextColour, selectedBackColour);
	_scrollbar->setMaximumValue(_listbox->getOptionCount());
}

void ScrollingListBox::removeOption(const s32 index) {
	_listbox->removeOption(index);
	_scrollbar->setMaximumValue(_listbox->getOptionCount());

	// Reposition grip if necessary
	if (_scrollbar->getValue() > _listbox->getOptionCount()) _scrollbar->setValue(0);
}

void ScrollingListBox::removeAllOptions() {
	_listbox->removeAllOptions();
	_scrollbar->setMaximumValue(0);
	_scrollbar->setValue(0);
};

// Get the preferred dimensions of the gadget
void ScrollingListBox::getPreferredDimensions(Rect& rect) const {

	// Get the listbox's preferred dimensions
	_listbox->getPreferredDimensions(rect);

	// Add on the scrollbar width
	rect.width += _scrollbarWidth;
}
