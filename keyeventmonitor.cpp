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

#include "keyeventmonitor.h"
#include <X11/Xlibint.h>
#include <stdio.h>
#include <stdlib.h>

KeyEventMonitor::KeyEventMonitor(QObject *parent) : QThread(parent)
{
	//Whether the mouse is pressed, used to identify the drag and drop operation of the mouse
	isRelease = false;
}

void KeyEventMonitor::run()
{
	// Open a connection to the X server that controls a display.
	Display *display = XOpenDisplay(0);

	if (display == 0)
	{
		fprintf(stderr, "unable to open display\n");
		return;
	}

	// Initialize the parameters for the function of XRecordCreateContext.
	// XRecordAllClients means recording all events of X Client.
	XRecordClientSpec clients = XRecordAllClients;

	// Control the range of recording events.
	XRecordRange *range = XRecordAllocRange();
	if (range == 0)
	{
		fprintf(stderr, "unable to allocate XRecordRange\n");
		return;
	}

	// Initialize range.
	// Only five types of events: KeyPress, KeyRelase, ButtonPress, ButtonRelease, MotionNotify.
	memset(range, 0, sizeof(XRecordRange));
	range->device_events.first = KeyPress;
	range->device_events.last = MotionNotify;

	// Create the record context by clients and range.
	// The variable is as the paratemer of XRecordEnableContext for event loop.
	XRecordContext context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
	if (context == 0)
	{
		fprintf(stderr, "XRecordCreateContext failed\n");
		return;
	}
	XFree(range);

	// Flush the output buffer and then waits until all requests have been received and processed by the X server.
	XSync(display, True);

	// Returns a Display structure that serves as the connection to the X server and that contains all the information about that X server.
	Display *display_datalink = XOpenDisplay(0);
	if (display_datalink == 0)
	{
		fprintf(stderr, "unable to open second display\n");
		return;
	}

	// Call XRecordEnableContext to build the XRecord context and enter the event loop.
	// The callback function will be evoked on receiving an event from X Server.
	if (!XRecordEnableContext(display_datalink, context, callback, (XPointer)this))
	{
		fprintf(stderr, "XRecordEnableContext() failed\n");
		return;
	}
}

// To wrap the handleRecordEvent function to avoid that the code about XRecord can not be compiled.
void KeyEventMonitor::callback(XPointer ptr, XRecordInterceptData *data)
{
	((KeyEventMonitor *)ptr)->handleRecordEvent(data);
}

// The callback function to handle the event from X Server.
void KeyEventMonitor::handleRecordEvent(XRecordInterceptData *data)
{
	if (data->category == XRecordFromServer)
	{
		xEvent *event = (xEvent *)data->data;
		switch (event->u.u.type)
		{
		case ButtonPress:
			//printf("X11 ButtonPress\n");
			break;
		case ButtonRelease:
			//printf("X11 ButtonRelease\n");
			break;
		case KeyPress:
			//printf("X11 KeyPress\n");
			break;
		case KeyRelease:
			//printf("X11 KeyRelease\n");
			this->isReleaseAlt(((unsigned char *)data->data)[1]);
			break;
		default:
			break;
		}
	}

	fflush(stdout);
	XRecordFreeData(data);
}

void KeyEventMonitor::isReleaseAlt(int code)
{
	if (code == 64) {
		emit KeyAltRelease();
	}
}
