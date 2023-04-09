#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QStandardItem"
//------------------------------------------------------------------------------------------------
//Конструктор MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Тестовое задание (Ротанов Д.И.)");
    //создаём объект дб и саму базу данных
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./testDB.db");

    //проверяем открылся бд или нет, если да создаём саму таблицу базы данных
    if(db.open())
    {
        qDebug("Open");
        query = new QSqlQuery(db);
        query->exec("CREATE TABLE Multibook(FullName TEXT, Age INT, Experience INT);");
        model_BD = new QSqlTableModel(this,db);
        model_BD->setTable("Multibook");
        model_BD->select();
        model_BD->setEditStrategy(QSqlTableModel::OnManualSubmit);
    }
    else
    {
        qDebug("no Open");
    }

    //настройки таблицы представления моделей model_BD и model_XML
    ui->tableView_Display_BD_or_XML->resizeColumnsToContents();
    ui->tableView_Display_BD_or_XML->horizontalHeader()->setStretchLastSection(true);

    //сигнал об изменении данных в таблице БД со слотом, который делает активной кнопку pbGenerationXML
    QObject::connect(model_BD, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                     this, SLOT(slotUnlock_buttons()));

    //сигнал об изменении данных в таблице БД со слотом, который применяет все сделанные изменения в БД
    QObject::connect(model_BD, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                     model_BD, SLOT(submitAll()));

    //По сигналу клика на кнопку "Выход", задача закрывается
    QObject::connect(ui->pbExit,SIGNAL(clicked(bool))
                     ,this , SLOT(close()));

    //предварительные настройки кнопок
    if(QFile::exists("Multibook.xml"))
    {
        ui->pbReadXml->setEnabled(true);
    }
    else
    {
        ui->pbReadXml->setDisabled(true);
    }
    ui->pbGenerationXML->setDisabled(true);
    ui->pbAddRow->setDisabled(true);
    ui->pbRemoveRow->setDisabled(true);
}

//деструктор MainWindow
MainWindow::~MainWindow()
{
    delete query;
    delete model_BD;
    delete model_XML;
    delete ui;
}

//||--------------------------------Раздел кода с функциями------------------------------------||
// Функция для создания отдельного элемента и присвоения номера отдельному человеку в xml документе
QDomElement MainWindow::makeElement(QDomDocument &domDoc, const QString &strName, const QString &strAttr, const QString &strText)
{
    QDomElement domElement = domDoc.createElement(strName);

    if (!strAttr.isEmpty())
    {
        QDomAttr domAttr = domDoc.createAttribute("number");
        domAttr.setValue(strAttr);
        domElement.setAttributeNode(domAttr);
    }

    if(!strText.isEmpty())
    {
        QDomText domText = domDoc.createTextNode(strText);
        domElement.appendChild(domText);
    }

    return domElement;
}

//функция для создания элементов в XML c помощью контактной информации из таблицы БД
QDomElement MainWindow::contact(QDomDocument &domDoc, const QString &strName, const QString &strAge, const QString &strExperience)
{
    QDomElement domElement = makeElement(domDoc, "Programist", QString().setNum(nNumber));
    domElement.appendChild(makeElement(domDoc, "FullName", "", strName));
    domElement.appendChild(makeElement(domDoc, "Age", "", strAge));
    domElement.appendChild(makeElement(domDoc, "Experience", "", strExperience));
    nNumber++;
    return domElement;
}

//Рекурсивная функция для чтения XML документа
void MainWindow::traverseNode(const QDomNode &node)
{
    static int FullName = 0;
    static int Age = 0;
    static int Exp = 0;

    //Возвращает первый дочерний узел. Если дочерний узел отсутствует, возвращается нулевой узел.
    QDomNode domNode = node.firstChild();
    while(!domNode.isNull())// domNode не нулевой узел?
    {
                int row = model_XML->rowCount();//присваивание переменной row текущего значения метода rowCount()
        if(domNode.isElement())// проверка является ли элементом, если да то преобразуем его к типу QDomElement
        {
            QDomElement domElement = domNode.toElement();
            if(!domElement.isNull())//если элемент не является нулевым
            {
                //далее идёт сравнение по тегам, все заносится в таблицу model_xml
                if(domElement.tagName() == "FullName")
                {
                    // объекту QString присваивается текст между тегов с помощью метода text()
                    QString text = domElement.text();
                    //заносим в таблицу model_XML прочитанный текст
                    model_XML->setItem(row, 0, new QStandardItem(text));
                    if(FullName < 1)
                    {
                        //заносим в контейнер QStringlist найденные теги(с помощью статических переменных int мы делаем это всего 1 раз)
                        lst << domElement.tagName();
                    }
                    FullName++;
                }

                if(domElement.tagName() == "Age")
                {
                    QString text = domElement.text();
                    model_XML->setItem(--row, 1, new QStandardItem(text));
                    if(Age < 1)
                    {
                        lst << domElement.tagName();
                    }
                    Age++;
                }

                if(domElement.tagName() == "Experience")
                {
                    QString text = domElement.text();
                    model_XML->setItem(--row, 2, new QStandardItem(text));
                    if(Exp < 1)
                    {
                        lst << domElement.tagName();
                    }
                    Exp++;
                }
            }
        }
        traverseNode(domNode);
        domNode = domNode.nextSibling(); //если возращает нулевое значение значит узлов нет, цикл закончится
    }
}


//||--------------------------------Раздел кода с кнопками------------------------------------||
//Кнопка для отображения таблицы базы данных на табличной форме
void MainWindow::on_pbDisplayBDtable_clicked()
{
    //выбираем модель БД для таблицы
    if(ui->tableView_Display_BD_or_XML->model() != model_BD)
    {
        ui->tableView_Display_BD_or_XML->setModel(model_BD);
    }

    //если модель БД присвоилась таблице
    if(ui->tableView_Display_BD_or_XML->model() == model_BD)
    {
        //В виджет Qlabel передаем текст и параметры
        ui->label_tableView_BD_or_XML->setText("Таблица БД");
        ui->label_tableView_BD_or_XML->setAlignment(Qt::AlignCenter);
        //блокируем кнопку pbDisplayBDtable
        ui->pbDisplayBDtable->setDisabled(true);
        //разрешаем редактировать таблицу БД и выделять ячейки
        ui->tableView_Display_BD_or_XML->setEditTriggers(QAbstractItemView::DoubleClicked);
        ui->tableView_Display_BD_or_XML->setSelectionMode(QAbstractItemView::SingleSelection);
        //Активизируем кнопки(pbAddRow, pbReadXml)
        ui->pbAddRow->setEnabled(true);
        //кнопка pbReadXml активизируется если найдет xml документ
        if(QFile::exists("Multibook.xml"))
        {
            ui->pbReadXml->setEnabled(true);
        }
    }
}

//Кнопка для генераций XML файла соответствующей структуре таблицы и данными из неё
void MainWindow::on_pbGenerationXML_clicked()
{
    ui->pbReadXml->setDisabled(false); // активизирует кнопку pbReadXML

    QDomDocument doc(model_BD->tableName());
    QDomElement domElement = doc.createElement(model_BD->tableName());
    doc.appendChild(domElement);
    //Создаём массив объектов и через циклы добавляем элементы
    QDomElement contact_arr[model_BD->rowCount()];
    for(int i =  0 ; i < model_BD->rowCount();++i)
    {
        contact_arr[i] = contact(doc,(model_BD->index(i,0).data().toString()), (model_BD->index(i,1).data().toString()),
                                 (model_BD->index(i,2).data().toString()));
    }

    for(int i = 0;i < model_BD->rowCount();i++)
    {
        domElement.appendChild(contact_arr[i]);
    }

    file = new QFile("Multibook.xml");
    if(file->open(QIODevice::WriteOnly))
    {
        QTextStream(file) << doc.toString();
        file->close();
    }
    ui->pbGenerationXML->setDisabled(true);
    nNumber = 1;
    delete file;
}

//Кнопка для чтения xml файла и заполнения таблицы с помощью неё
void MainWindow::on_pbReadXml_clicked()
{
    // Удалил старую модель
    if (model_XML != nullptr)
    {
        delete model_XML;
        model_XML = nullptr;
    }

    file = new QFile("Multibook.xml");
    if(QFile::exists("Multibook.xml"))
    {
        if(file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            //модель таблицы для отображения данных из xml
            model_XML = new QStandardItemModel();

            //Объект класса QDomDocument находится в header
            //Для считывания XML документа передаём в метод setContent() объект класс Qfile
            //Если возращает true, значит синтаксический анализатор распознает пространства имен в XML-файле
            if(domDoc.setContent(file))
            {
                //далее получаем объект класса QdomElement вызывая метод documentElement() класса Qdomdocument,
                //который возращает корневой элемент xml-документа
                QDomElement domElement = domDoc.documentElement();
                //полученный объект передаём в функцию
                traverseNode(domElement);
            }
            else
            {
                qDebug() << "Синтаксический анализатор не распознает пространства имен в XML-файле"
                         << "Ошибка в методе setContent()";
            }

            //прочитанные в xml заголовки присваиваем модели model_XML
            model_XML->setHorizontalHeaderLabels(lst);

            //выбираем model_XML для таблицы
            if(ui->tableView_Display_BD_or_XML->model() != model_XML)
            {
                ui->tableView_Display_BD_or_XML->setModel(model_XML);
            }

            //если модель XML присвоилась таблице
            if(ui->tableView_Display_BD_or_XML->model() == model_XML)
            {
                //В виджет Qlabel передаем текст и параметры
                ui->label_tableView_BD_or_XML->setText("Таблица XML");
                ui->label_tableView_BD_or_XML->setAlignment(Qt::AlignCenter);
                //Запрещаем редактировать таблицу и выделять ячейки
                ui->tableView_Display_BD_or_XML->setEditTriggers(QAbstractItemView::NoEditTriggers);
                ui->tableView_Display_BD_or_XML->setSelectionMode(QAbstractItemView::NoSelection);
                //блокировка кнопок (pbReadXML,pbAddRow)
                ui->pbReadXml->setDisabled(true);
                ui->pbAddRow->setDisabled(true);
                ui->pbRemoveRow->setDisabled(true);
                //Активизация кнопки (pbDisplayBDtable)
                ui->pbDisplayBDtable->setEnabled(true);
            }
        }
        file->close();
    }
    delete file;
}

//Кнопка добавления строки в конце таблицы БД
void MainWindow::on_pbAddRow_clicked()
{
    model_BD->insertRow(model_BD->rowCount());
    model_BD->submitAll();
    ui->pbAddRow->setDisabled(true);
}

//Кнопка удаляет выбранную строку в таблице БД
void MainWindow::on_pbRemoveRow_clicked()
{
    model_BD->removeRow(nRow);
    model_BD->submitAll();
    model_BD->select();
    //проверка есть ли данные в таблице после удаления
    if(model_BD->index(0,0).isValid() == false)
    {
        ui->pbGenerationXML->setDisabled(true);
    }
    else
    {
        ui->pbGenerationXML->setDisabled(false);
    }
    //выключаем кнопку удалить строку
    ui->pbRemoveRow->setDisabled(true);
}


//||--------------------------------Раздел кода со слотами------------------------------------||
//слот активизирует кнопку pbGenerationXML
void MainWindow::slotUnlock_buttons()
{
    ui->pbGenerationXML->setDisabled(false);
    ui->pbAddRow->setDisabled(false);
}

//||---------------------------------Раздел кода для работы с представлением tableView_Display_BD_or_XML---------||
//клик по таблице передаёт индекс выбранной строки переменной nRow
void MainWindow::on_tableView_Display_BD_or_XML_clicked(const QModelIndex &index)
{
    //условие чтобы кнопка удалить строку активизировалась только по клику на таблице БД
    if(ui->tableView_Display_BD_or_XML->model() == model_BD)
    {
        nRow = index.row();
        ui->pbRemoveRow->setEnabled(true);
    }
}
