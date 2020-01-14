/*
 * Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
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

#ifndef UKWS_COMMON_H
#define UKWS_COMMON_H

#define UKWS_OBJ_IND_SUBWIDGET      "indicator_sub_widget"
#define UKWS_OBJ_IND_MAINWIDGET     "indicator_main_widget"
#define UKWS_OBJ_WS_MGR_SUBWIDGET   "ws_manager_sub_widget"
#define UKWS_OBJ_WS_MGR_MAINWIDGET  "ws_manager_main_widget"
#define UKWS_OBJ_FLOW_SCROLLBAR     "flow_scrollbar"
#define UKWS_OBJ_WINBOX             "winbox"
#define UKWS_OBJ_WINBOX_WIN_NAME    "winbox-wintitle"
#define UKWS_OBJ_WINBOX_THUMBNAIL   "winbox-thumbnail"
#define UKWS_OBJ_WSBOX              "wsbox"
#define UKWS_OBJ_WSBOX_THUMBNAIL    "wsbox-thumbnail"

enum UkwsWidgetShowStatus {
    Hidden = 0,
    Shown,
    Constructing,
    Destructing,
    Interrupted,
};

#endif // UKWS_COMMON_H
