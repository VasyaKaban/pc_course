#pragma once

#include <QString>
#include <QCryptographicHash>
#include "NoReplace.hpp"

class User : NoCopy, NoMove
{
public:
	static User & Create()
	{
		static User usr;
		return usr;
	}

	QString GetLogin() {return login;}
	QString GetSha256Password() {return sha256_password;}
	QString GetRank() {return rank;}

	void SetLogin(const QString &_login) {login = _login;}
	void SetSha256Password(const QString &_sha256_password) {sha256_password = _sha256_password;}
	void SetRank(const QString &_rank) {rank = _rank;}

	static QString HashPasswordAsString(const QString &password)
	{
		return QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
	}

	static QByteArray HashPasswordAsByteArray(const QString &password)
	{
		return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
	}

private:
	User() = default;

	QString login;
	QString sha256_password;
	QString rank;
};
