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

#undef signals
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/shm.h>

#include <ukwm/compositor/plugins/ukui_plugin.h>
#include "ukui_plugin_generated.h"
#include "alt_tab_list.h"

static GDBusConnection *pConnection = NULL;
static UkwmPlugin *pProxy = NULL;

char *newstr(char *old_str)
{
	char *str = NULL;

	if (old_str == NULL)
	{
		str = malloc(1);
		if (str != NULL)
			*str = '\0';
	}
	else
	{
		int l = strlen(old_str) + 1;
		str = malloc(l);
		if (str != NULL)
			strncpy(str, old_str, l);
	}

	return str;
}

alt_tab_item *new_alt_tab_item()
{
	alt_tab_item *ati = malloc(sizeof(alt_tab_item));
	memset(ati, 0x0, sizeof(alt_tab_item));

	return ati;
}

void free_alt_tab_item(alt_tab_item *ati)
{
	if (ati->title_name != NULL)
		free(ati->title_name);

	free(ati);
}

//GFunc free_alt_tab_item_from_glist(gpointer data, gpointer user_data)
//{
//	free_alt_tab_item((alt_tab_item *)data);
//}

bool UkwmPluginDBusServiceIsReady(void)
{
	gchar *owner_name = NULL;
	owner_name = g_dbus_proxy_get_name_owner((GDBusProxy *)pProxy);
	if (owner_name != NULL)
	{
		//		g_print("Owner Name: %s\n", owner_name);
		g_free(owner_name);
		return true;
	}
	else
	{
		//		g_print("Owner Name is NULL.\n");
		return false;
	}
}

bool InitUkwmPluginDBusComm(void)
{
	bool bRet = TRUE;
	GError *pConnError = NULL;
	GError *pProxyError = NULL;

	//g_print("InitDBusCommunication: Client started.\n");

	do
	{
		bRet = TRUE;

		/** First step: get a connection */
		if (pConnection == NULL)
			pConnection = g_bus_get_sync(UKUI_PLUGIN_BUS, NULL, &pConnError);

		if (pConnection == NULL)
		{
			g_print("InitDBusCommunication: Failed to connect to dbus. Reason: %s.\n", pConnError->message);
			g_error_free(pConnError);
			bRet = FALSE;
		}

		/** Second step: try to get a connection to the given bus.*/
		if (pProxy == NULL)
			pProxy = ukwm_plugin_proxy_new_sync(pConnection,
												 G_DBUS_PROXY_FLAGS_NONE,
												 UKUI_PLUGIN_BUS_NAME,
												 UKUI_PLUGIN_OBJECT_PATH,
												 NULL,
												 &pProxyError);
		if (pProxy == NULL)
		{
			g_print("InitDBusCommunication: Failed to create proxy. Reason: %s.\n", pProxyError->message);
			g_error_free(pProxyError);
			bRet = FALSE;
		}
		g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(pProxy), G_MAXINT);

	} while (bRet == FALSE);

	return true;
}

void FinishUkwmPluginDBusComm(void)
{
	//printf("DBus Debug: %s [%d]\n", __FUNCTION__, __LINE__);
	g_object_ref(pConnection);
	g_object_ref(pProxy);
	pConnection = NULL;
	pProxy = NULL;
}

GList *DBusGetAltTabList(void)
{
	GList *tab_list = NULL;

	gboolean bRet;
	int out_count = 0;
	GVariant *out_tab_list_gva;
	GError *error = NULL;

	bRet = ukwm_plugin_call_get_alt_tab_list_sync(pProxy, &out_count,
												   &out_tab_list_gva, NULL, &error);
	if (bRet == FALSE)
	{
		//printf("Can't get anything.\n");
		return NULL;
	}
	//printf("out_count = %d\n", out_count);

	char *title_name = NULL;

	GVariantIter *_iter;
	GVariant *_item;
	int i = 0;

	_iter = g_variant_iter_new(out_tab_list_gva);
	while (g_variant_iter_next(_iter, "v", &_item))
	{
		alt_tab_item *ati = new_alt_tab_item();

		g_variant_get(_item, "(siiii)", &title_name,
					  &ati->width,
					  &ati->height,
					  &ati->x,
					  &ati->y);
		ati->title_name = newstr(title_name);
		tab_list = g_list_append(tab_list, ati);
		//		printf("Win[%02d]: %s (%03d, %03d) -> (%03d, %03d)\n",
		//			   i + 1, ati->title_name,
		//			   ati->x, ati->y,
		//			   ati->x + ati->width,
		//			   ati->y + ati->height);

		g_variant_unref(_item);
		i++;
	}
	g_variant_iter_free(_iter);
	g_variant_unref(out_tab_list_gva);

	return tab_list;
}

void DBusActivateWindowByTabListIndex(int index)
{
	gboolean bRet;

	//printf("DBus Debug: %s [%d], index = %d\n", __FUNCTION__, __LINE__, index);
	bRet = ukwm_plugin_call_activate_window_by_tab_list_index_sync(pProxy, index, NULL, NULL);
	if (bRet == FALSE)
	{
		//printf("Can't activate window: [%d]\n", index);
	}
}
