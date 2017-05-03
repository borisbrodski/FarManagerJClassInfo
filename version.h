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

#include <plugin.hpp>


#define PLUGIN_VER_MAJOR	3
#define PLUGIN_VER_MINOR	10
#define PLUGIN_VER_BUILD	0
#define PLUGIN_FAR_BUILD	FARMANAGERVERSION_BUILD

#define PLUGIN_NAME			"Java class info"
#define PLUGIN_AUTHOR		"Artem Senichev (+BB)"
#define PLUGIN_DESCR		"Java class file viewer and decompiler"
#define PLUGIN_FILENAME		"JClassInfo.dll"

#define VSTR__(v) #v
#define VSTR(v) VSTR__(v)
#define PLUGIN_VERSION_TXT VSTR(PLUGIN_VER_MAJOR) "." VSTR(PLUGIN_VER_MINOR) "." VSTR(PLUGIN_VER_BUILD) "." VSTR(PLUGIN_FAR_BUILD)
#define PLUGIN_VERSION_NUM PLUGIN_VER_MAJOR,PLUGIN_VER_MINOR,PLUGIN_VER_BUILD,FARMANAGERVERSION_BUILD
