#ifndef UKWS_COMMON_H
#define UKWS_COMMON_H

#define UKWS_OBJ_IND_SUBWIDGET      "indicator_sub_widget"
#define UKWS_OBJ_IND_MAINWIDGET     "indicator_main_widget"
#define UKWS_OBJ_WS_MGR_SUBWIDGET   "ws_manager_sub_widget"
#define UKWS_OBJ_WS_MGR_MAINWIDGET  "ws_manager_main_widget"
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
