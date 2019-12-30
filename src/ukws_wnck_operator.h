#ifndef UKWS_WNCK_OPERATOR_H
#define UKWS_WNCK_OPERATOR_H

extern "C" {
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
}

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
