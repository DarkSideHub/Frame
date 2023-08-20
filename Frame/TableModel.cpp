#include "TableModel.h"

#include "ExtensionSystem.h"
#include "moc_TableModel.cpp"

namespace DataModel {

QString PLUGIN_METADATA;
QString PLUGIN_NAME;
QString PLUGIN_VERSION;
QString PLUGIN_REQUIRED;
QString SPEC_STATE;
QString ARGUMENTS;
QString ARGUMENT_NAME;
QString ARGUMENT_PARAMETER;
QString ARGUMENT_DESCRIPTION;
QString DEPENDENCIES;
QString DEPENDENCY_NAME;
QString DEPENDENCY_VERSION;
QString DEPENDENCY_TYPE;
QString DEPENDENCY_TYPE_SOFT;
QString DEPENDENCY_TYPE_HARD;
QString DEPENDENCY_TYPE_TEST;
QString CATEGORY;

void TABLEMODEL::UpModelData() noexcept {
  QAbstractItemModel::beginResetModel();
  Info.clear();
  auto &mPluginManage =
      ExtensionSystem::PluginManager::PLUGIN_MANAGE::GetInstance();
  Info = std::move(mPluginManage.GetInfo());
  QAbstractItemModel::endResetModel();
  // SetHeader();
}

// bool TABLEMODEL::SetHeader() noexcept {
//   if (Info.empty()) {
//     return false;
//   }
//   int num = 0;
//   for (auto Element : Info.begin()->keys()) {
//     std::cout << Element.toStdString() << std::endl;
//     if (!QAbstractItemModel::setHeaderData(num++, Qt::Horizontal, Element)) {
//       return false;
//     }
//   }
//   emit headerDataChanged(Qt::Horizontal, 0, num);
//   return true;
// }

QVariant TABLEMODEL::data(const QModelIndex &Index, int Role) const {
  std::cout << "获取数据" << std::endl;
  int mRow = Index.row();
  int mColumn = Index.column();
  if (mRow > Info.size()) {
    return QVariant();
  }

  switch (Role) {
    case Qt::DisplayRole: {
      switch (mColumn) {
        case 0: {
          QJsonValue mValue = Info[mRow].value("Name");
          if (mValue.isUndefined() || !mValue.isString()) {
            return QVariant();
          }
          std::cout << "行:" << mRow << "列" << mColumn << ":"
                    << mValue.toString().toStdString() << std::endl;
          return mValue.toString();
        }
        case 1: {
          QJsonValue mValue = Info[mRow].value("State");
          if (mValue.isUndefined() || !mValue.isString()) {
            return QVariant();
          }
          std::cout << "行:" << mRow << "列" << mColumn << ":"
                    << mValue.toString().toStdString() << std::endl;
          return mValue.toString();
        }
        case 2: {
          QJsonValue mValue = Info[mRow].value("Version");
          if (mValue.isUndefined() || !mValue.isString()) {
            return QVariant();
          }
          std::cout << "行:" << mRow << "列" << mColumn << ":"
                    << mValue.toString().toStdString() << std::endl;
          return mValue.toString();
        }
        case 3: {
          QJsonValue mValue = Info[mRow].value("Category");
          if (mValue.isUndefined() || !mValue.isString()) {
            return QVariant();
          }
          std::cout << "行:" << mRow << "列" << mColumn << ":"
                    << mValue.toString().toStdString() << std::endl;
          return mValue.toString();
        }
        case 4: {
          QJsonValue mValue = Info[mRow].value("Description");
          if (mValue.isUndefined() || !mValue.isString()) {
            return QVariant();
          }
          std::cout << "行:" << mRow << "列" << mColumn << ":"
                    << mValue.toString().toStdString() << std::endl;
          return mValue.toString();
        }
        default:
          break;
      }
    }
    default:
      break;
  }
  return QVariant();
}

}  // namespace DataModel
