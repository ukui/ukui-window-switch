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

#ifndef UKWS_WNCK_OPERATOR_H
#define UKWS_WNCK_OPERATOR_H

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#include <QList>

class UkwsWnckOperator
{
public:
    explicit UkwsWnckOperator(WnckScreen *screen=nullptr,
                              WnckWorkspace *workspace = nullptr,
                              WnckApplication *application = nullptr,
                              WnckClassGroup *classGroup = nullptr,
                              WnckWindow *window = nullptr);
    ~UkwsWnckOperator();

    void updateWorkspaceList(WnckScreen *screen = nullptr);
    void updateWindowList();
    void updateWlWindowList(quint32 wl_winId);

    WnckScreen *screen;
    WnckWorkspace *workspace;
    WnckApplication *application;
    WnckClassGroup *classGroup;
    WnckWindow *window;

    bool needCheckWorkspace;
    bool needCheckScreen;

    QList<WnckWorkspace *> *workspaceQList;
    QList<WnckWindow *> *windowQList;

private:
    void checkAndInitScreen();
    void checkAndInitWorkspaceAndScreen();

//    void checkAndInitApplication();
//    void checkAndInitClassGroup();
//    void checkAndInitWindow();
};


#endif // UKWS_WNCK_OPERATOR_H
