#pragma once

#include <QDialog>
#include <QUrl>

class QLineEdit;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(const QUrl &rpcUrl, const QString &secret, int pollInterval,
                   QWidget *parent = nullptr);

    QUrl rpcUrl() const;
    QString secret() const;
    int pollInterval() const;

private:
    QLineEdit *m_urlEdit;
    QLineEdit *m_secretEdit;
    QSpinBox *m_intervalSpin;
};
