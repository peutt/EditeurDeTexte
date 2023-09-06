#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QList>
#include<QSettings>
#include<QTextEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void slotOuvrir();
    void fermerOnglet(int index);
    void textChange();
    void sauvegarderTout();
    void sauvegarderUn();
    void afficherPositionCurseur();
    void rechercherTexte();
    void remplacerTout();

    void ouvrirDernierFichier(QAction *action);
private:
    QSettings settings;
    Ui::MainWindow *ui;
    QMap<QTextEdit*, QString> contenuInitialMap;
    void connectActions();
    void initializeSettings();
    void displayRecentFiles();
    void ouvrir(QString file_name);
    void sauvegarder(int index);
};
#endif // MAINWINDOW_H
