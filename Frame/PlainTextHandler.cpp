#include "../src/httplib/yhttp.hpp"
#include "moc_PlainTextHandler.cpp"

DOC::DOC() {
  Position = 0;
  selectionStart = 0;
  selectionEnd = 0;
  logger = spdlog::get("Logger");
}

void DOC::SetDoc(QQuickTextDocument *mDoc) noexcept {
  if (Doc == mDoc) {
    return;
  }
  Doc = mDoc;
}
QTextCursor DOC::GetCursor() noexcept {
  QTextCursor cursor = QTextCursor(Doc->textDocument());
  if (selectionStart != selectionEnd) {
    cursor.setPosition(selectionStart);
    cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
  } else {
    cursor.setPosition(Position);
  }
  return cursor;
}

void EDITOR::load(const QUrl &FileUrl) noexcept {
  QQmlEngine *engine = qmlEngine(this);
  if (!engine) {
    qWarning() << "load() called before DocumentHandler has QQmlEngine";
    return;
  }
  const QUrl path =
      engine->interceptUrl(FileUrl, QQmlAbstractUrlInterceptor::UrlString);
  const QString fileName = QQmlFile::urlToLocalFileOrQrc(path);
  if (QFile::exists(fileName)) {
    QMimeType mime = QMimeDatabase().mimeTypeForFile(fileName);
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
      QByteArray data = file.readAll();
      auto mDoc = GetDoc();
      if (mDoc->textDocument()) {
        mDoc->textDocument()->setBaseUrl(path.adjusted(QUrl::RemoveFilename));
        mDoc->textDocument()->setModified(false);
        if (mime.inherits("text/markdown")) {
          emit loaded(QString::fromUtf8(data), Qt::MarkdownText);
        } else {
          auto encoding = QStringConverter::encodingForHtml(data);
          if (encoding) {
            QStringDecoder decoder(*encoding);
            emit loaded(decoder(data), Qt::AutoText);
          } else {
            // fall back to utf8
            emit loaded(QString::fromUtf8(data), Qt::AutoText);
          }
        }
      }
    }
  }
  m_fileUrl = FileUrl;
  emit fileUrlChanged();
}

void EDITOR::Work() noexcept {
  struct Start {};
}

void EDITOR::Send() noexcept {
  QEventLoop loop;
  connect(this, &EDITOR::done, &loop, &QEventLoop::quit);
  std::jthread SendThread([&]() {
    std::string context = GetDoc()->textDocument()->toRawText().toStdString();
    std::size_t pos = context.find(';');
    std::string url = context.substr(0, pos);
    std::string data = context.substr(pos + 1, context.size());
    HTTP mhttp(url);
    if (mhttp.Send(data) && mhttp.Get()) {
      QTextCursor cursor = QTextCursor(GetDoc()->textDocument());
      cursor.movePosition(QTextCursor::End);
      SetPosition(cursor.position());
      logger->info("ResponseHearder: " + mhttp.RawResponse);
      emit sendLog(mhttp.RawResponse.data(), LOGLEVEL::Info);
    }
    emit done();
  });

  SendThread.detach();
  loop.exec();

  disconnect(this, &EDITOR::done, &loop, &QEventLoop::quit);
  return;
}
void EDITOR::Write(std::string_view text) noexcept {
  QTextCursor Cursor = QTextCursor(GetDoc()->textDocument());
  Cursor.movePosition(QTextCursor::End);
  SetPosition(Cursor.position());
  Cursor.insertText(text.data());
  logger->info("写入成功");
}
std::string EDITOR::Read() noexcept { return ""; }

void LOGGER::Write(std::string_view text) noexcept {
  QTextCursor Cursor = QTextCursor(GetDoc()->textDocument());
  Cursor.movePosition(QTextCursor::End);
  SetPosition(Cursor.position());
  Cursor.insertText(text.data());
  auto err_logger = spdlog::stderr_color_mt("stderr");
}
std::string LOGGER::Read() noexcept { return ""; }

void LOGGER::ReceiveLog(const QString &log, LOGLEVEL Loglevel) noexcept {
  QTextCursor Cursor = QTextCursor(GetDoc()->textDocument());
  Cursor.movePosition(QTextCursor::End);
  SetPosition(Cursor.position());
  std::stringstream ss;
  auto time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  ss << std::put_time(std::localtime(&time), "%F %T");
  Cursor.insertText("[");
  Cursor.insertText(std::move(ss.str().data()));
  Cursor.insertText("][");

  QTextCharFormat Format;
  switch (Loglevel) {
    case LOGLEVEL::Trace: {
      Format.setForeground(QBrush(Qt::darkGray));
      Cursor.insertText("Trace", Format);
      break;
    }
    case LOGLEVEL::Debug: {
      Format.setForeground(QBrush(Qt::black));
      Cursor.insertText("Debug", Format);
      break;
    }
    case LOGLEVEL::Info: {
      Format.setForeground(QBrush(Qt::green));
      Cursor.insertText("Info", Format);
      break;
    }
    case LOGLEVEL::Warn: {
      Format.setForeground(QBrush(Qt::yellow));
      Cursor.insertText("Warn", Format);
      break;
    }
    case LOGLEVEL::Error: {
      Format.setForeground(QBrush(Qt::red));
      Cursor.insertText("Error", Format);
      break;
    }
    case LOGLEVEL::Critical: {
      Format.setForeground(QBrush(Qt::darkRed));
      Cursor.insertText("Critical", Format);
      break;
    }
    default: {
      Format.setForeground(QBrush(Qt::black));
      Cursor.insertText("Unknow", Format);
    }
  }
  Format.setForeground(QBrush(Qt::black));
  Cursor.insertText("]:", Format);
  Cursor.insertText(log);
  Cursor.insertText("\n");
}
