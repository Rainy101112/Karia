#include "settingsdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include <KLocalizedString>

SettingsDialog::SettingsDialog(const QUrl &rpcUrl, const QString &secret, int pollInterval,
                               QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("设置"));

    m_urlEdit = new QLineEdit(rpcUrl.toString(), this);
    m_urlEdit->setPlaceholderText(QStringLiteral("http://127.0.0.1:6800/jsonrpc"));

    m_secretEdit = new QLineEdit(secret, this);
    m_secretEdit->setEchoMode(QLineEdit::Password);
    m_secretEdit->setPlaceholderText(i18n("可选：aria2 RPC secret"));

    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(1, 60);
    m_intervalSpin->setValue(pollInterval);
    m_intervalSpin->setSuffix(i18n(" 秒"));

    auto *form = new QFormLayout;
    form->addRow(i18n("RPC 地址："), m_urlEdit);
    form->addRow(i18n("RPC 密钥："), m_secretEdit);
    form->addRow(i18n("刷新间隔："), m_intervalSpin);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QUrl SettingsDialog::rpcUrl() const
{
    QString text = m_urlEdit->text().trimmed();
    if (!text.contains(QStringLiteral("://")))
        text.prepend(QStringLiteral("http://"));
    return QUrl(text);
}

QString SettingsDialog::secret() const
{
    return m_secretEdit->text();
}

int SettingsDialog::pollInterval() const
{
    return m_intervalSpin->value();
}
