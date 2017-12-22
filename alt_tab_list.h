#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#undef signals
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <glib.h>
#include <ukwm/compositor/plugins/ukui_plugin.h>

bool InitUKUIPluginDBusComm(void);
void FinishUKUIPluginDBusComm(void);
bool UKUIPluginDBusServiceIsReady(void);
GList *DBusGetAltTabList(void);
void DBusActivateWindowByTabListIndex(int index);

char *newstr(char *old_str);
alt_tab_item *new_alt_tab_item();
void free_alt_tab_item(alt_tab_item *ati);
//GFunc free_alt_tab_item_from_glist(gpointer data, gpointer user_data);

#endif // DBUS_CLIENT_H
