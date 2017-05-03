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

#pragma once

#include "jclass.h"


class panel
{
private:
	panel() {}

public:
	/**
	 * Open java class file.
	 * \param file_name java class file name
	 * \param silent silent mode flag (true to show error message)
	 * \return panel instance (nullptr on error)
	 */
	static panel* open(const wchar_t* file_name, const bool silent);

	/**
	 * Get panel info.
	 * \param info panel info
	 */
	void get_panel_info(OpenPanelInfo& info);

	/**
	 * Get panel list.
	 * \param items far panel items list
	 * \param items_count number of items
	 */
	void get_panel_list(PluginPanelItem** items, size_t& items_count);

	/**
	 * Free panel file list.
	 * \param items far panel items list
	 * \param items_count number of items
	 */
	void free_panel_list(PluginPanelItem* items, const size_t items_count);

	/**
	 * Handle keyboard event.
	 * \param key_event keyboard event
	 * \return true if event handled
	 */
	bool handle_keyboard(const KEY_EVENT_RECORD& key_event);

private:
	wstring	_title;						///< Panel title
	wstring	_file_name;					///< Host file name
	vector<jclass::jmember>	_jmembers;	///< Java class members descriptions
};
