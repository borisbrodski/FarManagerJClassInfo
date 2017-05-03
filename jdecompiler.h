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
#include "jclass.h"


class jdecompiler
{
public:
	//! Used decompilator.
	enum decompiler {
		jd_jad,
		jd_fernflower,
		jd_javap
	};

	/**
	 * Decompile java class file.
	 * \param file_name java class name
	 * \param jd used decompilator
	 * \return false if error
	 */
	bool decompile(const wchar_t* file_name, const decompiler jd);

	/**
	 * Get line number in source java file for specified member.
	 * \param member member description
	 * \return line number (1 if not found)
	 */
	intptr_t find_line(const jclass::jmember& member) const;

	/**
	 * Get decompiled source file name.
	 * \return decompiled source file name
	 */
	const wchar_t* source_file() const { return _java_file_name.c_str(); }

private:
	/**
	 * Decompilation with JAD.
	 * \param file_name java class file name
	 * \return false if error
	 */
	bool decompile_jad(const wchar_t* file_name);

	/**
	 * Decompilation with Fernflower.
	 * \param file_name java class file name
	 * \return false if error
	 */
	bool decompile_fernflower(const wchar_t* file_name);

	/**
	 * Decompilation with javap.
	 * \param file_name java class file name
	 * \return false if error
	 */
	bool decompile_javap(const wchar_t* file_name);
	
	/**
	 * Execute program.
	 * \param exe executable module
	 * \param params execution parameters
	 * \param stdout_file redirected stdout file handle (INVALID_HANDLE_VALUE to ignore)
	 * \param expected_code expected exit code by process (0xFFFFFFFF to ignore)
	 * \return false if error
	 */
	bool execute(const wchar_t* exe, const wchar_t* params, HANDLE stdout_file = INVALID_HANDLE_VALUE, const DWORD expected_code = 0xFFFFFFFF) const;

	/**
	 * Get plug-in module path.
	 * \return plug-in module path
	 */
	wstring module_path() const;

	/**
	 * Get temporary path.
	 * \return temporary path
	 */
	wstring get_tmp_path() const;

	/**
	 * Find java executable module path.
	 * \param java_bin_path java executable path
	 * \return false if error (java interpreter not found)
	 */
	bool find_java_bin(wstring& java_bin_path) const;

	/**
	 * Get java home path from registry.
	 * \param key_path registry key path
	 * \return java home path from registry
	 */
	wstring get_javahome_path(const wchar_t* key_path) const;

	/**
	 * Convert wide string to ansi UTF8.
	 * \param val source wide string
	 * \return ansi UTF8 string
	 */
	string w2a(const wstring& val) const;

private:
	wstring _java_bin_path;		///< Java interpreter bin directory path
	wstring _java_file_name;	///< Destination java source file
};
