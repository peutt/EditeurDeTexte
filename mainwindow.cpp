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
    connect (ui->actionOuvrir,SIGNAL(triggered()),this,SLOT(slotOuvrir()));
    connect (ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(fermerOnglet(int)));
    connect (ui->pushButtonRecherche,SIGNAL(clicked()),this,SLOT(rechercherTexte()));
    connect (ui->pushButtonRemplaceTout,SIGNAL(clicked()),this,SLOT(remplacerTout()));
    connect (ui->action10_derniers_fichiers,SIGNAL(triggered()),this,SLOT(afficherDerniersFichiersOuverts()));

    // Initialise l'objet QSettings
    settings.beginGroup("MonEditeurDeTexte"); // Utilise un groupe pour éviter les collisions de clés
    settings.setValue("fichiersRecents", QStringList()); // Initialise la liste des fichiers récemment ouverts
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

    QTextEdit *textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        // Compare le contenu actuel avec le contenu initial de cet onglet
        QString contenuInitial = contenuInitialMap.value(textEdit);

        if (textEdit->toPlainText() == contenuInitial)
        {
            // Le contenu est identique, on retire l'astérisque "*"
            if (tabText.endsWith("*"))
            {
                ui->tabWidget->setTabText(index, tabText.left(tabText.length() - 1));
            }
        }
        else
        {
            // Le contenu a été modifié, on ajoute l'astérisque "*"
            if (!tabText.endsWith("*"))
            {
                ui->tabWidget->setTabText(index, tabText + "*");
            }
        }
    }
}
void MainWindow::ouvrir(QString file_name){
    // Ajoute le chemin complet du fichier ouvert à la liste des fichiers récemment ouverts
    QStringList fichiersRecents = settings.value("fichiersRecents").toStringList();

    if(!fichiersRecents.contains(file_name))//Verifie l'unicité du fichier ouvert dans la liste des fichiers reçament ouvert
    {
        fichiersRecents.prepend(file_name);
    }


    // Limite la liste à un maximum de dix éléments
    while (fichiersRecents.size() > 10)
    {
        fichiersRecents.removeLast();
    }

    // Enregistre la liste mise à jour dans les paramètres de l'application
    settings.setValue("fichiersRecents", fichiersRecents);



    // Vérifie si un onglet correspondant au fichier est déjà ouvert
    for (int index = 0; index < ui->tabWidget->count(); ++index)
    {
        QString tabToolTip = ui->tabWidget->tabToolTip(index);
        if (tabToolTip == file_name)
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

    QFileInfo fileInfo(file_name);

    QString nomFichier = fileInfo.fileName(); // Obtient le nom du fichier
    int index = ui->tabWidget->addTab(textEdit, QString(nomFichier).arg(ui->tabWidget->count()+1));
    ui->tabWidget->setTabToolTip(index, file_name);
    QTextStream in (&file);
    QString text = in.readAll();
    textEdit->setText(text);
    contenuInitialMap[textEdit] = text;
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(textChange()));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(afficherPositionCurseur()));

    file.close();
}
void MainWindow::slotOuvrir(){
    QString file_name = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");
    if (file_name.isEmpty())
        return;
    ouvrir(file_name);
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

    QFile file(ui->tabWidget->tabToolTip(index));
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
        format.setBackground(Qt::white); // Change la couleur d'arrière-plan au blanc
        cursor.mergeCharFormat(format);
    }

    // Obtient le texte à rechercher à partir d'un champ de recherche
    QString texteRecherche = ui->lineEdit->text();

    if (texteRecherche.isEmpty())
        return;


    if (textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        QTextDocument *document = textEdit->document();

        QTextCursor rechercheCursor(document);

        // Parcours le texte pour trouver toutes les occurrences
        while (!rechercheCursor.isNull() && !rechercheCursor.atEnd())
        {
            if (ui->checkBox->isChecked()) // Si l'option de sensibilité à la casse est cochée
            {
                rechercheCursor = document->find(texteRecherche, rechercheCursor,QTextDocument::FindCaseSensitively);
            } else {
                rechercheCursor = document->find(texteRecherche, rechercheCursor);
            }


            if (!rechercheCursor.isNull())
            {
                // Traite la correspondance trouvée ici, met en surbrillance le texte
                QTextCharFormat format;
                format.setBackground(QColor(Qt::yellow));
                rechercheCursor.mergeCharFormat(format);
            }
        }
    }
}

void MainWindow::remplacerTout()
{
    // Obtiens le texte à rechercher et le texte de remplacement à partir des champs de saisie
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



        // Parcours le texte pour trouver toutes les occurrences
        while (!rechercheCursor.isNull() && !rechercheCursor.atEnd())
        {
            if (ui->checkBox->isChecked()) // Si l'option de sensibilité à la casse est cochée
            {
                rechercheCursor = document->find(texteRecherche, rechercheCursor, QTextDocument::FindCaseSensitively);
            }

            rechercheCursor = document->find(texteRecherche, rechercheCursor);

            if (!rechercheCursor.isNull())
            {
                // Remplace le texte trouvé par le texte de remplacement
                rechercheCursor.insertText(texteRemplacement);
            }
        }

    }
}

void MainWindow::afficherDerniersFichiersOuverts()
{
    // Lit la liste des fichiers récemment ouverts à partir des paramètres de l'application
    QStringList fichiersRecents = settings.value("fichiersRecents").toStringList();

    // Crée un message contenant la liste des fichiers récemment ouverts
    QString message = "Derniers fichiers ouverts :\n";
    int numFilesToShow = qMin(10, fichiersRecents.size()); // Afficher au maximum 10 fichiers
    for (int i = 0; i < numFilesToShow; ++i)
    {
        message += QString("%1. %2\n").arg(i + 1).arg(fichiersRecents[i]);
    }

    // Affichez le message dans un QMessageBox

    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, "Derniers fichiers ouverts", message, QMessageBox::Open | QMessageBox::Close);
    if (reply == QMessageBox::Open)
    {
        for(auto fichier:fichiersRecents){
            ouvrir(fichier);
        }
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

