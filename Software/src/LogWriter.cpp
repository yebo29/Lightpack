#include <QtCore>
#include <iostream>

#include "LightpackApplication.hpp"
#include "Settings.hpp"
#include "version.h"
#include "LogWriter.hpp"

using namespace std;



LogWriter::LogWriter()
{
	Q_ASSERT(g_logWriter == NULL);
	m_logStream.setString(&m_startupLogStore);
	m_disabled = false;
}

LogWriter::~LogWriter()
{
	Q_ASSERT(g_logWriter == NULL);
}

int LogWriter::initWith(const QString& logsDirPath)
{
	// Using locale codec for console output in messageHandler(..) function ( cout << qstring.toStdString() )
	QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());

	QDir logsDir(logsDirPath);
	if (logsDir.exists() == false) {
		std::cout << "mkdir " << logsDirPath.toStdString() << std::endl;
		if (logsDir.mkdir(logsDirPath) == false) {
			std::cerr << "Failed mkdir '" << logsDirPath.toStdString() << "' for logs. Exit." << std::endl;
			return LightpackApplication::LogsDirecroryCreationFail_ErrorCode;
		}
	}

	if (rotateLogFiles(logsDir) == false)
		std::cerr << "Failed to rotate old log files." << std::endl;

	const QString logFilePath = logsDirPath + "/Prismatik.0.log";
	QScopedPointer<QFile> logFile(new QFile(logFilePath));
	if (logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		QMutexLocker locker(&m_mutex);

		m_logStream.setDevice(logFile.take());
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		m_logStream << Qt::endl;
#else
		m_logStream << endl;
#endif

		const QDateTime currentDateTime(QDateTime::currentDateTime());
		m_logStream << currentDateTime.date().toString("yyyy_MM_dd") << " ";
		m_logStream << currentDateTime.time().toString("hh:mm:ss:zzz") << " Prismatik " << VERSION_STR;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		m_logStream << Qt::endl;
#else
		m_logStream << endl;
#endif

		// write the cached log
		m_logStream << m_startupLogStore;
		m_startupLogStore.clear();
	} else {
		std::cerr << "Failed to open logs file: '" << logFilePath.toStdString() << "'. Exit." << std::endl;
		return LightpackApplication::OpenLogsFail_ErrorCode;
	}

	qDebug() << "Logs file:" << logFilePath;

	return LightpackApplication::OK_ErrorCode;
}

void LogWriter::initDisabled()
{
	QMutexLocker locker(&m_mutex);
	m_startupLogStore.clear();
	m_disabled = true;
}

void LogWriter::writeMessage(const QString& msg, Level level)
{
	static const char* s_logLevelNames[] = { "Debug", "Warning", "Critical", "Fatal" };
	Q_ASSERT(level >= Debug && level < LevelCount);
	Q_STATIC_ASSERT(sizeof(s_logLevelNames)/sizeof(s_logLevelNames[0]) == LevelCount);

	const QString timeMark = QDateTime::currentDateTime().time().toString("hh:mm:ss:zzz");
	const QString finalMsg = QString("%1 %2: %3\n").arg(timeMark, s_logLevelNames[level], msg);
	QMutexLocker locker(&m_mutex);
	cerr << finalMsg.toStdString();
	if (!m_disabled) {
		m_logStream << finalMsg;
		m_logStream.flush();
	}
}

void LogWriter::messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
	static const LogWriter::Level s_msgType2Loglevel[] = {
		LogWriter::Debug, LogWriter::Warn, LogWriter::Critical, LogWriter::Fatal, LogWriter::Debug
	};
	Q_ASSERT(type >= 0 && type < sizeof(s_msgType2Loglevel)/sizeof(s_msgType2Loglevel[0]));
	Q_UNUSED(ctx);
	if (g_logWriter)
		g_logWriter->writeMessage(msg, s_msgType2Loglevel[type]);

	if (type == QtFatalMsg) {
		exit(LightpackApplication::QFatalMessageHandler_ErrorCode);
	}
}

bool LogWriter::rotateLogFiles(const QDir& logsDir)
{
	if (!logsDir.exists())
		return false;

	QStringList logFiles = logsDir.entryList(QStringList("Prismatik.?.log"), QDir::Files, QDir::Name);
	// Delete all old files except last |StoreLogsLaunches| files.
	for (int i = logFiles.count() - 1; i >= StoreLogsLaunches - 1; i--)
	{
		if (!QFile::remove(logsDir.absoluteFilePath(logFiles.at(i)))) {
			qCritical() << "Failed to remove log: " << logFiles.at(i);
			return false;
		}
	}

	logFiles = logsDir.entryList(QStringList("Prismatik.?.log"), QDir::Files, QDir::Name);
	Q_ASSERT(logFiles.count() <= StoreLogsLaunches);
	// Move Prismatik.[N].log file to Prismatik.[N+1].log
	for (int i = logFiles.count() - 1; i >= 0; i--)
	{
		const QStringList& splitted = logFiles[i].split('.');
		Q_ASSERT(splitted.length() == 3);

		const int num = splitted.at(1).toInt();
		const QString from = logsDir.absoluteFilePath(logFiles.at(i));
		const QString to = logsDir.absoluteFilePath(QString("Prismatik.") + QString::number(num + 1) + ".log");

		if (QFile::exists(to))
			QFile::remove(to);

		qDebug() << "Rename log: " << from << " to " << to;

		if (!QFile::rename(from, to)) {
			qCritical() << "Failed to rename log: " << from << " to " << to;
			return false;
		}
	}

	return true;
}

// static
LogWriter* LogWriter::g_logWriter = NULL;
