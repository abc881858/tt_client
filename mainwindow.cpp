#include "mainwindow.h"
#include "customform.h"
#include "formcanvas.h"

#include <QScrollArea>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_area = new QScrollArea(this);
    m_area->setWidgetResizable(true);
    m_area->setFrameShape(QFrame::NoFrame);

    m_container = new FormCanvas;
    m_container->setObjectName("formContainer");
    m_container->setMinimumSize(1400, 900);

    m_area->setWidget(m_container);
    setCentralWidget(m_area);

    auto *tb = addToolBar("Tools");
    QAction *addAct = tb->addAction("添加组件");
    QAction *addWideAct = tb->addAction("添加宽组件");
    tb->addSeparator();
    QAction *saveAct = tb->addAction("保存布局");
    QAction *loadAct = tb->addAction("加载布局");
    connect(addAct, &QAction::triggered, this, &MainWindow::addComponent);
    connect(addWideAct, &QAction::triggered, this, &MainWindow::addWideComponent);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveLayout);
    connect(loadAct, &QAction::triggered, this, &MainWindow::loadLayout);

    resize(1280, 800);
}

FormCanvas* MainWindow::container() const
{
    return m_container;
}

void MainWindow::addComponent()
{
    createForm(QRect(40 + 20 * m_forms.size(), 40 + 20 * m_forms.size(), 420, 280));
    maybeExpandContainer();
}

void MainWindow::addWideComponent()
{
    createForm(QRect(60, 360, 720, 300));
    maybeExpandContainer();
}

void MainWindow::onFormMoved(const QRect &)
{
    maybeExpandContainer();
}

void MainWindow::onFormClose(CustomForm *f)
{
    if (!f) return;
    m_forms.removeAll(f);
    f->deleteLater();
    maybeExpandContainer();
}

void MainWindow::maybeExpandContainer()
{
    int maxRight = 0, maxBottom = 0;
    // 清除空指针
    for (auto it = m_forms.begin(); it != m_forms.end(); ) {
        if (it->isNull()) it = m_forms.erase(it);
        else ++it;
    }
    for (const auto &pf : m_forms) {
        if (auto *w = pf.data()) {
            QRect g = w->geometry();
            maxRight  = std::max(maxRight,  g.right());
            maxBottom = std::max(maxBottom, g.bottom());
        }
    }
    // 留边距
    maxRight  += 40;
    maxBottom += 40;

    const int needW = std::max(maxRight,  m_container->minimumWidth());
    const int needH = std::max(maxBottom, m_container->minimumHeight());
    if (needW != m_container->minimumWidth() || needH != m_container->minimumHeight())
        m_container->setMinimumSize(needW, needH);
}

CustomForm* MainWindow::createForm(const QRect &geom)
{
    auto *f = new CustomForm(container());
    f->setGeometry(geom);
    f->show();

    connect(f, &CustomForm::moved, this, &MainWindow::onFormMoved);
    connect(f, &CustomForm::requestClose, this, &MainWindow::onFormClose);

    m_forms << QPointer<CustomForm>(f);
    return f;
}

QJsonArray MainWindow::serializeForms() const
{
    QJsonArray arr;
    for (const auto &pf : m_forms) {
        if (auto *w = pf.data()) {
            const QRect g = w->geometry();
            QJsonObject obj;
            obj["x"] = g.x();
            obj["y"] = g.y();
            obj["w"] = g.width();
            obj["h"] = g.height();
            arr.append(obj);
        }
    }
    return arr;
}

void MainWindow::recreateFromJson(const QJsonArray &arr)
{
    for (const auto &pf : m_forms) {
        if (auto *w = pf.data())
            w->deleteLater();
    }
    m_forms.clear();

    for (const QJsonValue &value : arr) {
        if (!value.isObject())
            continue;
        const QJsonObject obj = value.toObject();
        const int x = obj.value("x").toInt();
        const int y = obj.value("y").toInt();
        const int w = obj.value("w").toInt(420);
        const int h = obj.value("h").toInt(280);
        createForm(QRect(x, y, std::max(w, 1), std::max(h, 1)));
    }

    maybeExpandContainer();
}

void MainWindow::saveLayout()
{
    const QString fileName = QFileDialog::getSaveFileName(this, tr("保存布局"), QString(), tr("布局文件 (*.json)"));
    if (fileName.isEmpty())
        return;

    const QJsonArray arr = serializeForms();
    QJsonDocument doc(arr);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("保存失败"), tr("无法写入文件：%1").arg(file.errorString()));
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void MainWindow::loadLayout()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("加载布局"), QString(), tr("布局文件 (*.json)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("加载失败"), tr("无法读取文件：%1").arg(file.errorString()));
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        QMessageBox::warning(this, tr("加载失败"), tr("文件格式不正确"));
        return;
    }

    recreateFromJson(doc.array());
}