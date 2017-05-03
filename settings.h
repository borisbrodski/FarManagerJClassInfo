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

#include "common.h"


class settings
{
public:
	/**
	 * Load settings.
	 */
	static void load();

	/**
	 * Configure settings.
	 * \return true if settings was changed
	 */
	static bool configure();

private:
	/**
	 * Save settings.
	 */
	static void save();

public:
	static bool view_access;		///< View access modifier flag
	static bool view_as_jo;			///< Replace slashes to dots flag
	static bool view_sob;			///< Short objects names flag
	static bool add_to_panel_menu;	///< Add plug-in to the panel plug-in menu flag
	static wstring cmd_prefix;		///< Plug-in command prefix
};
