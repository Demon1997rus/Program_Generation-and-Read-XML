#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QtXml/QtXml>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //функция для создания отдельного элемента и присвоения номера отдельному человеку в xml документе
    QDomElement makeElement(QDomDocument& domDoc,
                            const QString& strName,
                            const QString& strAttr = QString(),
                            const QString& strText = QString()
                            );

    //функция для создания элементов в XML c помощью контактной информации из таблицы
    QDomElement contact(QDomDocument& domDoc,
                        const QString& strName,
                        const QString& strAge,
                        const QString& strExperience
                        );

    void traverseNode(const QDomNode& node);


private slots:
    void on_pbDisplayBDtable_clicked();
    void on_pbGenerationXML_clicked();
    void on_pbReadXml_clicked();
    void on_pbAddRow_clicked();
    void slotUnlock_buttons();
    void on_pbRemoveRow_clicked();
    void on_tableView_Display_BD_or_XML_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QFile *file;
    QSqlDatabase db;
    QSqlQuery *query;
    QSqlTableModel *model_BD;
    QStandardItemModel *model_XML = nullptr;
    QStringList lst;
    QDomDocument domDoc;//чтение XML

    int nNumber = 1;
    int nRow;
};
#endif // MAINWINDOW_H
