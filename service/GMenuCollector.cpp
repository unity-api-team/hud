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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <common/WindowStackInterface.h>
#include <service/GMenuCollector.h>

#include <libqtgmenu/QtGMenuImporter.h>

#include <QStringList>
#include <QDebug>

using namespace hud::service;
using namespace qtgmenu;

GMenuCollector::GMenuCollector(const QString &name,
		const QDBusObjectPath &actionPath, const QDBusObjectPath &menuPath) :
		m_name(name), m_actionPath(actionPath), m_menuPath(menuPath) {

	m_importer.reset(
			new QtGMenuImporter(m_name, m_menuPath.path(),
					m_actionPath.path()));
}

GMenuCollector::~GMenuCollector() {
}

bool GMenuCollector::isValid() const {
	return !m_menuPath.path().isEmpty();
}

CollectorToken::Ptr GMenuCollector::activate() {
	CollectorToken::Ptr collectorToken(m_collectorToken);

	if (collectorToken.isNull()) {
		qDebug() << "GMenuCollector::opening menus" << m_name
				<< m_actionPath.path() << m_menuPath.path();
		collectorToken.reset(new CollectorToken(shared_from_this()));
		m_collectorToken = collectorToken;

		m_menu = m_importer->GetQMenu();
	}

	return collectorToken;
}

void GMenuCollector::deactivate() {
	qDebug() << "GMenuCollector::deactivate";
}

QMenu * GMenuCollector::menu() {
	if (m_menu) {
		return m_menu.get();
	} else {
		return nullptr;
	}
}
