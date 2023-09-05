#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void ouvrir();
    void fermerOnglet(int index);
    void textChange();
    void sauvegarderTout();
    void sauvegarderUn();
    void afficherPositionCurseur();
    void rechercherTexte();
private:
    Ui::MainWindow *ui;
    void sauvegarder(int index);

};
#endif // MAINWINDOW_H
