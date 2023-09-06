#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTabWidget>
#include <QDebug>
#include <QInputDialog>
#include <QTextDocument>
#include <QKeyEvent>
#include <QPoint>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->clear();
    ui->menuFichiers_recents->clear();

    connectActions();
    initializeSettings();
    displayRecentFiles();
}

void MainWindow::connectActions()
{
    connect(ui->actionSauvegarder_tout, &QAction::triggered, this, &MainWindow::sauvegarderTout);
    connect(ui->actionSauvegarder, &QAction::triggered, this, &MainWindow::sauvegarderUn);
    connect(ui->actionOuvrir, &QAction::triggered, this, &MainWindow::slotOuvrir);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::fermerOnglet);
    connect(ui->pushButtonRecherche, &QPushButton::clicked, this, &MainWindow::rechercherTexte);
    connect(ui->pushButtonRemplaceTout, &QPushButton::clicked, this, &MainWindow::remplacerTout);
    connect(ui->menuFichiers_recents, &QMenu::triggered, this, &MainWindow::ouvrirDernierFichier);
}

void MainWindow::initializeSettings()
{
    settings.beginGroup("EditeurDeTexte");
}

void MainWindow::displayRecentFiles()
{
    QStringList fichiersRecents = settings.value("fichiersRecents").toStringList();
    for (const QString &fichier : fichiersRecents)
    {
        QAction *action = new QAction(fichier, this);
        ui->menuFichiers_recents->addAction(action);
    }
}

void MainWindow::ouvrirDernierFichier(QAction* action)
{
    ouvrir(action->text());
}

void MainWindow::sauvegarderUn()
{
    sauvegarder(ui->tabWidget->currentIndex());
}

void MainWindow::sauvegarderTout()
{
    QList<int> tabIndices;

    for (int index = 0; index < ui->tabWidget->count(); ++index)
    {
        QString tabText = ui->tabWidget->tabText(index);
        if (tabText.endsWith("*"))
        {
            tabIndices.append(index);
        }
    }

    for (int index : tabIndices)
    {
        sauvegarder(index);
    }
}

void MainWindow::textChange()
{
    int index = ui->tabWidget->currentIndex();
    QString tabText = ui->tabWidget->tabText(index);

    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        QString contenuInitial = contenuInitialMap.value(textEdit);

        if (textEdit->toPlainText() == contenuInitial)
        {
            if (tabText.endsWith("*"))
            {
                ui->tabWidget->setTabText(index, tabText.left(tabText.length() - 1));
            }
        }
        else
        {
            if (!tabText.endsWith("*"))
            {
                ui->tabWidget->setTabText(index, tabText + "*");
            }
        }
    }
}

void MainWindow::ouvrir(QString file_name)
{
    QStringList fichiersRecents = settings.value("fichiersRecents").toStringList();

    if (!fichiersRecents.contains(file_name))
    {
        fichiersRecents.prepend(file_name);
    }

    while (fichiersRecents.size() > 10)
    {
        fichiersRecents.removeLast();
    }

    settings.setValue("fichiersRecents", fichiersRecents);
    ui->menuFichiers_recents->clear();
    displayRecentFiles();

    for (int index = 0; index < ui->tabWidget->count(); ++index)
    {
        QString tabToolTip = ui->tabWidget->tabToolTip(index);
        if (tabToolTip == file_name)
        {
            return;
        }
    }

    QFile file(file_name);
    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, "Erreur d'ouverture", "Impossible d'ouvrir le fichier.");
    }

    auto textEdit = new QTextEdit();

    QFileInfo fileInfo(file_name);

    QString nomFichier = fileInfo.fileName();
    int index = ui->tabWidget->addTab(textEdit, QString(nomFichier).arg(ui->tabWidget->count() + 1));
    ui->tabWidget->setTabToolTip(index, file_name);
    QTextStream in(&file);
    QString text = in.readAll();
    textEdit->setText(text);
    contenuInitialMap[textEdit] = text;

    connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::textChange);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &MainWindow::afficherPositionCurseur);

    file.close();
}

void MainWindow::slotOuvrir()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");
    if (file_name.isEmpty())
        return;
    ouvrir(file_name);
}

void MainWindow::fermerOnglet(int index)
{
    if (ui->tabWidget->tabText(index).endsWith("*"))
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmation de sauvegarde", "Le fichier '" + ui->tabWidget->tabText(index) + "' a été modifié. Voulez-vous enregistrer les modifications avant de fermer ?", QMessageBox::Save | QMessageBox::Discard);
        if (reply == QMessageBox::Save)
        {
            sauvegarder(index);
        }
    }
    ui->tabWidget->removeTab(index);
}

void MainWindow::sauvegarder(int index)
{
    QString tabText = ui->tabWidget->tabText(index);
    if (tabText.endsWith("*"))
    {
        tabText.chop(1);
        ui->tabWidget->setTabText(index, tabText);
    }

    QFile file(ui->tabWidget->tabToolTip(index));
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, ui->tabWidget->tabText(index), "Erreur lors de la sauvegarde.");
    }

    QTextStream stream(&file);

    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->widget(index));
    if (textEdit)
    {
        contenuInitialMap[textEdit] = textEdit->toPlainText();
        stream << textEdit->toPlainText();
    }
    file.close();
}

void MainWindow::afficherPositionCurseur()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        int lineNumber = cursor.blockNumber() + 1;
        int columnNumber = cursor.columnNumber() + 1;

        statusBar()->showMessage(tr("Ligne %1, Colonne %2").arg(lineNumber).arg(columnNumber));
    }
    else
    {
        statusBar()->clearMessage();
    }
}

void MainWindow::rechercherTexte()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        cursor.select(QTextCursor::Document);
        QTextCharFormat format;
        format.setBackground(Qt::white);
        cursor.mergeCharFormat(format);
    }

    QString texteRecherche = ui->lineEdit->text();

    if (texteRecherche.isEmpty())
        return;

    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        QTextDocument *document = textEdit->document();

        QTextCursor rechercheCursor(document);

        while (!rechercheCursor.isNull() && !rechercheCursor.atEnd())
        {
            if (ui->checkBox->isChecked())
            {
                rechercheCursor = document->find(texteRecherche, rechercheCursor, QTextDocument::FindCaseSensitively);
            }
            else
            {
                rechercheCursor = document->find(texteRecherche, rechercheCursor);
            }

            if (!rechercheCursor.isNull())
            {
                QTextCharFormat format;
                format.setBackground(QColor(Qt::yellow));
                rechercheCursor.mergeCharFormat(format);
            }
        }
    }
}

void MainWindow::remplacerTout()
{
    QString texteRecherche = ui->lineEdit->text();
    QString texteRemplacement = ui->lineEdit_2->text();
    if (texteRecherche.isEmpty() || texteRemplacement.isEmpty())
        return;

    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());

    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        QTextDocument *document = textEdit->document();

        QTextCursor rechercheCursor(document);

        while (!rechercheCursor.isNull() && !rechercheCursor.atEnd())
        {
            if (ui->checkBox->isChecked())
            {
                rechercheCursor = document->find(texteRecherche, rechercheCursor, QTextDocument::FindCaseSensitively);
            }
            else
            {
                rechercheCursor = document->find(texteRecherche, rechercheCursor);
            }

            if (!rechercheCursor.isNull())
            {
                rechercheCursor.insertText(texteRemplacement);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    settings.endGroup();
}
