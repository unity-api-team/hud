/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include "QtGMenuConverter.h"
#include <internal/QtGMenuImporterPrivate.h>

#undef signals
#include <gio/gio.h>

using namespace qtgmenu;

namespace qtgmenu
{

class QtGMenuConverterPrivate
{
};

} // namespace qtgmenu

QtGMenuConverter::QtGMenuConverter()
    : d( new QtGMenuConverterPrivate() )
{
}

QtGMenuConverter::~QtGMenuConverter()
{
}

std::shared_ptr< QMenu > QtGMenuConverter::ToQMenu( const GMenuModel& from_menu )
{
  return std::shared_ptr < QMenu > ( new QMenu() );
}

GMenuModel* QtGMenuConverter::ToGMenuModel( const QMenu& from_menu )
{
  return G_MENU_MODEL( g_menu_new() ) ;
}
