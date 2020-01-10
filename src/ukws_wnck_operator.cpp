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

#include "ukws_wnck_operator.h"

#include <QDebug>

#include <glib/glist.h>
#include <xcb/xcb.h>

UkwsWnckOperator::UkwsWnckOperator(WnckScreen *screen, WnckWorkspace *workspace,
                                   WnckApplication *application,
                                   WnckClassGroup *classGroup,
                                   WnckWindow *window)
{
    this->screen = screen;
    this->workspace = workspace;
    this->application = application;
    this->classGroup = classGroup;
    this->window = window;

    this->needCheckWorkspace = false;
    this->needCheckScreen = false;

    workspaceQList = new QList<WnckWorkspace *>;
    windowQList = new QList<WnckWindow *>;
}

UkwsWnckOperator::~UkwsWnckOperator()
{
    /*
     * 注意：
     *     这里可能存在内存泄露，未在wnck官方文档中找到释放screen等结构体的函数
     *     所幸在本应用中，UkwsWnckOperator只有一个，应用退出后OS会自动回收资源，
     * 运行过程中无影响。
     */
    this->screen = nullptr;
    this->workspace = nullptr;
    this->application = nullptr;
    this->classGroup = nullptr;
    this->window = nullptr;

    workspaceQList->clear();
    windowQList->clear();
}

void UkwsWnckOperator::checkAndInitScreen()
{
    if (this->screen == nullptr) {
        if (this->workspace == nullptr) {
            /*
             * The returned WnckScreen is owned by libwnck and must not be
             *   referenced or unreferenced.
             * 来自官方文档的描述，无需释放screen，直接使用。
             */
            this->screen = wnck_screen_get_default();
        } else {
            this->screen = wnck_workspace_get_screen(this->workspace);
        }
    }
}

void UkwsWnckOperator::checkAndInitWorkspaceAndScreen()
{
    checkAndInitScreen();
    if (this->workspace == nullptr) {
        /*
         * The returned WnckWorkspace is owned by libwnck and must not be
         *   referenced or unreferenced.
         * 来自官方文档的描述，无需释放workspace，直接使用。
         */
        this->workspace = wnck_screen_get_active_workspace(this->screen);
    }
}

void UkwsWnckOperator::updateWorkspaceList(WnckScreen *screen)
{
    // workspaceQList所指向的实例都是来自于wnck的screen，无需释放
    workspaceQList->clear();

    this->screen = screen;
    checkAndInitScreen();

    wnck_screen_force_update(this->screen);
    // The list should not be modified nor freed, as it is owned by screen.
    // 来自wnck的官方解释，GList来自wnck的scren，无需释放
    GList *workspaceGlist = wnck_screen_get_workspaces(this->screen);
    GList *node = workspaceGlist;
    while (node) {
        workspaceQList->append((WnckWorkspace *)node->data);
        node = node->next;
    }
}

void UkwsWnckOperator::updateWindowList(/*WnckScreen *screen, WnckWorkspace *workspace*/)
{
    // windowQList所指向的实例都是来自于wnck的screen，无需释放
    bool needShow = false;
    windowQList->clear();

    checkAndInitWorkspaceAndScreen();

    wnck_screen_force_update(this->screen);
    // The list should not be modified nor freed, as it is owned by screen.
    // 来自wnck的官方解释，GList来自wnck的screen，无需释放
    GList *windowGlist = wnck_screen_get_windows_stacked(this->screen);

    GList *node = windowGlist;
    while (node) {
        WnckWindow *win = static_cast<WnckWindow *>(node->data);
        needShow = true;
        if (needCheckWorkspace) {
            if (wnck_window_get_workspace(win) != this->workspace)
                needShow = false;
        }

        if (needCheckScreen) {
            if (wnck_window_get_screen(win) != this->screen)
                needShow = false;
        }

        // 过滤掉其他类型的窗口，如桌面、panel、dock等
        if ((wnck_window_get_window_type(win) == WNCK_WINDOW_NORMAL) && needShow)
            windowQList->insert(-1, WNCK_WINDOW(node->data));

        node = node->next;
    }
}
