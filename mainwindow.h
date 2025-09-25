#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QList>

class QScrollArea;
class QWidget;
class CustomForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void addComponent();
    void addWideComponent();
    void onFormMoved(const QRect &r);
    void onFormClose(CustomForm *f);

private:
    QWidget* container() const;
    void maybeExpandContainer();

private:
    QScrollArea *m_area = nullptr;
    QWidget     *m_container = nullptr;
    QList<QPointer<CustomForm>> m_forms;
};