/**************************************************************************
 *  JClassInfo plug-in for FAR 3.0                                        *
 *  Copyright (C) 2012-2014 by Artem Senichev <artemsen@gmail.com>        *
 *  https://sourceforge.net/projects/farplugs/                            *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include "panel.h"
#include "jtformat.h"
#include "settings.h"
#include "jdecompiler.h"
#include "version.h"


panel* panel::open(const wchar_t* file_name, const bool silent)
{
	assert(file_name && file_name[0]);

	panel* instance = new panel();

	jclass jc;
	jclass::jclassinfo jclass_info;
	if (!jc.read(file_name, jclass_info, instance->_jmembers)) {
		delete instance;
		instance = nullptr;
	}
	else {
		instance->_file_name = file_name;
		instance->_title = jclass_info.name;
		jtformat::as_java_object(instance->_title);
	}

	if (!silent && instance == nullptr) {
		const wchar_t* err_msg[] = { TEXT(PLUGIN_NAME), L"Unable to open file as Java class", file_name };
		_PSI.Message(&_FPG, &_FPG, FMSG_WARNING | FMSG_MB_OK, nullptr, err_msg, sizeof(err_msg) / sizeof(err_msg[0]), 0);
	}

	return instance;
}


void panel::get_panel_info(OpenPanelInfo& info)
{
	//Configure key bar
	static KeyBarLabel kbl[] = {
		{ { VK_F3, 0 }, L"JAD", L"JAD" },
		{ { VK_F4, 0 }, L"Fernfl", L"Fernflower" },
		{ { VK_F5, 0 }, L"Javap", L"Javap" },
		{ { VK_F6, 0 }, L"", L"" },
		{ { VK_F7, 0 }, L"", L"" },
		{ { VK_F8, 0 }, L"", L"" },
		{ { VK_F1, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F2, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F3, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F4, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F5, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F6, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F7, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F8, SHIFT_PRESSED }, L"", L"" },
		{ { VK_F3, RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED }, L"", L"" },
		{ { VK_F4, RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED }, L"", L"" },
		{ { VK_F5, RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED }, L"", L"" },
		{ { VK_F6, RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED }, L"", L"" },
	};

	static KeyBarTitles kbt;
	static PanelMode panel_modes[10];

	static bool init = false;
	if (!init) {
		kbt.Labels = kbl;
		kbt.CountLabels = sizeof(kbl) / sizeof(kbl[0]);

		//Configure one panel view for all modes
		static const wchar_t* column_titles[] = { L"Member" };
		ZeroMemory(&panel_modes, sizeof(panel_modes));
		for (size_t i = 0; i < sizeof(panel_modes) / sizeof(panel_modes[0]); ++i) {
			panel_modes[i].ColumnTypes =  L"N";
			panel_modes[i].ColumnWidths = L"0";
			panel_modes[i].ColumnTitles = column_titles;
			panel_modes[i].StatusColumnTypes =  L"C0";
			panel_modes[i].StatusColumnWidths = L"0";
		}

		init = true;
	}

	info.StructSize = sizeof(info);
	info.PanelTitle = _title.c_str();
	info.HostFile = _file_name.c_str();
	info.Flags = OPIF_ADDDOTS | OPIF_DISABLEFILTER | OPIF_DISABLESORTGROUPS | OPIF_SHOWPRESERVECASE;
	info.StartPanelMode = '0';
	info.KeyBar = &kbt;
	info.PanelModesArray = panel_modes;
	info.PanelModesNumber = sizeof(panel_modes) / sizeof(panel_modes[0]);
}


void panel::get_panel_list(PluginPanelItem** items, size_t& items_count)
{
	items_count = _jmembers.size();
	*items = new PluginPanelItem[items_count];
	ZeroMemory(*items, sizeof(PluginPanelItem) * items_count);

	jtformat jfmt;
	jfmt.set_short_type(settings::view_sob);
	jfmt.set_jo_view(settings::view_as_jo);
	jfmt.set_access(settings::view_access);

	size_t idx = 0;
	for (vector<jclass::jmember>::const_iterator it = _jmembers.begin(); it != _jmembers.end(); ++it) {
		PluginPanelItem& item = (*items)[idx];

		if (!jtformat::is_public(*it))
			item.FileAttributes = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;

		item.FileSize = (it->type == jclass::method ? 1 : 0);
		item.NumberOfLinks = static_cast<DWORD>(idx);

		const wstring descr = jfmt.format(*it);
		const size_t descr_size = descr.length() + 1;
		item.FileName = new wchar_t[descr_size];
		wcscpy_s(const_cast<wchar_t*>(item.FileName), descr_size, descr.c_str());

		const size_t name_size = it->name.length() + 1;
		item.AlternateFileName = new wchar_t[name_size];
		wcscpy_s(const_cast<wchar_t*>(item.AlternateFileName), name_size, it->name.c_str());

		wchar_t** custom_column_data = new wchar_t*[1];
		const size_t сс_size = it->description.length() + 1;
		custom_column_data[0] = new wchar_t[сс_size];
		wcscpy_s(custom_column_data[0], сс_size, it->description.c_str());
		item.CustomColumnData = custom_column_data;
		item.CustomColumnNumber = 1;

		++idx;
	}
}


void panel::free_panel_list(PluginPanelItem* items, const size_t items_count)
{
	assert(items_count == 0 || items);

	for (size_t i = 0; i < items_count; ++i) {
		PluginPanelItem& item = items[i];

		delete[] item.FileName;
		delete[] item.AlternateFileName;

		for (size_t j = 0; j < item.CustomColumnNumber; ++j)
			delete[] item.CustomColumnData[j];
		delete[] item.CustomColumnData;
	}

	delete[] items;
}


bool panel::handle_keyboard(const KEY_EVENT_RECORD& key_event)
{
	if (key_event.dwControlKeyState == 0 && (key_event.wVirtualKeyCode == VK_F3 || key_event.wVirtualKeyCode == VK_F4 || key_event.wVirtualKeyCode == VK_F5)) {
		jdecompiler jd;
		jdecompiler::decompiler mode = jdecompiler::jd_jad;
		switch (key_event.wVirtualKeyCode) {
			case VK_F3: mode = jdecompiler::jd_jad; break;
			case VK_F4: mode = jdecompiler::jd_fernflower; break;
			case VK_F5: mode = jdecompiler::jd_javap; break;
		}

		if (jd.decompile(_file_name.c_str(), mode)) {
			intptr_t line_num = 1;

			//Get currently selected item (member) to determine line number
			const intptr_t ppi_len = _PSI.PanelControl(PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, nullptr);
			if (ppi_len != 0) {
				vector<unsigned char> buffer(ppi_len);
				PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(&buffer.front());
				FarGetPluginPanelItem fgppi;
				ZeroMemory(&fgppi, sizeof(fgppi));
				fgppi.StructSize = sizeof(fgppi);
				fgppi.Size = buffer.size();
				fgppi.Item = ppi;
				if (_PSI.PanelControl(PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, &fgppi) && ppi->NumberOfLinks < _jmembers.size())
					line_num = jd.find_line(_jmembers[ppi->NumberOfLinks]);
			}

			_PSI.Editor(jd.source_file(), _title.c_str(), 0, 0, -1, -1, EF_DELETEONCLOSE | EF_DISABLESAVEPOS | EF_DISABLEHISTORY, line_num, 1, CP_REDETECT);
		}
		return true;
	}
	return false;
}
