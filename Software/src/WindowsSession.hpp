/*
 * EndSessionDetector.hpp
 *
 *	Created on: 01.03.2014
 *		Author: EternalWind
 *		Project: Lightpack
 *
 *	Lightpack is very simple implementation of the backlight for a laptop
 *
 *	Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *	Lightpack is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Lightpack is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once
#ifndef WINDOWS_SESSION_HPP
#define WINDOWS_SESSION_HPP

#include "SystemSession.hpp"

#if !defined WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>


namespace SystemSession {
	class WindowsEventFilter : public EventFilter
	{
	public:
		WindowsEventFilter();
		~WindowsEventFilter();
		bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) Q_DECL_OVERRIDE;
	private:
		HPOWERNOTIFY m_powerSettingNotificationHandle;
	};
}
#endif // WINDOWS_SESSION_HPP
