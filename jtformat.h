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


class jtformat
{
public:
	jtformat();

	/**
	 * Set short type format for name (java.util.Map -> Map)
	 * \param v format type
	 */
	void set_short_type(const bool v)	{ _short_type = v; }

	/**
	 * Set java object format for name (java/util/Map -> java.util.Map)
	 * \param v format type
	 */
	void set_jo_view(const bool v)		{ _jo_view = v; }

	/**
	 * Set access name info format
	 * \param v format type
	 */
	void set_access(const bool v)		{ _access = v; }

	/**
	 * Format member info as text
	 * \param info member info structure description
	 * \return member info text description
	 */
	wstring format(const jclass::jmember& info) const;

	/**
	 * Get type name of member
	 * \param info member
	 * \return type name
	 */
	wstring get_type_name(const jclass::jmember& info) const;

	/**
	 * Check for member access type
	 * \param info member info structure description
	 * \return true if member is public
	 */
	static bool is_public(const jclass::jmember& info);

	/**
	 * Convert java object name to java format (java/util/Map -> java.util.Map)
	 * \param val object name
	 */
	static void as_java_object(wstring& val);

private:
	/**
	 * Get access name for specified type
	 * \param val access type
	 * \return access name
	 */
	wstring access_name(const uint16_t val) const;

	/**
	 * Parse member description
	 * \param token token (start description)
	 * \param name type name
	 * \return end token
	 */
	const wchar_t* type_name(const wchar_t* token, wstring& name) const;

	/**
	 * Parse member description
	 * \param descr member description
	 * \param ret_val member return value (for methods) or typ (for fields)
	 * \param args methods arguments
	 */
	void parse_description(const wstring& descr, wstring& ret_val, wstring& args) const;

private:
	bool _short_type;
	bool _jo_view;
	bool _access;
};
