#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QList>
#include <QJsonArray>

class QScrollArea;
class QWidget;
class CustomForm;
class FormCanvas;

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
    void saveLayout();
    void loadLayout();

private:
    FormCanvas* container() const;
    void maybeExpandContainer();
    CustomForm* createForm(const QRect &geom);
    QJsonArray serializeForms() const;
    void recreateFromJson(const QJsonArray &arr);

private:
    QScrollArea *m_area = nullptr;
    FormCanvas  *m_container = nullptr;
    QList<QPointer<CustomForm>> m_forms;
};