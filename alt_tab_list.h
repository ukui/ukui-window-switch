/*
 * Copyright (C) 2017 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#undef signals
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <glib.h>
#include <ukwm/compositor/plugins/ukui_plugin.h>

bool InitUkwmPluginDBusComm(void);
void FinishUkwmPluginDBusComm(void);
bool UkwmPluginDBusServiceIsReady(void);
GList *DBusGetAltTabList(void);
void DBusActivateWindowByTabListIndex(int index);

char *newstr(char *old_str);
alt_tab_item *new_alt_tab_item();
void free_alt_tab_item(alt_tab_item *ati);
//GFunc free_alt_tab_item_from_glist(gpointer data, gpointer user_data);

#endif // DBUS_CLIENT_H
