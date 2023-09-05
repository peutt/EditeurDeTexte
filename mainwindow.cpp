#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include<QMessageBox>
#include<QTextStream>
#include <QTabWidget>
#include<QDebug>
#include <QInputDialog>
#include <QTextDocument>
#include<QKeyEvent>
#include<QPoint>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->clear();
    connect (ui->actionSauvegarder_tout,SIGNAL(triggered()),this,SLOT(sauvegarderTout()));
    connect (ui->actionSauvegarder,SIGNAL(triggered()),this,SLOT(sauvegarderUn()));
    connect (ui->actionOuvrir,SIGNAL(triggered()),this,SLOT(ouvrir()));
    connect (ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(fermerOnglet(int)));
    connect (ui->pushButtonRecherche,SIGNAL(clicked()),this,SLOT(rechercherTexte()));
}

void MainWindow::sauvegarderUn(){
    sauvegarder(ui->tabWidget->currentIndex());
}

void MainWindow::sauvegarderTout(){
    QList<int> tabIndices;  // Crée une liste pour stocker les indices des onglets modifiés

    // Parcours tous les onglets pour trouver ceux qui ont été modifiés
    for (int index = 0; index < ui->tabWidget->count(); ++index)
    {
        QString tabText = ui->tabWidget->tabText(index);
        if (tabText.endsWith("*"))
        {
            tabIndices.append(index);  // Ajoute l'indice de l'onglet modifié à la liste
        }
    }

    // Utilise la liste des indices pour sauvegarder les onglets modifiés
    for (int index : tabIndices)
    {
       sauvegarder(index);
    }
}

void MainWindow::textChange(){
    int index = ui->tabWidget->currentIndex();
    QString tabText = ui->tabWidget->tabText(index);

    // Ajoute un astérisque au nom de l'onglet pour indiquer qu'il est modifié
    if (!tabText.endsWith("*")) {
        ui->tabWidget->setTabText(index, tabText + "*");
    }
}

void MainWindow::ouvrir(){
    QString file_name = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");

    // Vérifie si un onglet correspondant au fichier est déjà ouvert
    for (int index = 0; index < ui->tabWidget->count(); ++index)
    {
        QString tabText = ui->tabWidget->tabText(index);
        if (tabText == file_name || tabText==file_name+"*")
        {
            // L'onglet est déjà ouvert, on ne fais rien
            return;
        }
    }
    QFile file(file_name);
    if(!file.open(QFile::ReadOnly)){
        QMessageBox::warning(this,"title","fichier non ouvert");
    }    
    auto textEdit=new QTextEdit();
    ui->tabWidget->addTab(textEdit, QString(file_name).arg(ui->tabWidget->count()+1));
    QTextStream in (&file);
    QString text = in.readAll();
    textEdit->setText(text);

    connect(textEdit, SIGNAL(textChanged()), this, SLOT(textChange()));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(afficherPositionCurseur()));

    file.close();

}

void MainWindow::fermerOnglet(int index){
    if(ui->tabWidget->tabText(index).endsWith("*")){
        // Affiche une boîte de dialogue pour demander à l'utilisateur de sauvegarder ou d'abandonner les modifications.
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmation de sauvegarde", "Le fichier '" + ui->tabWidget->tabText(index) + "' a été modifié. Voulez-vous enregistrer les modifications avant de fermer ?", QMessageBox::Save | QMessageBox::Discard);
        if (reply == QMessageBox::Save)
        {
            // L'utilisateur souhaite sauvegarder, appelez la méthode sauvegarder.
            sauvegarder(index);
        }
    }
    ui->tabWidget->removeTab(index);
}

void MainWindow::sauvegarder(int index){
    // Renomme l'onglet en supprimant le caractère '*'
    QString tabText = ui->tabWidget->tabText(index);
    if (tabText.endsWith("*"))
    {
        tabText.chop(1); // Supprime le dernier caractère '*'
        ui->tabWidget->setTabText(index, tabText);
    }

    QFile file(ui->tabWidget->tabText(index));
    if(!file.open(QFile::WriteOnly)){
        QMessageBox::warning(this,ui->tabWidget->tabText(index),"fichier non sauvgarder");
    }
    QTextStream stream(&file);
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->widget(index));

    stream<<textEdit->toPlainText();
    file.close();
}

void MainWindow::afficherPositionCurseur()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        int lineNumber = cursor.blockNumber() + 1; // Ajoute 1 car les numéros de ligne commencent généralement à 1
        int columnNumber = cursor.columnNumber() + 1; // Ajoute 1 car les numéros de colonne commencent généralement à 1

        statusBar()->showMessage(tr("Ligne %1, Colonne %2").arg(lineNumber).arg(columnNumber));
    }
    else
    {
        statusBar()->clearMessage();
    }
}

void MainWindow::rechercherTexte()
{
    //On réinitialise la recherche
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        cursor.select(QTextCursor::Document);
        QTextCharFormat format;
        format.setBackground(Qt::white); // Changez la couleur d'arrière-plan au blanc ou à la couleur par défaut
        cursor.mergeCharFormat(format);
    }

    // Obtenez le texte à rechercher à partir d'une boîte de dialogue ou d'un champ de recherche
    QString texteRecherche = ui->lineEdit->text();

    if (texteRecherche.isEmpty())
        return;


    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        QTextDocument *document = textEdit->document();

        QTextCursor rechercheCursor(document);

        // Configurez les options de recherche en fonction des besoins
        QTextDocument::FindFlags options = 0;

        if (ui->checkBox->isChecked()) // Si l'option de sensibilité à la casse est cochée
        {
            options |= QTextDocument::FindCaseSensitively;
        }

        // Parcourez le texte pour trouver toutes les occurrences
        while (!rechercheCursor.isNull() && !rechercheCursor.atEnd())
        {
            rechercheCursor = document->find(texteRecherche, rechercheCursor, options);

            if (!rechercheCursor.isNull())
            {
                // Traitez la correspondance trouvée ici, par exemple, mettez en surbrillance le texte
                QTextCharFormat format;
                format.setBackground(QColor(Qt::yellow));
                rechercheCursor.mergeCharFormat(format);
            }
        }
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

