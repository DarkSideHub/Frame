#ifndef PLAINTEXTHANDLER_FILE
#define PLAINTEXTHANDLER_FILE

#include "Header.hpp"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
// TODO如何采用状态机重构
//  事件
namespace {
struct Start {};
struct End {};
struct StartSend {};
struct StartRead {};

// 动作

// 转化表

};  // namespace

class DOC {
 private:
  std::uint64_t Position;
  std::uint64_t selectionStart;
  std::uint64_t selectionEnd;
  QQuickTextDocument *Doc;

 public:
  DOC();
  virtual ~DOC() = default;

  std::shared_ptr<spdlog::logger> logger;
  QQuickTextDocument *GetDoc() const { return Doc; };
  void SetDoc(QQuickTextDocument *mDoc) noexcept;
  QTextCursor GetCursor() noexcept;
  inline void SetPosition(std::uint64_t mPosition) noexcept {
    Position = mPosition;
  };
  inline std::uint64_t GetPosition() noexcept { return Position; };
  virtual void Write(std::string_view text = "") noexcept = 0;
  virtual std::string Read() noexcept = 0;
};

enum class LOGLEVEL { Trace, Debug, Info, Warn, Error, Critical };
class EDITOR : public QObject, public DOC {
  Q_OBJECT
  Q_PROPERTY(
      QQuickTextDocument *qmldoc READ GetDoc WRITE SetDoc NOTIFY docChanged)
 private:
  QUrl m_fileUrl;

 public:
  explicit EDITOR(QObject *parent = nullptr) : QObject(parent){};
  ~EDITOR() = default;
  void Write(std::string_view text) noexcept override;
  std::string Read() noexcept override;
  void Work() noexcept;

 public slots:
  void load(const QUrl &FileUrl) noexcept;
  void Send() noexcept;

 signals:
  void loaded(const QString &text, int format);
  void fileUrlChanged();
  void done();
  void sendLog(const QString &log, LOGLEVEL Loglevel);
  void docChanged();
};

class LOGGER : public QObject, public DOC {
  Q_OBJECT
  Q_PROPERTY(
      QQuickTextDocument *qmllog READ GetDoc WRITE SetDoc NOTIFY docChanged)
 private:
 public:
  explicit LOGGER(QObject *parent = nullptr) : QObject(parent){};
  ~LOGGER() = default;
  void Write(std::string_view text) noexcept override;
  std::string Read() noexcept override;

 public slots:
  void ReceiveLog(const QString &log, LOGLEVEL Loglevel) noexcept;
 signals:
  void done();
  void docChanged();
};

#endif