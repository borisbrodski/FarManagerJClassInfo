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

#include "jtformat.h"


//Access flags (can be used in class, fields and methods)
#define ACC_PUBLIC     0x0001	//Declared public; may be accessed from outside its package.
#define ACC_PRIVATE    0x0002	//Declared private in source.
#define ACC_PROTECTED  0x0004 	//Declared protected in source.
#define ACC_STATIC     0x0008 	//Declared or implicitly static in source.
#define ACC_FINAL      0x0010	//Declared final; no subclasses allowed.
#define ACC_SUPER      0x0020	//Treat superclass methods specially when invoked by the invokespecial instruction.
#define ACC_VOLATILE   0x0040	//Declared volatile; cannot be cached.
#define ACC_TRANSIENT  0x0080	//Declared transient; not written or read by a persistent object manager.
#define ACC_INTERFACE  0x0200	//Is an interface, not a class.
#define ACC_ABSTRACT   0x0400	//Declared abstract; may not be instantiated.
#define ACC_SYNTHETIC  0x1000	//Declared synthetic; Not present in the source code.
#define ACC_ANNOTATION 0x2000	//Declared as an annotation type.
#define ACC_ENUM       0x4000	//Declared as an enum type.

//Base types
#define BTYPE_BYTE		'B'		//'byte': signed byte
#define BTYPE_CHAR		'C'		//'char': Unicode character
#define BTYPE_DBL		'D'		//'double': double-precision floating-point value
#define BTYPE_FLOAT		'F'		//'float': single-precision floating-point value
#define BTYPE_INT		'I'		//'int': integer
#define BTYPE_LONG		'J'		//'long': long integer
#define BTYPE_OBJ		'L'		//'reference': an instance of class
#define BTYPE_SHORT		'S'		//'short': signed short
#define BTYPE_BOOL		'Z'		//'boolean': true or false
#define BTYPE_REF		'['		//'reference': one array dimension
#define BTYPE_VOID		'V'		//'void'


jtformat::jtformat()
:	_short_type(true),
	_jo_view(true),
	_access(true)
{
}


wstring jtformat::format(const jclass::jmember& info) const
{
	wstring rv, args;
	parse_description(info.description, rv, args);

	wstring val;
	if (_access) {
		const wstring acc_name = access_name(info.access);
		if (!acc_name.empty()) {
			val = acc_name;
			val += L' ';
		}
	}
	val += rv;
	val += L' ';
	val += info.name;
	if (info.type != jclass::field)
		val += args;

	return val;
}


wstring jtformat::get_type_name(const jclass::jmember& info) const
{
	wstring rv, args;
	parse_description(info.description, rv, args);
	return rv;
}


bool jtformat::is_public(const jclass::jmember& info)
{
	return (info.access & ACC_PUBLIC) != 0;
}


void jtformat::as_java_object(wstring& val)
{
	size_t delim_pos = 0;
	while ((delim_pos = val.find(L'/', delim_pos)) != string::npos)
		val[delim_pos] = L'.';
}


wstring jtformat::access_name(const uint16_t val) const
{
	wstring acc;
	if (val & ACC_PUBLIC)		acc += L"public ";
	if (val & ACC_PRIVATE)		acc += L"private ";
	if (val & ACC_PROTECTED)	acc += L"protected ";
	if (val & ACC_STATIC)		acc += L"static ";
	if (val & ACC_FINAL)		acc += L"final ";
	if (val & ACC_VOLATILE)		acc += L"volatile ";
	if (val & ACC_SUPER)		acc += L"super ";
	if (val & ACC_TRANSIENT)	acc += L"transient ";
	if (val & ACC_INTERFACE)	acc += L"interface ";
	if (val & ACC_ABSTRACT)		acc += L"abstract ";
	if (val & ACC_SYNTHETIC)	acc += L"synthetic ";
	if (val & ACC_ANNOTATION)	acc += L"annotation ";
	if (val & ACC_ENUM)			acc += L"enum ";
	if (!acc.empty())
		acc.erase(acc.length() - 1);
	return acc;
}


const wchar_t* jtformat::type_name(const wchar_t* token, wstring& name) const
{
	assert(token && *token);

	switch (*token) {
		case BTYPE_BYTE:	name = L"byte";    return ++token;
		case BTYPE_CHAR:	name = L"char";	   return ++token;
		case BTYPE_DBL:		name = L"double";  return ++token;
		case BTYPE_FLOAT:	name = L"float";   return ++token;
		case BTYPE_INT:		name = L"int";	   return ++token;
		case BTYPE_LONG:	name = L"long";	   return ++token;
		case BTYPE_SHORT:	name = L"short";   return ++token;
		case BTYPE_BOOL:	name = L"boolean"; return ++token;
		case BTYPE_VOID:	name = L"void";	   return ++token;
	}

	if (*token == BTYPE_REF) {
		char arr_num = 1;
		while (*(++token) == BTYPE_REF)
			++arr_num;
		token = type_name(token, name);
		for (char i = 0; i < arr_num; ++i)
			name += L"[]";
		return token;
	}

	if (*token == BTYPE_OBJ) {
		const wchar_t* token_end = wcschr(token, L';');
		if (!token_end)
			throw exception("Incorrect type format: object without comma");
		name.assign(++token, token_end);
		if (_jo_view)
			as_java_object(name);
		if (_short_type) {
			const size_t ldp = name.rfind(_jo_view ? L'.' : L'/');
			if (ldp != string::npos)
				name.erase(0, ldp + 1);
		}
		return ++token_end;
	}

	throw exception("Incorrect type format: unknown identifier");
}


void jtformat::parse_description(const wstring& descr, wstring& ret_val, wstring& args) const
{
	if (descr.empty())
		return;

	const wchar_t* token = descr.c_str();

	if (*token == L'(') {
		//Parse arguments
		++token;
		args = L'(';
		while (*token && *token != L')') {
			wstring t_name;
			token = type_name(token, t_name);
			if (args.size() != 1)
				args += L", ";
			args += t_name;
		}
		args += L')';
		++token;
	}

	//Parse return value
	type_name(token, ret_val);
}
