#ifndef EXTENSIONSYSTEM_FILE
#define EXTENSIONSYSTEM_FILE

#include "Header.hpp"
#include "PlainTextHandler.h"

namespace ExtensionSystem {
// sender,reveiver,info
using MSG = std::tuple<QString, QString, QJsonObject>;

enum class StopFlag { SynchronousStop, AsynchronousStop };

enum class TYPE { Required, Optional, Test };

enum class STATE {
  Invalid,
  Read,
  Resolved,
  Loaded,
  Initialized,
  Running,
  Stopped,
  Deleted
};

// name,version,type
using DEPENDENCY = std::tuple<QString, QString, TYPE>;

class SPEC;
class EXTENSIONSYSTEM_EXPORT IPlugin : public QObject {
  Q_OBJECT

 private:
  std::shared_ptr<SPEC> Spec;

 public:
  IPlugin() = default;
  virtual ~IPlugin() = 0;
  virtual bool Initialize(
      std::vector<std::string> Args = std::vector<std::string>()) noexcept = 0;
  virtual bool Run() noexcept = 0;
  virtual StopFlag Stop() noexcept = 0;
  std::shared_ptr<SPEC> GetSpec() noexcept { return Spec; };
  // 用于插件和主框架通信
  virtual void Receive(MSG Msg) noexcept = 0;

 signals:
  void asynchronousStopFinished();
  void sendSignal(MSG Msg);
  // 向日志控件发送日志
  void sendLog(QString Log, LOGLEVEL Loglevel);
};

// 插件模板，基类。用于描述插件信息和状态，但不执行加载。插件核心类，该类实现插件的所有属性
class EXTENSIONSYSTEM_EXPORT SPEC {
 private:
  struct DependencyHash {
    std::size_t operator()(const DEPENDENCY& mDependency) const noexcept {
      return std::hash<QString>{}(std::get<0>(mDependency));
    }
  };

  struct DependencyEqual {
    bool operator()(const DEPENDENCY& lhs,
                    const DEPENDENCY& rhs) const noexcept {
      if (lhs == rhs) {
        return true;
      }
      return false;
    }
  };

  STATE State;
  std::shared_ptr<IPlugin> PluginObject;
  std::vector<DEPENDENCY> Dependencies;
  std::unordered_map<DEPENDENCY, std::shared_ptr<SPEC>, DependencyHash,
                     DependencyEqual>
      DependencySpecs;

  std::vector<std::string> Arg;
  QPluginLoader Loader;
  QString Name;
  QString Version;
  TYPE Type;
  QString IID;
  QString Category;

 public:
  using DependencySpecsTpye =
      std::unordered_map<DEPENDENCY, std::shared_ptr<SPEC>, DependencyHash,
                         DependencyEqual>;
  SPEC() = default;
  ~SPEC() = default;

  // 读取插件信息，获取插件依赖关系
  bool Read(const QString& filePath) noexcept;
  bool SetState(STATE mState) noexcept {
    State = mState;
    return true;
  };
  STATE GetState() noexcept { return State; };
  QString GetName() noexcept { return Name; };
  QString GetIID() noexcept { return IID; };
  QString GetVersion() noexcept { return Version; };
  TYPE GetType() noexcept { return Type; };
  QString GetCategory() noexcept { return Category; };
  std::shared_ptr<IPlugin> GetPlugin() noexcept { return PluginObject; };

  void SetName(const QString& mName) noexcept { Name = mName; };
  void SetIID(const QString& mIID) noexcept { IID = mIID; };
  void SetVersion(const QString& mVersion) noexcept { Version = mVersion; };
  void SetType(TYPE mType) noexcept { Type = mType; };
  void SetCategory(const QString& mCategory) noexcept { Category = mCategory; };

  DependencySpecsTpye GetDependencySpecs() noexcept { return DependencySpecs; };
  std::vector<DEPENDENCY> GetDependency() noexcept { return Dependencies; };
  void DependencySpecsClear() noexcept { DependencySpecs.clear(); };
  void DependencySpecsInsert(DEPENDENCY mDependency,
                             std::shared_ptr<SPEC> mSpec) noexcept {
    DependencySpecs[mDependency] = mSpec;
  };
  bool IsValidVersion(const QString& mVersion) noexcept { return true; };

  bool Load() noexcept;
  bool Initialize() noexcept;
  bool Kill() noexcept;
  bool Run() noexcept;
  StopFlag Stop() noexcept;
};

namespace PluginManager {

class EXTENSIONSYSTEM_EXPORT PLUGIN_MANAGE : public QObject {
  Q_OBJECT
 private:
  PLUGIN_MANAGE(){};
  ~PLUGIN_MANAGE(){};
  PLUGIN_MANAGE(const PLUGIN_MANAGE&) = delete;
  PLUGIN_MANAGE& operator=(const PLUGIN_MANAGE&) = delete;
  std::vector<std::shared_ptr<QObject>> Objects;
  std::vector<std::shared_ptr<SPEC>> PluginSpecs;
  std::unordered_map<QString, std::vector<std::shared_ptr<SPEC>>>
      PluginCategory;

 public:
  static PLUGIN_MANAGE& GetInstance() {
    static PLUGIN_MANAGE instance;
    return instance;
  }

  bool AddObject(std::shared_ptr<IPlugin> mObject) noexcept;
  bool RemoveObject(std::shared_ptr<IPlugin> mObject) noexcept;
  // 如果有多个给定类型，则返回任意一个
  template <typename T>
  std::shared_ptr<T> GetObject() noexcept;
  std::shared_ptr<QObject> GetObject(const QString& mName) noexcept;
  std::vector<std::shared_ptr<QObject>> GetAllObject() noexcept {
    return Objects;
  }

  bool Read() noexcept;
  bool Load() noexcept;
  bool UnLoad(const QString& Path) noexcept;
  bool UnLoad() noexcept;
  bool Resolve() noexcept;
  bool EnterTargetState(const std::shared_ptr<SPEC>,
                        STATE TargetState) noexcept;
  std::shared_ptr<SPEC> Find(const DEPENDENCY& Dependency) noexcept;
  void CategoryClear() noexcept { PluginCategory.clear(); };
  void CategoryInsert(QString& mCategory,
                      std::vector<std::shared_ptr<SPEC>> mSpecs) noexcept {
    PluginCategory[mCategory] = mSpecs;
  };

  // 循环依赖检查，如果CircularityCheckQueue中出现两个相同的插件实例，说明有循环依赖
  // Queue中保存已经经过循环检查的插件实例
  bool CircularityCheck(
      std::shared_ptr<SPEC> Spec, std::vector<std::shared_ptr<SPEC>>& Queue,
      std::vector<std::shared_ptr<SPEC>>& CircularityCheckQueue) noexcept;
 public slots:
  // 用于插件和主框架通信
  void Receive(MSG Msg) noexcept;
 signals:
  void objectChange(std::shared_ptr<QObject> mObject);
  void pluginsChanged();
  void sendSignal(MSG Msg);
  // 向日志控件发送日志
  void sendLog(QString Log, LOGLEVEL Loglevel);
};
};  // namespace PluginManager
};  // namespace ExtensionSystem

QT_BEGIN_NAMESPACE

#define EchoInterface_iid "org.qt-project.Qt.Examples.IPlugin"

Q_DECLARE_INTERFACE(ExtensionSystem::IPlugin, EchoInterface_iid)
QT_END_NAMESPACE
#endif
