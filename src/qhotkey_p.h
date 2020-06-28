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

#ifndef QHOTKEY_P_H
#define QHOTKEY_P_H

#include "qhotkey.h"
#include <QAbstractNativeEventFilter>
#include <QMultiHash>
#include <QMutex>
#include <QGlobalStatic>

class QHOTKEY_SHARED_EXPORT QHotkeyPrivate : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    QHotkeyPrivate();//singleton!!!
    ~QHotkeyPrivate();

    static QHotkeyPrivate *instance();

    QHotkey::NativeShortcut nativeShortcut(Qt::Key keycode, Qt::KeyboardModifiers modifiers);

    bool addShortcut(QHotkey *hotkey);
    bool removeShortcut(QHotkey *hotkey);

protected:
    void activateShortcut(QHotkey::NativeShortcut shortcut);

    virtual quint32 nativeKeycode(Qt::Key keycode, bool &ok) = 0;//platform implement
    virtual quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool &ok) = 0;//platform implement

    virtual bool registerShortcut(QHotkey::NativeShortcut shortcut) = 0;//platform implement
    virtual bool unregisterShortcut(QHotkey::NativeShortcut shortcut) = 0;//platform implement

private:
    QMultiHash<QHotkey::NativeShortcut, QHotkey*> shortcuts;

    Q_INVOKABLE bool addShortcutInvoked(QHotkey *hotkey);
    Q_INVOKABLE bool removeShortcutInvoked(QHotkey *hotkey);
    Q_INVOKABLE QHotkey::NativeShortcut nativeShortcutInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers);
};

#define NATIVE_INSTANCE(ClassName) \
    Q_GLOBAL_STATIC(ClassName, hotkeyPrivate) \
    \
    QHotkeyPrivate *QHotkeyPrivate::instance()\
    {\
        return hotkeyPrivate;\
    }

#endif // QHOTKEY_P_H
