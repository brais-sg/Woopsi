#include "multilinetextbox.h"
#include "fontbase.h"
#include "text.h"
#include "graphicsport.h"
#include "woopsifuncs.h"
#include "stringiterator.h"
#include "woopsitimer.h"
#include "woopsikey.h"

using namespace WoopsiUI;

MultiLineTextBox::MultiLineTextBox(s16 x, s16 y, u16 width, u16 height, const WoopsiString& text, u32 flags, s16 maxRows, GadgetStyle* style) : ScrollingPanel(x, y, width, height, flags, style) {

	_hAlignment = TEXT_ALIGNMENT_HORIZ_CENTRE;
	_vAlignment = TEXT_ALIGNMENT_VERT_CENTRE;
	_padding = 2;
	_topRow = 0;

	Rect rect;
	getClientRect(rect);
	_text = new Text(getFont(), "", rect.width - (_padding << 1));

	_flags.draggable = true;
	_maxRows = maxRows;

	calculateVisibleRows();

	// Set maximum rows if value not set
	if (_maxRows == 0) {
		_maxRows = _visibleRows + 1;
	}

	_cursorPos = 0;
	_showCursor = false;

	setText(text);
}

void MultiLineTextBox::drawText(GraphicsPort* port, s32 topRow, s32 bottomRow) {

	// Early exit checks
	if ((topRow < 0) && (bottomRow < 0)) return;
	if ((bottomRow >= _text->getLineCount()) && (topRow >= _text->getLineCount())) return;

	// Prevent overflows
	if (topRow < 0) topRow = 0;
	if (bottomRow >= _text->getLineCount()) bottomRow = _text->getLineCount() - 1;

	// Draw lines of text
	s16 textX;
	s16 textY;
	s32 currentRow = topRow;
	u8 rowLength = 0;

	// Draw all rows in this region
	while (currentRow <= bottomRow) {

		rowLength = _text->getLineTrimmedLength(currentRow);

		textX = getRowX(currentRow) + _canvasX;
		textY = getRowY(currentRow) + _canvasY;
		
		if (isEnabled()) {
			port->drawText(textX, textY, _text->getFont(), *_text, _text->getLineStartIndex(currentRow), rowLength);
		} else {
			port->drawText(textX, textY, _text->getFont(), *_text, _text->getLineStartIndex(currentRow), rowLength, getDarkColour());
		}

		currentRow++;
	}
}

void MultiLineTextBox::drawTextTop(GraphicsPort* port) {

	Rect rect;
	port->getClipRect(rect);

	// Early exit if there is no text to display
	if (_text->getLineCount() == 0) return;

	// Calculate various values needed to output text for this cliprect
	u8 lineHeight = _text->getLineHeight();
	s32 regionY = -_canvasY + rect.y;						// Y co-ord of the visible region of this canvas

	s32 topRow = (regionY / lineHeight) - 1;				// Calculate the top line of text in this region
	s32 bottomRow = ((regionY + rect.height) / lineHeight);	// Calculate bottom line of text

	// Draw lines of text
	drawText(port, topRow, bottomRow);	
}

void MultiLineTextBox::drawContents(GraphicsPort* port) {

	port->drawFilledRect(0, 0, _width, _height, getBackColour());

	// Always use top alignment if the number of rows of text exceeds or is
	// equal to the number of visible rows
	if (_visibleRows <= _text->getLineCount()) {
		drawTextTop(port);
	} else {
		drawText(port, 0, _text->getLineCount());
	}

	// Draw the cursor
	drawCursor(port);
}

void MultiLineTextBox::drawBorder(GraphicsPort* port) {

	// Stop drawing if the gadget indicates it should not have an outline
	if (isBorderless()) return;

	port->drawBevelledRect(0, 0, _width, _height, getShadowColour(), getShineColour());
}

void MultiLineTextBox::drawCursor(GraphicsPort* port) {

	// Get the cursor co-ords
	if (_showCursor) {
		u32 cursorRow = 0;

		u16 cursorX = 0;
		s16 cursorY = 0;

		// Only calculate the cursor position if the cursor isn't at the start of the text
		if (_cursorPos > 0) {

			// Calculate the row in which the cursor appears
			cursorRow = _text->getLineContainingCharIndex(_cursorPos);

			// Cursor line offset gives us the distance of the cursor from the start of the line
			u8 cursorLineOffset = _cursorPos - _text->getLineStartIndex(cursorRow);
			
			StringIterator* iterator = _text->newStringIterator();
			iterator->moveTo(_text->getLineStartIndex(cursorRow));
			
			// Sum the width of each char in the row to find the x co-ord 
			for (s32 i = 0; i < cursorLineOffset; ++i) {
				cursorX += getFont()->getCharWidth(iterator->getCodePoint());
				iterator->moveToNext();
			}
			
			delete iterator;
		}

		// Add offset of row (taking into account canvas co-ord and text alignment)
		// to calculated value
		cursorX += getRowX(cursorRow) + _canvasX;

		// Calculate y co-ord of the cursor
		cursorY = getRowY(cursorRow) + _canvasY;

		// Draw cursor
		if ((u32)_cursorPos < _text->getLength()) {
			port->drawFilledXORRect(cursorX, cursorY, getFont()->getCharWidth(_text->getCharAt(_cursorPos)), getFont()->getHeight());
		} else {
			port->drawFilledXORRect(cursorX, cursorY, getFont()->getCharWidth(' '), getFont()->getHeight());
		}
	}
}

// Calculate values for centralised text
u8 MultiLineTextBox::getRowX(s32 row) {

	Rect rect;
	getClientRect(rect);

	u8 rowLength = _text->getLineTrimmedLength(row);
	u8 rowPixelWidth = _text->getFont()->getStringWidth(*_text, _text->getLineStartIndex(row), rowLength);

	// Calculate horizontal position
	switch (_hAlignment) {
		case TEXT_ALIGNMENT_HORIZ_CENTRE:
			return ((rect.width - (_padding << 1)) - rowPixelWidth) >> 1;
		case TEXT_ALIGNMENT_HORIZ_LEFT:
			return _padding;
		case TEXT_ALIGNMENT_HORIZ_RIGHT:
			return rect.width - rowPixelWidth - _padding;
	}

	// Will never be reached
	return 0;
}

s16 MultiLineTextBox::getRowY(s32 row) {

	s16 textY = 0;
	s16 startPos = 0;

    s32 canvasRows = 0;
    s32 textRows = 0;

	Rect rect;
	getClientRect(rect);

	// Calculate vertical position
	switch (_vAlignment) {
		case TEXT_ALIGNMENT_VERT_CENTRE:

			// Calculate the maximum number of rows
            canvasRows = _canvasHeight / _text->getLineHeight();
			textY = row * _text->getLineHeight();

            // Get the number of rows of text
            textRows = _text->getLineCount();
			
			// Ensure there's always one row
			if (textRows == 0) textRows = 1;

            // Calculate the start position of the block of text
            startPos = ((canvasRows - textRows) * _text->getLineHeight()) >> 1;

            // Calculate the row Y co-ordinate
			textY = startPos + textY;
			break;
		case TEXT_ALIGNMENT_VERT_TOP:
			textY = _padding + (row * _text->getLineHeight());
			break;
		case TEXT_ALIGNMENT_VERT_BOTTOM:
			textY = rect.height - (((_text->getLineCount() - row) * _text->getLineHeight())) - _padding;
			break;
	}

	return textY;
}

void MultiLineTextBox::calculateVisibleRows() {

	Rect rect;
	getClientRect(rect);

	_visibleRows = (rect.height - (_padding << 1)) / _text->getLineHeight();
}

void MultiLineTextBox::setTextAlignmentHoriz(TextAlignmentHoriz alignment) {
	_hAlignment = alignment;
	redraw();
}

void MultiLineTextBox::setTextAlignmentVert(TextAlignmentVert alignment) {
	_vAlignment = alignment;
	redraw();
}

const Text* MultiLineTextBox::getText() const {
	return _text;
}

void MultiLineTextBox::setText(const WoopsiString& text) {

	_text->setText(text);

	// Ensure that we have the correct number of rows
	if (_text->getLineCount() > _maxRows) {
		_text->stripTopLines(_text->getLineCount() - _maxRows);

		_canvasHeight = _text->getPixelHeight() + (_padding << 1);
	}

	// Update max scroll value
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);

		// Scroll to bottom of new text
		jump(0, -(_canvasHeight - _height));
	}

	redraw();

	_gadgetEventHandlers->raiseValueChangeEvent();
}

void MultiLineTextBox::appendText(const WoopsiString& text) {

	_text->append(text);

	// Ensure that we have the correct number of rows
	if (_text->getLineCount() > _maxRows) {
		_text->stripTopLines(_text->getLineCount() - _maxRows);

		_canvasHeight = _text->getPixelHeight() + (_padding << 1);
	}

	// Update max scroll value
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);

		// Scroll to bottom of new text
		jump(0, -(_canvasHeight - _height));
	}

	redraw();

	_gadgetEventHandlers->raiseValueChangeEvent();
}

void MultiLineTextBox::removeText(const u32 startIndex) {
	_text->remove(startIndex);

	moveCursorToPosition(startIndex);

	// Update max scroll value
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);

		// Scroll to bottom of new text
		jump(0, -(_canvasHeight - _height));
	}

	redraw();

	_gadgetEventHandlers->raiseValueChangeEvent();
}

void MultiLineTextBox::removeText(const u32 startIndex, const u32 count) {
	_text->remove(startIndex, count);

	moveCursorToPosition(startIndex);

	// Update max scroll value
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);

		// Scroll to bottom of new text
		jump(0, -(_canvasHeight - _height));
	}

	redraw();

	_gadgetEventHandlers->raiseValueChangeEvent();
}

void MultiLineTextBox::setFont(FontBase* font) {
	_style.font = font;
	_text->setFont(font);

	// Ensure that we have the correct number of rows
	if (_text->getLineCount() > _maxRows) {
		_text->stripTopLines(_text->getLineCount() - _maxRows);

		_canvasHeight = _text->getPixelHeight() + (_padding << 1);
	}

	// Update max scroll value
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);

		// Scroll to bottom of new text
		jump(0, -(_canvasHeight - _height));
	}

	redraw();
}

const u16 MultiLineTextBox::getPageCount() const {

	// Get client rect
	Rect clientRect;
	getClientRect(clientRect);

	// Return number of screens of text
	if (_visibleRows > 0) {
		u16 pages = _text->getLineCount() / _visibleRows;

		return pages + 1;
	} else {
		return 1;
	}
}

const u16 MultiLineTextBox::getCurrentPage() const {

	// Get client rect
	Rect clientRect;
	getClientRect(clientRect);

	// Calculate the top line of text
	s32 topRow = (-_canvasY / _text->getLineHeight());

	// Return the page on which the top row falls
	if (_visibleRows > 0) {
		return topRow / _visibleRows;
	} else {
		return 1;
	}
}

void MultiLineTextBox::onResize(u16 width, u16 height) {

	// Ensure the base class resize method is called
	ScrollingPanel::onResize(width, height);

	// Resize the canvas' width
	Rect rect;
	getClientRect(rect);
	_canvasWidth = rect.width;
	_canvasHeight = rect.height;
	_canvasX = 0;
	_canvasY = 0;

	calculateVisibleRows();

	// Re-wrap the text
	_text->setWidth(_width);
	_text->wrap();
	
	bool raiseEvent = false;

	// Ensure that we have the correct number of rows
	if (_text->getLineCount() > _maxRows) {
		_text->stripTopLines(_text->getLineCount() - _maxRows);

		_canvasHeight = _text->getPixelHeight() + (_padding << 1);
		raiseEvent = true;
	}

	// Update canvas height
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);
	}

	if (raiseEvent) _gadgetEventHandlers->raiseValueChangeEvent();
}

const u32 MultiLineTextBox::getTextLength() const {
	return _text->getLength();
}

void MultiLineTextBox::showCursor() {
	if (!_showCursor) {
		_showCursor = true;
		redraw();
	}
}

void MultiLineTextBox::hideCursor() {
	if (_showCursor) {
		_showCursor = false;
		redraw();
	}
}

void MultiLineTextBox::insertTextAtCursor(const WoopsiString& text) {
	insertText(text, getCursorPosition());
}

void MultiLineTextBox::moveCursorToPosition(const s32 position) {

	// Force position to within confines of string
	if (position < 0) {
		_cursorPos = 0;
	} else {
		s32 len = (s32)_text->getLength();
		_cursorPos = len > position ? position : len;
	}

	redraw();
}

void MultiLineTextBox::insertText(const WoopsiString& text, const u32 index) {
	// Get current text length - use this later to quickly get the length
	// of the inserted string to shift the cursor around
	u32 oldLen = _text->getLength();

	_text->insert(text, index);
	
	// Get the new string length and use it to calculate the length
	// of the inserted string
	u32 insertLen = _text->getLength() - oldLen;

	moveCursorToPosition(index + insertLen);

	// Update max scroll value
	if (_text->getLineCount() > _visibleRows) {
		_canvasHeight = _text->getPixelHeight() + (_padding << 1);

		// Scroll to bottom of new text
		jump(0, -(_canvasHeight - _height));
	}

	redraw();

	_gadgetEventHandlers->raiseValueChangeEvent();
}

void MultiLineTextBox::onClick(s16 x, s16 y) {
	startDragging(x, y);
}

void MultiLineTextBox::onKeyPress(KeyCode keyCode) {
	if (keyCode == KEY_CODE_LEFT) {
		if (_cursorPos > 0) {
			moveCursorToPosition(_cursorPos - 1);
		}
	} else if (keyCode == KEY_CODE_RIGHT) {
		if (_cursorPos < (s32)_text->getLength()) {
			moveCursorToPosition(_cursorPos + 1);
		}
	}
}

void MultiLineTextBox::onKeyRepeat(KeyCode keyCode) {
	if (keyCode == KEY_CODE_LEFT) {
		if (_cursorPos > 0) {
			moveCursorToPosition(_cursorPos - 1);
		}
	} else if (keyCode == KEY_CODE_RIGHT) {
		if (_cursorPos < (s32)_text->getLength()) {
			moveCursorToPosition(_cursorPos + 1);
		}
	}
}

void MultiLineTextBox::handleKeyboardPressEvent(const KeyboardEventArgs& e) {
	processKey(e.getKey());
}

void MultiLineTextBox::handleKeyboardRepeatEvent(const KeyboardEventArgs& e) {
	processKey(e.getKey());
}

void MultiLineTextBox::processKey(const WoopsiKey* key) {

	if (key->getKeyType() == WoopsiKey::KEY_BACKSPACE) {

		// Delete character in front of cursor
		if (_cursorPos > 0) removeText(_cursorPos - 1, 1);
	} else if (key->getValue() != '\0') {

		// Not modifier; append value
		insertTextAtCursor(key->getValue());
	} 
}
