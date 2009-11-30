#include "listdataitem.h"

using namespace WoopsiUI;

ListDataItem::ListDataItem(const char* text, const u32 value) {

	_text = new char[strlen(text) + 1];
	strcpy(_text, text);

	_value = value;
	_isSelected = false;
}

ListDataItem::~ListDataItem() {
	delete [] _text;
}

s8 ListDataItem::compareTo(const ListDataItem* item) const {
	return strcmp(_text, item->getText());
}