//this file is part of BBWin
//Copyright (C)2006-2008 Etienne GRIGNON  ( etienne.grignon@gmail.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// $Id$

#ifndef 	__IBBWINEXCEPTION_H__
#define		__IBBWINEXCEPTION_H__

class IBBWinException {
	protected :
		std::string			msg;
		
	public:
		virtual ~IBBWinException() { };
		virtual std::string getMessage() { return msg; } ;
};

#endif // !__IBBWINEXCEPTION_H__