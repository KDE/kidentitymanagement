/*
    SPDX-FileCopyrightText: 2002-2004 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "identity.h"
using namespace Qt::Literals::StringLiterals;

#include "kidentitymanagementcore_debug.h"
#include <KConfigGroup>
#include <KEmailAddress>

#include <QByteArray>
#include <QHostInfo>
#include <QMimeData>

using namespace KIdentityManagementCore;

static Identity *identityNull = nullptr;

Q_DECLARE_METATYPE(KIdentityManagementCore::Signature)

Identity::Identity(const QString &id, const QString &fullName, const QString &emailAddr, const QString &organization, const QString &replyToAddr)
{
    qRegisterMetaType<Signature>();
    setProperty(QLatin1StringView(s_uoid), 0);
    setProperty(QLatin1StringView(s_identity), id);
    setProperty(QLatin1StringView(s_name), fullName);
    setProperty(QLatin1StringView(s_primaryEmail), emailAddr);
    setProperty(QLatin1StringView(s_organization), organization);
    setProperty(QLatin1StringView(s_replyto), replyToAddr);
    // FIXME KF6: Sonnet::defaultLanguageName is gone
    // setDictionary( Sonnet::defaultLanguageName() );
    setProperty(QLatin1StringView(s_disabledFcc), false);
    setProperty(QLatin1StringView(s_defaultDomainName), QHostInfo::localHostName());
}

const Identity &Identity::null()
{
    if (!identityNull) {
        identityNull = new Identity;
    }
    return *identityNull;
}

bool Identity::isNull() const
{
    QHash<QString, QVariant>::const_iterator i = mPropertiesMap.constBegin();

    while (i != mPropertiesMap.constEnd()) {
        const QString &key = i.key();
        // Take into account that the defaultDomainName for a null identity is not empty
        if (key == QLatin1StringView(s_defaultDomainName)) {
            ++i;
            continue;
        }
        // Take into account that the dictionary for a null identity is not empty
        if (key == QLatin1StringView(s_dict)) {
            ++i;
            continue;
        }
        // Take into account that disableFcc == false for a null identity
        if (key == QLatin1StringView(s_disabledFcc) && i.value().toBool() == false) {
            ++i;
            continue;
        }
        // The uoid is 0 by default, so ignore this
        if (!(key == QLatin1StringView(s_uoid) && i.value().toUInt() == 0)) {
            if (!i.value().isNull() || (i.value().metaType().id() == QMetaType::QString && !i.value().toString().isEmpty())) {
                return false;
            }
        }
        ++i;
    }

    return true;
}

void Identity::readConfig(const KConfigGroup &config)
{
    // get all keys and convert them to our QHash.
    QMap<QString, QString> entries = config.entryMap();
    QMap<QString, QString>::const_iterator i = entries.constBegin();
    QMap<QString, QString>::const_iterator end = entries.constEnd();
    while (i != end) {
        const QString &key = i.key();
        if (key == QLatin1StringView(s_emailAliases) || key == QLatin1StringView(s_activities)) {
            // HACK: Read s_emailAliases as a stringlist
            mPropertiesMap.insert(key, config.readEntry(key, QStringList()));
        } else {
            mPropertiesMap.insert(key, config.readEntry(key));
        }
        ++i;
    }
    // needs to update to v5.21.41
    // Check if we update to to encryption override mode
    // before we had only auto_encrypt and auto_sign and no global setting
    if (!mPropertiesMap.contains(QLatin1StringView(s_encryptionOverride)) && !mPropertiesMap.contains(QLatin1StringView(s_warnnotencrypt))
        && !mPropertiesMap.contains(QLatin1StringView(s_warnnotsign))) {
        setEncryptionOverride(true);
    }
    mSignature.readConfig(config);
}

void Identity::writeConfig(KConfigGroup &config) const
{
    QHash<QString, QVariant>::const_iterator i = mPropertiesMap.constBegin();
    QHash<QString, QVariant>::const_iterator end = mPropertiesMap.constEnd();
    while (i != end) {
        config.writeEntry(i.key(), i.value());
        qCDebug(KIDENTITYMANAGEMENT_LOG) << "Store:" << i.key() << ":" << i.value();
        ++i;
    }
    if (!mPropertiesMap.contains(QLatin1StringView(s_encryptionOverride))) {
        config.writeEntry(QLatin1StringView(s_encryptionOverride), false);
        qCDebug(KIDENTITYMANAGEMENT_LOG) << "Add" << s_encryptionOverride << ":" << false;
    }
    mSignature.writeConfig(config);
}

bool Identity::mailingAllowed() const
{
    return !property(QLatin1StringView(s_primaryEmail)).toString().isEmpty();
}

QString Identity::mimeDataType()
{
    return QStringLiteral("application/x-kmail-identity-drag");
}

bool Identity::canDecode(const QMimeData *md)
{
    if (md) {
        return md->hasFormat(mimeDataType());
    } else {
        return false;
    }
}

void Identity::populateMimeData(QMimeData *md) const
{
    QByteArray a;
    {
        QDataStream s(&a, QIODevice::WriteOnly);
        s << *this;
    }
    md->setData(mimeDataType(), a);
}

Identity Identity::fromMimeData(const QMimeData *md)
{
    Identity i;
    if (canDecode(md)) {
        QByteArray ba = md->data(mimeDataType());
        QDataStream s(&ba, QIODevice::ReadOnly);
        s >> i;
    }
    return i;
}

// ------------------ Operators --------------------------//

QDataStream &KIdentityManagementCore::operator<<(QDataStream &stream, const KIdentityManagementCore::Identity &i)
{
    return stream << static_cast<quint32>(i.uoid()) << i.mPropertiesMap[QLatin1StringView(s_identity)] << i.mPropertiesMap[QLatin1StringView(s_name)]
                  << i.mPropertiesMap[QLatin1StringView(s_organization)] << i.mPropertiesMap[QLatin1StringView(s_pgps)]
                  << i.mPropertiesMap[QLatin1StringView(s_pgpe)] << i.mPropertiesMap[QLatin1StringView(s_smimes)]
                  << i.mPropertiesMap[QLatin1StringView(s_smimee)] << i.mPropertiesMap[QLatin1StringView(s_primaryEmail)]
                  << i.mPropertiesMap[QLatin1StringView(s_emailAliases)] << i.mPropertiesMap[QLatin1StringView(s_replyto)]
                  << i.mPropertiesMap[QLatin1StringView(s_bcc)] << i.mPropertiesMap[QLatin1StringView(s_vcard)]
                  << i.mPropertiesMap[QLatin1StringView(s_transport)] << i.mPropertiesMap[QLatin1StringView(s_fcc)]
                  << i.mPropertiesMap[QLatin1StringView(s_drafts)] << i.mPropertiesMap[QLatin1StringView(s_templates)] << i.mSignature
                  << i.mPropertiesMap[QLatin1StringView(s_dict)] << i.mPropertiesMap[QLatin1StringView(s_xface)]
                  << i.mPropertiesMap[QLatin1StringView(s_xfaceenabled)] << i.mPropertiesMap[QLatin1StringView(s_face)]
                  << i.mPropertiesMap[QLatin1StringView(s_faceenabled)] << i.mPropertiesMap[QLatin1StringView(s_prefcrypt)]
                  << i.mPropertiesMap[QLatin1StringView(s_cc)] << i.mPropertiesMap[QLatin1StringView(s_attachVcard)]
                  << i.mPropertiesMap[QLatin1StringView(s_autocorrectionLanguage)] << i.mPropertiesMap[QLatin1StringView(s_disabledFcc)]
                  << i.mPropertiesMap[QLatin1StringView(s_defaultDomainName)] << i.mPropertiesMap[QLatin1StringView(s_autocryptEnabled)]
                  << i.mPropertiesMap[QLatin1StringView(s_autocryptPrefer)] << i.mPropertiesMap[QLatin1StringView(s_encryptionOverride)]
                  << i.mPropertiesMap[QLatin1StringView(s_pgpautosign)] << i.mPropertiesMap[QLatin1StringView(s_pgpautoencrypt)]
                  << i.mPropertiesMap[QLatin1StringView(s_warnnotencrypt)] << i.mPropertiesMap[QLatin1StringView(s_warnnotsign)]
                  << i.mPropertiesMap[QLatin1StringView(s_activities)];
}

QDataStream &KIdentityManagementCore::operator>>(QDataStream &stream, KIdentityManagementCore::Identity &i)
{
    quint32 uoid;
    stream >> uoid >> i.mPropertiesMap[QLatin1StringView(s_identity)] >> i.mPropertiesMap[QLatin1StringView(s_name)]
        >> i.mPropertiesMap[QLatin1StringView(s_organization)] >> i.mPropertiesMap[QLatin1StringView(s_pgps)] >> i.mPropertiesMap[QLatin1StringView(s_pgpe)]
        >> i.mPropertiesMap[QLatin1StringView(s_smimes)] >> i.mPropertiesMap[QLatin1StringView(s_smimee)] >> i.mPropertiesMap[QLatin1StringView(s_primaryEmail)]
        >> i.mPropertiesMap[QLatin1StringView(s_emailAliases)] >> i.mPropertiesMap[QLatin1StringView(s_replyto)] >> i.mPropertiesMap[QLatin1StringView(s_bcc)]
        >> i.mPropertiesMap[QLatin1StringView(s_vcard)] >> i.mPropertiesMap[QLatin1StringView(s_transport)] >> i.mPropertiesMap[QLatin1StringView(s_fcc)]
        >> i.mPropertiesMap[QLatin1StringView(s_drafts)] >> i.mPropertiesMap[QLatin1StringView(s_templates)] >> i.mSignature
        >> i.mPropertiesMap[QLatin1StringView(s_dict)] >> i.mPropertiesMap[QLatin1StringView(s_xface)] >> i.mPropertiesMap[QLatin1StringView(s_xfaceenabled)]
        >> i.mPropertiesMap[QLatin1StringView(s_face)] >> i.mPropertiesMap[QLatin1StringView(s_faceenabled)] >> i.mPropertiesMap[QLatin1StringView(s_prefcrypt)]
        >> i.mPropertiesMap[QLatin1StringView(s_cc)] >> i.mPropertiesMap[QLatin1StringView(s_attachVcard)]
        >> i.mPropertiesMap[QLatin1StringView(s_autocorrectionLanguage)] >> i.mPropertiesMap[QLatin1StringView(s_disabledFcc)]
        >> i.mPropertiesMap[QLatin1StringView(s_defaultDomainName)] >> i.mPropertiesMap[QLatin1StringView(s_autocryptEnabled)]
        >> i.mPropertiesMap[QLatin1StringView(s_autocryptPrefer)] >> i.mPropertiesMap[QLatin1StringView(s_encryptionOverride)]
        >> i.mPropertiesMap[QLatin1StringView(s_pgpautosign)] >> i.mPropertiesMap[QLatin1StringView(s_pgpautoencrypt)]
        >> i.mPropertiesMap[QLatin1StringView(s_warnnotencrypt)] >> i.mPropertiesMap[QLatin1StringView(s_warnnotsign)]
        >> i.mPropertiesMap[QLatin1StringView(s_activities)];

    i.setProperty(QLatin1StringView(s_uoid), uoid);
    return stream;
}

bool Identity::operator<(const Identity &other) const
{
    if (isDefault()) {
        return true;
    }
    if (other.isDefault()) {
        return false;
    }
    return identityName() < other.identityName();
}

bool Identity::operator>(const Identity &other) const
{
    if (isDefault()) {
        return false;
    }
    if (other.isDefault()) {
        return true;
    }
    return identityName() > other.identityName();
}

bool Identity::operator<=(const Identity &other) const
{
    return !operator>(other);
}

bool Identity::operator>=(const Identity &other) const
{
    return !operator<(other);
}

bool Identity::operator==(const Identity &other) const
{
    // The deserializer fills in the QHash will lots of invalid variants, which
    // is OK, but the CTOR doesn't fill the hash with the missing fields, so
    // regular mPropertiesMap == other.mPropertiesMap comparison will fail.
    // This algo considers both maps equal even if one map does not contain the
    // key and the other one contains the key but with an invalid value
    for (const auto &pair : {qMakePair(mPropertiesMap, other.mPropertiesMap), qMakePair(other.mPropertiesMap, mPropertiesMap)}) {
        const auto lhs = pair.first;
        const auto rhs = pair.second;
        for (auto lhsIt = lhs.constBegin(), lhsEnd = lhs.constEnd(); lhsIt != lhsEnd; ++lhsIt) {
            const auto rhsIt = rhs.constFind(lhsIt.key());
            // Does the other map contain the key?
            if (rhsIt == rhs.constEnd()) {
                // It does not, so check if our value is invalid, if yes, consider it
                // equal to not present and continue
                if (lhsIt->isValid()) {
                    return false;
                }
            } else if (lhsIt.value() != rhsIt.value()) {
                // Both maps have the key, but different value -> different maps
                return false;
            }
        }
    }

    return mSignature == other.mSignature;
}

bool Identity::operator!=(const Identity &other) const
{
    return !operator==(other);
}

// --------------------- Getters -----------------------------//

QVariant Identity::property(const QString &key) const
{
    if (key == QLatin1StringView(s_signature)) {
        return QVariant::fromValue(mSignature);
    } else {
        return mPropertiesMap.value(key);
    }
}

QString Identity::fullEmailAddr() const
{
    const QString name = mPropertiesMap.value(QLatin1StringView(s_name)).toString();
    const QString mail = mPropertiesMap.value(QLatin1StringView(s_primaryEmail)).toString();

    if (name.isEmpty()) {
        return mail;
    }

    const QString specials(QStringLiteral("()<>@,.;:[]"));

    QString result;

    // add DQUOTE's if necessary:
    bool needsQuotes = false;
    const int nameLength(name.length());
    for (int i = 0; i < nameLength; i++) {
        if (specials.contains(name[i])) {
            needsQuotes = true;
        } else if (name[i] == QLatin1Char('\\') || name[i] == QLatin1Char('"')) {
            needsQuotes = true;
            result += QLatin1Char('\\');
        }
        result += name[i];
    }

    if (needsQuotes) {
        result.insert(0, QLatin1Char('"'));
        result += QLatin1Char('"');
    }

    result += " <"_L1 + mail + QLatin1Char('>');

    return result;
}

QString Identity::identityName() const
{
    return property(QLatin1StringView(s_identity)).toString();
}

QString Identity::signatureText(bool *ok) const
{
    return mSignature.withSeparator(ok);
}

bool Identity::signatureIsInlinedHtml() const
{
    return mSignature.isInlinedHtml();
}

bool Identity::isDefault() const
{
    return mIsDefault;
}

uint Identity::uoid() const
{
    return property(QLatin1StringView(s_uoid)).toInt();
}

QString Identity::fullName() const
{
    return property(QLatin1StringView(s_name)).toString();
}

QString Identity::organization() const
{
    return property(QLatin1StringView(s_organization)).toString();
}

QByteArray Identity::pgpEncryptionKey() const
{
    return property(QLatin1StringView(s_pgpe)).toByteArray();
}

QByteArray Identity::pgpSigningKey() const
{
    return property(QLatin1StringView(s_pgps)).toByteArray();
}

QByteArray Identity::smimeEncryptionKey() const
{
    return property(QLatin1StringView(s_smimee)).toByteArray();
}

QByteArray Identity::smimeSigningKey() const
{
    return property(QLatin1StringView(s_smimes)).toByteArray();
}

QString Identity::preferredCryptoMessageFormat() const
{
    return property(QLatin1StringView(s_prefcrypt)).toString();
}

QString Identity::primaryEmailAddress() const
{
    return property(QLatin1StringView(s_primaryEmail)).toString();
}

const QStringList Identity::emailAliases() const
{
    return property(QLatin1StringView(s_emailAliases)).toStringList();
}

QString Identity::vCardFile() const
{
    return property(QLatin1StringView(s_vcard)).toString();
}

bool Identity::attachVcard() const
{
    return property(QLatin1StringView(s_attachVcard)).toBool();
}

QString Identity::replyToAddr() const
{
    return property(QLatin1StringView(s_replyto)).toString();
}

QString Identity::bcc() const
{
    return property(QLatin1StringView(s_bcc)).toString();
}

QString Identity::cc() const
{
    return property(QLatin1StringView(s_cc)).toString();
}

Signature &Identity::signature()
{
    return mSignature;
}

bool Identity::isXFaceEnabled() const
{
    return property(QLatin1StringView(s_xfaceenabled)).toBool();
}

QString Identity::xface() const
{
    return property(QLatin1StringView(s_xface)).toString();
}

bool Identity::isFaceEnabled() const
{
    return property(QLatin1StringView(s_faceenabled)).toBool();
}

QString Identity::face() const
{
    return property(QLatin1StringView(s_face)).toString();
}

QString Identity::dictionary() const
{
    return property(QLatin1StringView(s_dict)).toString();
}

QString Identity::templates() const
{
    const QString str = property(QLatin1StringView(s_templates)).toString();
    return verifyAkonadiId(str);
}

QString Identity::drafts() const
{
    const QString str = property(QLatin1StringView(s_drafts)).toString();
    return verifyAkonadiId(str);
}

QString Identity::fcc() const
{
    const QString str = property(QLatin1StringView(s_fcc)).toString();
    return verifyAkonadiId(str);
}

QString Identity::transport() const
{
    return property(QLatin1StringView(s_transport)).toString();
}

bool Identity::signatureIsCommand() const
{
    return mSignature.type() == Signature::FromCommand;
}

bool Identity::signatureIsPlainFile() const
{
    return mSignature.type() == Signature::FromFile;
}

bool Identity::signatureIsInline() const
{
    return mSignature.type() == Signature::Inlined;
}

bool Identity::useSignatureFile() const
{
    return signatureIsPlainFile() || signatureIsCommand();
}

QString Identity::signatureInlineText() const
{
    return mSignature.text();
}

QString Identity::signatureFile() const
{
    return mSignature.path();
}

QString Identity::autocorrectionLanguage() const
{
    return property(QLatin1StringView(s_autocorrectionLanguage)).toString();
}

// --------------------- Setters -----------------------------//

void Identity::setProperty(const QString &key, const QVariant &value)
{
    if (key == QLatin1StringView(s_signature)) {
        mSignature = value.value<Signature>();
    } else {
        if (value.isNull() || (value.metaType().id() == QMetaType::QString && value.toString().isEmpty())) {
            mPropertiesMap.remove(key);
        } else {
            mPropertiesMap.insert(key, value);
        }
    }
}

void Identity::setUoid(uint aUoid)
{
    setProperty(QLatin1StringView(s_uoid), aUoid);
}

void Identity::setIdentityName(const QString &name)
{
    setProperty(QLatin1StringView(s_identity), name);
}

void Identity::setFullName(const QString &str)
{
    setProperty(QLatin1StringView(s_name), str);
}

void Identity::setOrganization(const QString &str)
{
    setProperty(QLatin1StringView(s_organization), str);
}

void Identity::setPGPSigningKey(const QByteArray &str)
{
    setProperty(QLatin1StringView(s_pgps), QLatin1StringView(str));
}

void Identity::setPGPEncryptionKey(const QByteArray &str)
{
    setProperty(QLatin1StringView(s_pgpe), QLatin1StringView(str));
}

void Identity::setSMIMESigningKey(const QByteArray &str)
{
    setProperty(QLatin1StringView(s_smimes), QLatin1StringView(str));
}

void Identity::setSMIMEEncryptionKey(const QByteArray &str)
{
    setProperty(QLatin1StringView(s_smimee), QLatin1StringView(str));
}

void Identity::setPrimaryEmailAddress(const QString &email)
{
    setProperty(QLatin1StringView(s_primaryEmail), email);
}

void Identity::setEmailAliases(const QStringList &aliases)
{
    setProperty(QLatin1StringView(s_emailAliases), aliases);
}

const QStringList Identity::activities() const
{
    return property(QLatin1StringView(s_activities)).toStringList();
}

void Identity::setActivities(const QStringList &a)
{
    setProperty(QLatin1StringView(s_activities), a);
}

void Identity::setVCardFile(const QString &str)
{
    setProperty(QLatin1StringView(s_vcard), str);
}

void Identity::setAttachVcard(bool attachment)
{
    setProperty(QLatin1StringView(s_attachVcard), attachment);
}

void Identity::setReplyToAddr(const QString &str)
{
    setProperty(QLatin1StringView(s_replyto), str);
}

void Identity::setSignatureFile(const QString &str)
{
    mSignature.setPath(str, signatureIsCommand());
}

void Identity::setSignatureInlineText(const QString &str)
{
    mSignature.setText(str);
}

void Identity::setTransport(const QString &str)
{
    setProperty(QLatin1StringView(s_transport), str);
}

void Identity::setFcc(const QString &str)
{
    setProperty(QLatin1StringView(s_fcc), str);
}

void Identity::setDrafts(const QString &str)
{
    setProperty(QLatin1StringView(s_drafts), str);
}

void Identity::setTemplates(const QString &str)
{
    setProperty(QLatin1StringView(s_templates), str);
}

void Identity::setDictionary(const QString &str)
{
    setProperty(QLatin1StringView(s_dict), str);
}

void Identity::setBcc(const QString &str)
{
    setProperty(QLatin1StringView(s_bcc), str);
}

void Identity::setCc(const QString &str)
{
    setProperty(QLatin1StringView(s_cc), str);
}

void Identity::setIsDefault(bool flag)
{
    mIsDefault = flag;
}

void Identity::setPreferredCryptoMessageFormat(const QString &str)
{
    setProperty(QLatin1StringView(s_prefcrypt), str);
}

void Identity::setXFace(const QString &str)
{
    QString strNew = str;
    strNew.remove(QLatin1Char(' '));
    strNew.remove(QLatin1Char('\n'));
    strNew.remove(QLatin1Char('\r'));
    setProperty(QLatin1StringView(s_xface), strNew);
}

void Identity::setXFaceEnabled(bool on)
{
    setProperty(QLatin1StringView(s_xfaceenabled), on);
}

void Identity::setFace(const QString &str)
{
    QString strNew = str;
    strNew.remove(QLatin1Char(' '));
    strNew.remove(QLatin1Char('\n'));
    strNew.remove(QLatin1Char('\r'));
    setProperty(QLatin1StringView(s_face), strNew);
}

void Identity::setFaceEnabled(bool on)
{
    setProperty(QLatin1StringView(s_faceenabled), on);
}

void Identity::setSignature(const Signature &sig)
{
    mSignature = sig;
}

bool Identity::matchesEmailAddress(const QString &addr) const
{
    const QString addrSpec = KEmailAddress::extractEmailAddress(addr).toLower();
    if (addrSpec == primaryEmailAddress().toLower()) {
        return true;
    }

    const QStringList lst = emailAliases();
    for (const QString &alias : lst) {
        if (alias.toLower() == addrSpec) {
            return true;
        }
    }

    return false;
}

void Identity::setAutocorrectionLanguage(const QString &language)
{
    setProperty(QLatin1StringView(s_autocorrectionLanguage), language);
}

bool Identity::disabledFcc() const
{
    const QVariant var = property(QLatin1StringView(s_disabledFcc));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setDisabledFcc(bool disable)
{
    setProperty(QLatin1StringView(s_disabledFcc), disable);
}

bool Identity::pgpAutoSign() const
{
    const QVariant var = property(QLatin1StringView(s_pgpautosign));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setPgpAutoSign(bool autoSign)
{
    setProperty(QLatin1StringView(s_pgpautosign), autoSign);
}

bool Identity::pgpAutoEncrypt() const
{
    const QVariant var = property(QLatin1StringView(s_pgpautoencrypt));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setPgpAutoEncrypt(bool autoEncrypt)
{
    setProperty(QLatin1StringView(s_pgpautoencrypt), autoEncrypt);
}

bool Identity::autocryptEnabled() const
{
    const auto var = property(QLatin1StringView(s_autocryptEnabled));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setAutocryptEnabled(const bool on)
{
    setProperty(QLatin1StringView(s_autocryptEnabled), on);
}

bool Identity::autocryptPrefer() const
{
    const auto var = property(QLatin1StringView(s_autocryptPrefer));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setAutocryptPrefer(const bool on)
{
    setProperty(QLatin1StringView(s_autocryptPrefer), on);
}

bool Identity::encryptionOverride() const
{
    const auto var = property(QLatin1StringView(s_encryptionOverride));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setEncryptionOverride(const bool on)
{
    setProperty(QLatin1StringView(s_encryptionOverride), on);
}

bool Identity::warnNotEncrypt() const
{
    const auto var = property(QLatin1StringView(s_warnnotencrypt));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setWarnNotEncrypt(const bool on)
{
    setProperty(QLatin1StringView(s_warnnotencrypt), on);
}

bool Identity::warnNotSign() const
{
    const auto var = property(QLatin1StringView(s_warnnotsign));
    if (var.isNull()) {
        return false;
    } else {
        return var.toBool();
    }
}

void Identity::setWarnNotSign(const bool on)
{
    setProperty(QLatin1StringView(s_warnnotsign), on);
}

QString Identity::defaultDomainName() const
{
    return property(QLatin1StringView(s_defaultDomainName)).toString();
}

void Identity::setDefaultDomainName(const QString &domainName)
{
    setProperty(QLatin1StringView(s_defaultDomainName), domainName);
}

QString Identity::verifyAkonadiId(const QString &str) const
{
    if (str.isEmpty()) {
        return str;
    }
    bool ok = false;
    const qlonglong val = str.toLongLong(&ok);
    Q_UNUSED(val)
    if (ok) {
        return str;
    } else {
        return {};
    }
}

#include "moc_identity.cpp"
