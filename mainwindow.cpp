#include "mainwindow.h"
#include "customform.h"

#include <QScrollArea>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_area = new QScrollArea(this);
    m_area->setWidgetResizable(true);
    m_area->setFrameShape(QFrame::NoFrame);

    m_container = new QWidget;
    m_container->setObjectName("formContainer");
    m_container->setMinimumSize(1400, 900);

    m_area->setWidget(m_container);
    setCentralWidget(m_area);

    auto *tb = addToolBar("Tools");
    QAction *addAct = tb->addAction("添加组件");
    QAction *addWideAct = tb->addAction("添加宽组件");
    connect(addAct, &QAction::triggered, this, &MainWindow::addComponent);
    connect(addWideAct, &QAction::triggered, this, &MainWindow::addWideComponent);

    resize(1280, 800);
}

QWidget* MainWindow::container() const
{
    return m_container;
}

void MainWindow::addComponent()
{
    auto *f = new CustomForm(container());
    f->setGeometry(40 + 20 * m_forms.size(), 40 + 20 * m_forms.size(), 420, 280);
    f->show();

    connect(f, &CustomForm::moved, this, &MainWindow::onFormMoved);
    connect(f, &CustomForm::requestClose, this, &MainWindow::onFormClose);

    m_forms << QPointer<CustomForm>(f);
    maybeExpandContainer();
}

void MainWindow::addWideComponent()
{
    auto *f = new CustomForm(container());
    f->setGeometry(60, 360, 720, 300);
    f->show();

    connect(f, &CustomForm::moved, this, &MainWindow::onFormMoved);
    connect(f, &CustomForm::requestClose, this, &MainWindow::onFormClose);

    m_forms << QPointer<CustomForm>(f);
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