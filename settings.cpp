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

#include "settings.h"
#include "version.h"

bool settings::view_access = true;
bool settings::view_as_jo = true;
bool settings::view_sob = true;
bool settings::add_to_panel_menu = false;
wstring settings::cmd_prefix = L"jclassinfo";


#define SAVE_SETTINGS(s, p) s.set(L ## #p, p);
#define LOAD_SETTINGS(s, p) p = s.get(L ## #p, p);

class settings_serializer
{
public:
	settings_serializer()
		: _handle(INVALID_HANDLE_VALUE)
	{
		FarSettingsCreate fsc;
		ZeroMemory(&fsc, sizeof(fsc));
		fsc.StructSize = sizeof(fsc);
		fsc.Guid = _FPG;
		fsc.Handle = INVALID_HANDLE_VALUE;
		if (_PSI.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &fsc))
			_handle = fsc.Handle;
	}

	~settings_serializer()
	{
		_PSI.SettingsControl(_handle, SCTL_FREE, 0, nullptr);
	}

	/**
	 * Set value (scalar types).
	 * \param name value name
	 * \param val value
	 * \return false if error
	 */
	template<class T> bool set(const wchar_t* name, const T& val) const
	{
		assert(name && *name);

		FarSettingsItem fsi;
		ZeroMemory(&fsi, sizeof(fsi));
		fsi.StructSize = sizeof(fsi);
		fsi.Name = name;
		fsi.Type = FST_QWORD;
		fsi.Number = static_cast<unsigned __int64>(val);
		return (_PSI.SettingsControl(_handle, SCTL_SET, 0, &fsi) != FALSE);
	}

	/**
	 * Set value (string type).
	 * \param name value name
	 * \param val value
	 * \return false if error
	 */
	bool set(const wchar_t* name, const wstring& val) const
	{
		assert(name && *name);

		FarSettingsItem fsi;
		ZeroMemory(&fsi, sizeof(fsi));
		fsi.StructSize = sizeof(fsi);
		fsi.Name = name;
		fsi.Type = FST_STRING;
		fsi.String = val.c_str();
		return (_PSI.SettingsControl(_handle, SCTL_SET, 0, &fsi) != FALSE);
	}

	/**
	 * Set value (boolean type).
	 * \param name value name
	 * \param val value
	 * \return false if error
	 */
	bool set(const wchar_t* name, const bool& val) const
	{
		return set<unsigned char>(name, val ? 1 : 0);
	}

	/**
	 * Get value (scalar types).
	 * \param name value name
	 * \param default_val default value
	 * \return value
	 */
	template<class T> T get(const wchar_t* name, const T& default_val) const
	{
		assert(name && *name);

		FarSettingsItem fsi;
		ZeroMemory(&fsi, sizeof(fsi));
		fsi.StructSize = sizeof(fsi);
		fsi.Name = name;
		fsi.Type = FST_QWORD;
		return (_PSI.SettingsControl(_handle, SCTL_GET, 0, &fsi) ? static_cast<T>(fsi.Number) : default_val);
	}

	/**
	 * Get value (string types).
	 * \param name value name
	 * \param default_val default value
	 * \return value
	 */
	wstring get(const wchar_t* name, const wstring& default_val) const
	{
		assert(name && *name);

		FarSettingsItem fsi;
		ZeroMemory(&fsi, sizeof(fsi));
		fsi.StructSize = sizeof(fsi);
		fsi.Name = name;
		fsi.Type = FST_STRING;
		return (_PSI.SettingsControl(_handle, SCTL_GET, 0, &fsi) ? wstring(fsi.String) : default_val);
	}

	/**
	 * Get value (boolean types).
	 * \param name value name
	 * \param default_val default value
	 * \return value
	 */
	bool get(const wchar_t* name, const bool& default_val) const
	{
		return get<unsigned char>(name, default_val ? 1 : 0) != 0;
	}

private:
	HANDLE _handle;
};


void settings::load()
{
	settings_serializer s;
	LOAD_SETTINGS(s, view_access);
	LOAD_SETTINGS(s, view_as_jo);
	LOAD_SETTINGS(s, view_sob);
	LOAD_SETTINGS(s, add_to_panel_menu);
	LOAD_SETTINGS(s, cmd_prefix);
}


void settings::save()
{
	settings_serializer s;
	SAVE_SETTINGS(s, view_access);
	SAVE_SETTINGS(s, view_as_jo);
	SAVE_SETTINGS(s, view_sob);
	SAVE_SETTINGS(s, add_to_panel_menu);
	SAVE_SETTINGS(s, cmd_prefix);
}


bool settings::configure()
{
	bool sett_changed = false;

	const FarDialogItem dlg_items[] = {
		/*  0 */ { DI_DOUBLEBOX, 3, 1, 47, 10, 0, nullptr, nullptr, LIF_NONE, TEXT(PLUGIN_NAME) },
		/*  1 */ { DI_CHECKBOX,  5, 2, 45, 2, view_access ? 1 : 0, nullptr, nullptr, LIF_NONE, L"View access modifiers" },
		/*  2 */ { DI_CHECKBOX,  5, 3, 45, 3, view_as_jo ? 1 : 0, nullptr, nullptr, LIF_NONE, L"Replace slashes to dots" },
		/*  3 */ { DI_CHECKBOX,  5, 4, 45, 4, view_sob ? 1 : 0, nullptr, nullptr, LIF_NONE, L"Short objects names" },
		/*  4 */ { DI_TEXT,      0, 5,  0, 5, 0, nullptr, nullptr, DIF_SEPARATOR },
		/*  5 */ { DI_CHECKBOX,  5, 6, 45, 6, add_to_panel_menu ? 1 : 0, nullptr, nullptr, LIF_NONE, L"Add plug-in to the panel plug-in menu" },
		/*  6 */ { DI_TEXT,      5, 7, 45, 7, 0, nullptr, nullptr, LIF_NONE, L"Plug-in command prefix:" },
		/*  7 */ { DI_EDIT,     29, 7, 45, 7, 0, nullptr, nullptr, LIF_NONE, cmd_prefix.c_str() },
		/*  8 */ { DI_TEXT,      0, 8,  0, 8, 0, nullptr, nullptr, DIF_SEPARATOR },
		/*  9 */ { DI_BUTTON,    0, 9,  0, 9, 0, nullptr, nullptr, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, L"Save" },
		/* 10 */ { DI_BUTTON,    0, 9,  0, 9, 0, nullptr, nullptr, DIF_CENTERGROUP, L"Cancel" }
	};

	const HANDLE dlg = _PSI.DialogInit(&_FPG, &_FPG, -1, -1, 51, 12, nullptr, dlg_items, sizeof(dlg_items) / sizeof(dlg_items[0]), 0, FDLG_NONE, nullptr, nullptr);
	const intptr_t rc = _PSI.DialogRun(dlg);
	sett_changed = (rc >= 0 && rc != sizeof(dlg_items) / sizeof(dlg_items[0]) - 1);
	if (sett_changed) {
		view_access = _PSI.SendDlgMessage(dlg, DM_GETCHECK, 1, nullptr) != 0;
		view_as_jo = _PSI.SendDlgMessage(dlg, DM_GETCHECK, 2, nullptr) != 0;
		view_sob = _PSI.SendDlgMessage(dlg, DM_GETCHECK, 3, nullptr) != 0;
		add_to_panel_menu = _PSI.SendDlgMessage(dlg, DM_GETCHECK, 5, nullptr) != 0;
		cmd_prefix = reinterpret_cast<const wchar_t*>(_PSI.SendDlgMessage(dlg, DM_GETCONSTTEXTPTR, 7, nullptr));
		save();
	}
	_PSI.DialogFree(dlg);

	return sett_changed;
}
