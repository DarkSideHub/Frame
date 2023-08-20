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
  // 因为会发生指针的传递，所以用shared_ptr
  std::shared_ptr<SPEC> Spec;

 public:
  IPlugin() = default;
  virtual ~IPlugin() = 0;
  virtual bool Initialize(
      std::vector<std::string> Args = std::vector<std::string>()) noexcept = 0;
  virtual bool Run() noexcept = 0;
  virtual StopFlag Stop() noexcept { return StopFlag::SynchronousStop; };
  std::shared_ptr<SPEC> GetSpec() noexcept { return Spec; };
  void SetSpec(std::shared_ptr<SPEC> mSpec) noexcept {
    Spec.reset();
    Spec = mSpec;
  };

  // 用于插件和主框架通信
  virtual void Receive(MSG Msg) noexcept = 0;

 signals:
  void asynchronousStopFinished(std::shared_ptr<SPEC> mSpec);
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
  QJsonObject MetaData;

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
  QString GetDescription() noexcept { return ""; };
  QJsonObject GetMetaData() noexcept { return MetaData; };

  std::shared_ptr<IPlugin> GetPlugin() noexcept { return PluginObject; };

  void SetIID(const QString& mIID) noexcept { IID = mIID; };
  void SetType(TYPE mType) noexcept { Type = mType; };
  void SetPlugin(std::shared_ptr<IPlugin> mPlugin) noexcept {
    PluginObject.reset();
    PluginObject = mPlugin;
  };

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
  bool Unload() noexcept;
  bool Run() noexcept;
  StopFlag Stop() noexcept;
};

namespace PluginManager {

class EXTENSIONSYSTEM_EXPORT PLUGIN_MANAGE : public QObject {
  Q_OBJECT
 private:
  PLUGIN_MANAGE(){};
  ~PLUGIN_MANAGE();
  PLUGIN_MANAGE(const PLUGIN_MANAGE&) = delete;
  PLUGIN_MANAGE& operator=(const PLUGIN_MANAGE&) = delete;
  //  因为插件实例只有一个，所以使用std::unordered_set存储插件信息，
  std::unordered_set<std::shared_ptr<QObject>> Objects;
  std::unordered_set<std::shared_ptr<SPEC>> PluginSpecs;
  std::unordered_map<QString, std::vector<std::shared_ptr<SPEC>>>
      PluginCategory;
  std::unordered_set<std::shared_ptr<SPEC>> AsyncPlugins;
  std::shared_ptr<LOGGER> Logger;
  // 如果有多个给定类型，则返回任意一个
  template <typename T>
  std::shared_ptr<T> GetObject() noexcept;
  std::shared_ptr<QObject> GetObject(std::string_view mName) noexcept;
  std::unordered_set<std::shared_ptr<QObject>> GetAllObject() noexcept {
    return Objects;
  }

  bool AddObject(std::shared_ptr<IPlugin> mObject) noexcept;
  bool RemoveObject(std::shared_ptr<IPlugin> mObject) noexcept;
  bool Read() noexcept;
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
  bool CircularityCheck(std::shared_ptr<SPEC> Spec,
                        std::vector<std::shared_ptr<SPEC>>& Queue,
                        std::unordered_set<std::shared_ptr<SPEC>>&
                            CircularityCheckQueue) noexcept;

 public:
  static PLUGIN_MANAGE& GetInstance() {
    static PLUGIN_MANAGE instance;
    return instance;
  }
  void SetLogger(std::shared_ptr<LOGGER> mLogger) noexcept {
    Logger.reset();
    Logger = mLogger;
  };

  // 外界只能加载和卸载插件
  bool Load() noexcept;
  bool UnLoad(const QString& Path) noexcept;
  bool UnLoad() noexcept;
  // 获取插件信息
  std::vector<QJsonObject> GetInfo() noexcept;

 public slots:
  // 用于插件和主框架通信
  void Receive(MSG Msg) noexcept;
  void AsyncStopFinished(std::shared_ptr<SPEC> mSpec) noexcept;
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
