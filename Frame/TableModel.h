#ifndef TABLEMODEL_FILE
#define TABLEMODEL_FILE

#include "Header.hpp"

// TODO 如果显示列表标签
namespace DataModel {

class TABLEMODEL : public QAbstractTableModel {
  Q_OBJECT
  QML_ELEMENT
  QML_ADDED_IN_MINOR_VERSION(1)

 private:
  std::vector<QJsonObject> Info;
  std::size_t RowNum;
  std::size_t ColNum;

 public:
  TABLEMODEL() {}
  ~TABLEMODEL() {}

  int rowCount(const QModelIndex &Parent = QModelIndex()) const override {
    Q_UNUSED(Parent);
    return Info.size();
  }

  int columnCount(const QModelIndex &Parent = QModelIndex()) const override {
    Q_UNUSED(Parent);
    return 5;
  }

  QVariant data(const QModelIndex &Index, int Role) const override;

  QHash<int, QByteArray> roleNames() const override {
    return {{Qt::DisplayRole, "display"}};
  }
  // bool SetHeader() noexcept;

 public slots:
  void UpModelData() noexcept;
};
}  // namespace DataModel

#endif