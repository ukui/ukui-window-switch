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

#include "qhotkey.h"
#include "qhotkey_p.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QMetaMethod>
#include <QThread>
#include <QDebug>

Q_LOGGING_CATEGORY(logQHotkey, "QHotkey")

QHotkey::QHotkey(QObject *parent) :
    QObject(parent),
    _keyCode(Qt::Key_unknown),
    _modifiers(Qt::NoModifier),
    _nativeShortcut(),
    _registered(false)
{}

QHotkey::QHotkey(const QKeySequence &sequence, bool autoRegister, QObject *parent) :
    QHotkey(parent)
{
    setShortcut(sequence, autoRegister);
}

QHotkey::QHotkey(Qt::Key keyCode, Qt::KeyboardModifiers modifiers, bool autoRegister, QObject *parent) :
    QHotkey(parent)
{
    setShortcut(keyCode, modifiers, autoRegister);
}

QHotkey::QHotkey(const QHotkey::NativeShortcut &shortcut, bool autoRegister, QObject *parent) :
    QHotkey(parent)
{
    setNativeShortcut(shortcut, autoRegister);
}

QHotkey::~QHotkey()
{
    if(_registered)
        QHotkeyPrivate::instance()->removeShortcut(this);
}

QKeySequence QHotkey::shortcut() const
{
    if(_keyCode == Qt::Key_unknown)
        return QKeySequence();
    else
        return QKeySequence(_keyCode | _modifiers);
}

Qt::Key QHotkey::keyCode() const
{
    return _keyCode;
}

Qt::KeyboardModifiers QHotkey::modifiers() const
{
    return _modifiers;
}

QHotkey::NativeShortcut QHotkey::currentNativeShortcut() const
{
    return _nativeShortcut;
}

bool QHotkey::isRegistered() const
{
    return _registered;
}

bool QHotkey::setShortcut(const QKeySequence &shortcut, bool autoRegister)
{
    if(shortcut.isEmpty()) {
        return resetShortcut();
    } else if(shortcut.count() > 1) {
        qCWarning(logQHotkey, "Keysequences with multiple shortcuts are not allowed! "
                              "Only the first shortcut will be used!");
    }

    return setShortcut(Qt::Key(shortcut[0] & ~Qt::KeyboardModifierMask),
            Qt::KeyboardModifiers(shortcut[0] & Qt::KeyboardModifierMask),
            autoRegister);
}

bool QHotkey::setShortcut(Qt::Key keyCode, Qt::KeyboardModifiers modifiers, bool autoRegister)
{
    if(_registered) {
        if(autoRegister) {
            if(!QHotkeyPrivate::instance()->removeShortcut(this))
                return false;
        } else
            return false;
    }

    if(keyCode == Qt::Key_unknown) {
        _keyCode = Qt::Key_unknown;
        _modifiers = Qt::NoModifier;
        _nativeShortcut = NativeShortcut();
        return true;
    }

    _keyCode = keyCode;
    _modifiers = modifiers;
    _nativeShortcut = QHotkeyPrivate::instance()->nativeShortcut(keyCode, modifiers);
    if(_nativeShortcut.isValid()) {
        if(autoRegister)
            return QHotkeyPrivate::instance()->addShortcut(this);
        else
            return true;
    } else {
        qCWarning(logQHotkey) << "Unable to map shortcut to native keys. Key:" << keyCode << "Modifiers:" << modifiers;
        _keyCode = Qt::Key_unknown;
        _modifiers = Qt::NoModifier;
        _nativeShortcut = NativeShortcut();
        return false;
    }
}

bool QHotkey::resetShortcut()
{
    if(_registered &&
       !QHotkeyPrivate::instance()->removeShortcut(this)) {
        return false;
    }

    _keyCode = Qt::Key_unknown;
    _modifiers = Qt::NoModifier;
    _nativeShortcut = NativeShortcut();
    return true;
}

bool QHotkey::setNativeShortcut(QHotkey::NativeShortcut nativeShortcut, bool autoRegister)
{
    if(_registered) {
        if(autoRegister) {
            if(!QHotkeyPrivate::instance()->removeShortcut(this))
                return false;
        } else
            return false;
    }

    if(nativeShortcut.isValid()) {
        _keyCode = Qt::Key_unknown;
        _modifiers = Qt::NoModifier;
        _nativeShortcut = nativeShortcut;
        if(autoRegister)
            return QHotkeyPrivate::instance()->addShortcut(this);
        else
            return true;
    } else {
        _keyCode = Qt::Key_unknown;
        _modifiers = Qt::NoModifier;
        _nativeShortcut = NativeShortcut();
        return true;
    }
}

bool QHotkey::setRegistered(bool registered)
{
    if(_registered && !registered)
        return QHotkeyPrivate::instance()->removeShortcut(this);
    else if(!_registered && registered) {
        if(!_nativeShortcut.isValid())
            return false;
        else
            return QHotkeyPrivate::instance()->addShortcut(this);
    } else
        return true;
}



// ---------- QHotkeyPrivate implementation ----------

QHotkeyPrivate::QHotkeyPrivate() :
    shortcuts()
{
    Q_ASSERT_X(qApp, Q_FUNC_INFO, "QHotkey requires QCoreApplication to be instantiated");
    qApp->eventDispatcher()->installNativeEventFilter(this);
}

QHotkeyPrivate::~QHotkeyPrivate()
{
    if(!shortcuts.isEmpty())
        qCWarning(logQHotkey) << "QHotkeyPrivate destroyed with registered shortcuts!";
    if(qApp && qApp->eventDispatcher())
        qApp->eventDispatcher()->removeNativeEventFilter(this);
}

QHotkey::NativeShortcut QHotkeyPrivate::nativeShortcut(Qt::Key keycode, Qt::KeyboardModifiers modifiers)
{
    Qt::ConnectionType conType = (QThread::currentThread() == thread() ?
                                      Qt::DirectConnection :
                                      Qt::BlockingQueuedConnection);
    QHotkey::NativeShortcut res;
    if(!QMetaObject::invokeMethod(this, "nativeShortcutInvoked", conType,
                                  Q_RETURN_ARG(QHotkey::NativeShortcut, res),
                                  Q_ARG(Qt::Key, keycode),
                                  Q_ARG(Qt::KeyboardModifiers, modifiers))) {
        return QHotkey::NativeShortcut();
    } else
        return res;
}

bool QHotkeyPrivate::addShortcut(QHotkey *hotkey)
{
    if(hotkey->_registered)
        return false;

    Qt::ConnectionType conType = (QThread::currentThread() == thread() ?
                                      Qt::DirectConnection :
                                      Qt::BlockingQueuedConnection);
    bool res = false;
    if(!QMetaObject::invokeMethod(this, "addShortcutInvoked", conType,
                                  Q_RETURN_ARG(bool, res),
                                  Q_ARG(QHotkey*, hotkey))) {
        return false;
    } else {
        if(res)
            emit hotkey->registeredChanged(true);
        return res;
    }
}

bool QHotkeyPrivate::removeShortcut(QHotkey *hotkey)
{
    if(!hotkey->_registered)
        return false;

    Qt::ConnectionType conType = (QThread::currentThread() == thread() ?
                                      Qt::DirectConnection :
                                      Qt::BlockingQueuedConnection);
    bool res = false;
    if(!QMetaObject::invokeMethod(this, "removeShortcutInvoked", conType,
                                  Q_RETURN_ARG(bool, res),
                                  Q_ARG(QHotkey*, hotkey))) {
        return false;
    } else {
        if(res)
            emit hotkey->registeredChanged(false);
        return res;
    }
}

void QHotkeyPrivate::activateShortcut(QHotkey::NativeShortcut shortcut)
{
    QMetaMethod signal = QMetaMethod::fromSignal(&QHotkey::activated);
    for(QHotkey *hkey : shortcuts.values(shortcut))
        signal.invoke(hkey, Qt::QueuedConnection);
}

bool QHotkeyPrivate::addShortcutInvoked(QHotkey *hotkey)
{
    QHotkey::NativeShortcut shortcut = hotkey->_nativeShortcut;

    if(!shortcuts.contains(shortcut)) {
        if(!registerShortcut(shortcut))
            return false;
    }

    shortcuts.insert(shortcut, hotkey);
    hotkey->_registered = true;
    return true;
}

bool QHotkeyPrivate::removeShortcutInvoked(QHotkey *hotkey)
{
    QHotkey::NativeShortcut shortcut = hotkey->_nativeShortcut;

    if(shortcuts.remove(shortcut, hotkey) == 0)
        return false;
    hotkey->_registered = false;
    emit hotkey->registeredChanged(true);
    if(shortcuts.count(shortcut) == 0)
        return unregisterShortcut(shortcut);
    else
        return true;
}

QHotkey::NativeShortcut QHotkeyPrivate::nativeShortcutInvoked(Qt::Key keycode, Qt::KeyboardModifiers modifiers)
{
    bool ok1, ok2 = false;
    auto k = nativeKeycode(keycode, ok1);
    auto m = nativeModifiers(modifiers, ok2);
    if(ok1 && ok2)
        return {k, m};
    else
        return {};
}



QHotkey::NativeShortcut::NativeShortcut() :
    key(),
    modifier(),
    valid(false)
{}

QHotkey::NativeShortcut::NativeShortcut(quint32 key, quint32 modifier) :
    key(key),
    modifier(modifier),
    valid(true)
{}

bool QHotkey::NativeShortcut::isValid() const
{
    return valid;
}

bool QHotkey::NativeShortcut::operator ==(const QHotkey::NativeShortcut &other) const
{
    return (key == other.key) &&
           (modifier == other.modifier) &&
           valid == other.valid;
}

bool QHotkey::NativeShortcut::operator !=(const QHotkey::NativeShortcut &other) const
{
    return (key != other.key) ||
           (modifier != other.modifier) ||
           valid != other.valid;
}

uint qHash(const QHotkey::NativeShortcut &key)
{
    return qHash(key.key) ^ qHash(key.modifier);
}

uint qHash(const QHotkey::NativeShortcut &key, uint seed)
{
    return qHash(key.key, seed) ^ qHash(key.modifier, seed);
}
