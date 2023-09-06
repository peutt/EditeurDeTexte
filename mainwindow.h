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
    void afficherDerniersFichiersOuverts();
private:
    QSettings settings;
    Ui::MainWindow *ui;
    QMap<QTextEdit*, QString> contenuInitialMap;
    void sauvegarder(int index);

    void ouvrir(QString);
};
#endif // MAINWINDOW_H
