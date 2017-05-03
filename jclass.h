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


class jclass
{
public:
	//! Java class description.
	struct jclassinfo {
		wstring name;		///< This class name
		wstring super;		///< Super class name
		uint16_t access;	///< Access (ACC_*)
	};

	//! Member type.
	enum jmember_type {
		method,
		field
	};

	//! Java class method and field description.
	struct jmember {
		jmember_type type;		///< Member type
		wstring name;			///< Method name
		wstring description;	///< Method description
		uint16_t access;		///< Access (ACC_*)
	};

	/**
	 * Check for java class format of file.
	 * \param file_hdr file header data pointer
	 * \param file_hdr_len file header data length
	 * \return true if it is java class format
	 */
	static bool format_supported(const unsigned char* file_hdr, const size_t file_hdr_len);

	/**
	 * Read java class file.
	 * \param file_name class file name
	 * \param class_info class description
	 * \param members class members description array
	 * \return false if error
	 */
	bool read(const wchar_t* file_name, jclassinfo& class_info, vector<jmember>& members);

private:
	/**
	 * Convert number from Big endian to Little endian.
	 * \param v source value (BE)
	 * \return LE value
	 */
	template<class T> static T be2le(const T v)
	{
		const unsigned char* data = reinterpret_cast<const unsigned char*>(&v);
		T t = 0;
		for (size_t i = 0; i < sizeof(T); ++i)
			t |= T(data[sizeof(T) - 1 - i]) << (i << 3);
		return t;
	}

	/**
	 * Read java class file.
	 * \param file_name class file name
	 * \return false if error
	 */
	bool read_java_class(const wchar_t* file_name);

	/**
	 * Read constant pool description.
	 */
	void read_constant_pool();

	/**
	 * Read interfaces description.
	 */
	void read_interfaces();

	/**
	 * Read fields description.
	 */
	void read_fields();

	/**
	 * Read methods description.
	 */
	void read_methods();

	/**
	 * Read attributes description.
	 */
	void read_attributes();

	/**
	 * Get string by index from string table.
	 * \param index string index
	 * \return value
	 */
	wstring get_string(const uint16_t index) const;

	/**
	 * Get members description.
	 * \param type member type
	 * \param members output members array
	 */
	void get_member_descr(const jmember_type type, vector<jmember>& members) const;

	/**
	 * Read number from buffer with byte order swap (Big endian (Java) to Little endian).
	 * \return number
	 */
	template<class T> T read_num()
	{
		const unsigned char* data = read(sizeof(T));
		return be2le(*reinterpret_cast<const T*>(data));
	}

	/**
	 * Read data from buffer.
	 * \param len data length
	 * \return pointer to data
	 */
	const unsigned char* read(const size_t len);

private:
	//! Constant pool types
	enum const_pool_type {
		CONSTANT_Phantom = 0,	//This type used as phantom item (without data)
		CONSTANT_Class = 7,
		CONSTANT_Fieldref = 9,
		CONSTANT_Methodref = 10,
		CONSTANT_InterfaceMethodref = 11,
		CONSTANT_String = 8,
		CONSTANT_Integer = 3,
		CONSTANT_Float = 4,
		CONSTANT_Long = 5,
		CONSTANT_Double = 6,
		CONSTANT_NameAndType = 12,
		CONSTANT_Utf8 = 1
	};

	//! Method description
	struct j_method {
		uint16_t access_flag;
		uint16_t name_index;
		uint16_t descriptor_index;
	};

	//! Field description
	typedef j_method j_field;

	//! Constant pool item description
	struct const_pool_class {
		uint16_t name_index;
	};

	//! Constant pool item description
	struct const_pool_fieldref {
		uint16_t class_index;
		uint16_t name_and_type_index;
	};

	//! Constant pool item description
	typedef const_pool_fieldref const_pool_methodref;

	//! Constant pool item description
	typedef const_pool_fieldref const_pool_interfmethodref;

	//! Constant pool item description
	struct const_pool_string {
		uint16_t string_index;
	};

	//! Constant pool item description
	struct const_pool_integer {
		uint32_t bytes;
	};

	//! Constant pool item description
	typedef const_pool_integer const_pool_float;

	//! Constant pool item description
	struct const_pool_long {
		uint32_t high_bytes;
		uint32_t low_bytes;
	};

	//! Constant pool item description
	typedef const_pool_long const_pool_double;

	//! Constant pool item description
	struct const_pool_nameandtype {
		uint16_t name_index;
		uint16_t descriptor_index;
	};

	//! Constant pool item description
	struct const_pool_utf8 {
		uint16_t length;
		char bytes;
	};

	//! Constant pool description
	struct j_const_pool {
		j_const_pool() : type(CONSTANT_Phantom), data(NULL) {}
		const_pool_type type;
		union {
			const unsigned char* data;
			const const_pool_class* cp_class;
			const const_pool_fieldref* cp_fieldref;
			const const_pool_methodref* cp_methodref;
			const const_pool_interfmethodref* cp_interfmethodref;
			const const_pool_string* cp_string;
			const const_pool_integer* cp_integer;
			const const_pool_float* cp_float;
			const const_pool_long* cp_long;
			const const_pool_double* cp_double;
			const const_pool_nameandtype* cp_nameandtype;
			const const_pool_utf8* cp_utf8;
		};
	};

private:
	vector<unsigned char>	_data_buff;	///< File content buffer
	size_t					_data_pos;	///< Position in buffer

	uint16_t	_class_access_flag;		///< Class access flags
	uint16_t	_class_name;			///< Reference to index from constant pool described this class name
	uint16_t	_super_class;			///< Reference to index from constant pool described this super name

	map<uint16_t, j_field> _fields;		///< Class fields description
	map<uint16_t, j_method> _methods;	///< Class methods description
	vector<j_const_pool> _const_pool;	///< Constant pool description
};
