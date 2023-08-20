
#include "ExtensionSystem.h"

#include "moc_ExtensionSystem.cpp"
namespace ExtensionSystem {
std::mutex ObjectMutex;
QString PLUGIN_METADATA = "MetaData";
QString PLUGIN_NAME = "Name";
QString PLUGIN_VERSION = "Version";
QString PLUGIN_REQUIRED = "Required";
QString SPEC_STATE = "State";
QString ARGUMENTS = "Arguments";
QString ARGUMENT_NAME = "Name";
QString ARGUMENT_PARAMETER = "Parameter";
QString ARGUMENT_DESCRIPTION = "Description";
QString DEPENDENCIES = "Dependencies";
QString DEPENDENCY_NAME = "Name";
QString DEPENDENCY_VERSION = "Version";
QString DEPENDENCY_TYPE = "Type";
QString DEPENDENCY_TYPE_SOFT = "Optional";
QString DEPENDENCY_TYPE_HARD = "Required";
QString DEPENDENCY_TYPE_TEST = "Test";
QString CATEGORY = "Category";
std::unordered_map<ExtensionSystem::STATE, QString> mMap = {
    {ExtensionSystem::STATE::Invalid, "Invalid"},
    {ExtensionSystem::STATE::Read, "Read"},
    {ExtensionSystem::STATE::Resolved, "Resolved"},
    {ExtensionSystem::STATE::Loaded, "Loaded"},
    {ExtensionSystem::STATE::Initialized, "Initialized"},
    {ExtensionSystem::STATE::Running, "Running"},
    {ExtensionSystem::STATE::Stopped, "Stopped"},
    {ExtensionSystem::STATE::Deleted, "Deleted"}};
bool SPEC::Load() noexcept {
  if (STATE::Resolved != State && STATE::Loaded != State) {
    return false;
  }
  if (STATE::Loaded == State) {
    return true;
  }
  PluginObject.reset(qobject_cast<IPlugin*>(Loader.instance()));
  if (nullptr == PluginObject) {
    Loader.unload();
    return false;
  }
  State = STATE::Loaded;
  return true;
};

bool SPEC::Initialize() noexcept {
  if (STATE::Loaded != State && STATE::Initialized != State) {
    return false;
  }
  if (STATE::Initialized == State) {
    return true;
  }
  if (nullptr == PluginObject) {
    return false;
  }
  if (!PluginObject->Initialize(Arg)) {
    return false;
  }
  State = STATE::Initialized;
  return true;
}

bool SPEC::Kill() noexcept {
  if (nullptr == PluginObject) {
    return true;
  }
  // 切断Spec和插件对象的双向指针
  PluginObject->GetSpec().reset();
  PluginObject.reset();
  State = STATE::Deleted;
  return true;
}

bool SPEC::Unload() noexcept { return Loader.unload(); }

bool SPEC::Run() noexcept {
  if (STATE::Initialized != State && STATE::Running != State) {
    return false;
  }
  if (STATE::Running == State) {
    return true;
  }
  if (nullptr == PluginObject) {
    return false;
  }
  if (!PluginObject->Run()) {
    return false;
  }
  State = STATE::Running;
  return true;
}
StopFlag SPEC::Stop() noexcept {
  if (nullptr == PluginObject) {
    return StopFlag::SynchronousStop;
  }
  State = STATE::Stopped;
  return PluginObject->Stop();
}

bool SPEC::Read(const QString& filePath) noexcept {
  State = STATE::Invalid;
  Loader.setFileName(filePath);
  if (Loader.fileName().isEmpty()) {
    return false;
  }
  MetaData;
  try {
    MetaData = Loader.metaData().value(PLUGIN_METADATA).toObject();
  } catch (...) {
    return false;
  }

  QJsonValue value;
  value = MetaData.value(PLUGIN_NAME);
  if (value.isUndefined() || !value.isString()) {
    return false;
  }
  Name = value.toString();
  value = MetaData.value(PLUGIN_VERSION);
  if (value.isUndefined() || !value.isString() ||
      !IsValidVersion(value.toString())) {
    return false;
  }
  Version = value.toString();
  value = MetaData.value(CATEGORY);
  if (value.isUndefined() || !value.isString()) {
    return false;
  }
  Category = value.toString();
  value = MetaData.value(DEPENDENCIES);
  if (value.isUndefined() || !value.isArray()) {
    return false;
  }

  for (const QJsonValue& v : value.toArray()) {
    if (!v.isObject()) {
      return false;
    }
    QJsonObject dependencyObject = v.toObject();
    DEPENDENCY dep;
    QJsonValue Name = dependencyObject.value(DEPENDENCY_NAME);
    QJsonValue Version = dependencyObject.value(DEPENDENCY_VERSION);
    QJsonValue Type = dependencyObject.value(DEPENDENCY_TYPE);

    if (Name.isUndefined() || !Name.isString()) {
      return false;
    }
    if ((Version.isUndefined() || !Version.isString() ||
         !IsValidVersion(Version.toString()))) {
      return false;
    }
    if ((Type.isUndefined() || !Type.isString())) {
      Dependencies.push_back(
          std::make_tuple(Name.toString(), Version.toString(), TYPE::Optional));
    } else {
      if (DEPENDENCY_TYPE_HARD == Type.toString()) {
        Dependencies.push_back(std::make_tuple(
            Name.toString(), Version.toString(), TYPE::Required));
      } else if (DEPENDENCY_TYPE_SOFT == Type.toString()) {
        Dependencies.push_back(std::make_tuple(
            Name.toString(), Version.toString(), TYPE::Optional));

      } else {
        Dependencies.push_back(
            std::make_tuple(Name.toString(), Version.toString(), TYPE::Test));
      }
    }
  }
  // TODO 还需要解析参数
  State = STATE::Read;
  return true;
}

namespace PluginManager {

PLUGIN_MANAGE::~PLUGIN_MANAGE() {
  for (auto Spec : PluginSpecs) {
    EnterTargetState(Spec, STATE::Stopped);
    EnterTargetState(Spec, STATE::Deleted);
    Spec->Unload();
  }
  Objects.clear();
  PluginSpecs.clear();
  PluginCategory.clear();
  AsyncPlugins.clear();
}

std::shared_ptr<QObject> PLUGIN_MANAGE::GetObject(
    std::string_view mName) noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  for (auto Spec : PluginSpecs) {
    if (Spec->GetName() == mName.data()) {
      return Spec->GetPlugin();
    }
  }
  return nullptr;
}
template <typename T>
std::shared_ptr<T> PLUGIN_MANAGE::GetObject() noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  for (auto mObject : Objects) {
    // TODO 如果一个插件类有多个实例，会报错
    if (std::is_same<T, decltype(mObject.get())>::value) {
      return mObject;
    }
  }
  return nullptr;
}

bool PLUGIN_MANAGE::AddObject(std::shared_ptr<IPlugin> mObject) noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  if (nullptr == mObject) {
    return false;
  }
  if ((std::end(Objects) == Objects.find(mObject))) {
    Objects.insert(mObject);
  }
  return false;
}

bool PLUGIN_MANAGE::RemoveObject(std::shared_ptr<IPlugin> mObject) noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  if (nullptr != mObject) {
    auto it = std::find(begin(Objects), end(Objects), mObject);
    if (std::end(Objects) != it) {
      Objects.erase(it);
      // emit objectChange(mObject);
      return true;
    }
  }
  // emit objectChange(mObject);
  return false;
}

bool PLUGIN_MANAGE::Load() noexcept {
  // 首先读取插件信息，求解插件之间的依赖，并建立加载顺序
  PluginSpecs.clear();
  PluginCategory.clear();
  QDir PluginsDir(QGuiApplication::applicationDirPath());
  std::string sa = PluginsDir.absolutePath().toStdString();
  if ("windows" == PLATFORM) {
    if (PluginsDir.dirName().toLower() == "debug" ||
        PluginsDir.dirName().toLower() == "release") {
      PluginsDir.cdUp();
    }
  } else if ("apple" == PLATFORM) {
    if (PluginsDir.dirName() == "MacOS") {
      PluginsDir.cdUp();
      PluginsDir.cdUp();
      PluginsDir.cdUp();
    }
  }
  PluginsDir.cd("plugins");
  const QStringList Entries = PluginsDir.entryList(QDir::Files);
  for (const QString& FileName : Entries) {
    auto Spec = std::make_shared<SPEC>();
    if (!Spec->Read(PluginsDir.absoluteFilePath(FileName))) {
      continue;
    }
    PluginCategory[Spec->GetCategory()].push_back(Spec);
    PluginSpecs.insert(Spec);
  }
  if (!Resolve()) {
    return false;
  }
  // 循环依赖检查
  // Queue中元素的顺序就是加载的顺序
  std::vector<std::shared_ptr<SPEC>> Queue;
  for (auto Spec : PluginSpecs) {
    std::unordered_set<std::shared_ptr<SPEC>> Circularity;
    if (!CircularityCheck(Spec, Queue, Circularity)) {
      // TODO 弹窗提示有插件有循环依赖
      std::cout << "有循环依赖" << std::endl;
    }
  }

  // 加载插件时，判断依赖项的状态是不是已加载
  for (const auto Spec : Queue) {
    if (!EnterTargetState(Spec, STATE::Loaded)) {
      std::cout << "加载插件失败" << std::endl;
    }
  }
  // 初始化插件：其实就是给插件传参
  for (const auto Spec : Queue) {
    if (!EnterTargetState(Spec, STATE::Initialized)) {
      std::cout << "初始化插件失败" << std::endl;
    }
  }
  for (const auto Spec : Queue) {
    if (!EnterTargetState(Spec, STATE::Running)) {
      Spec->Kill();
      emit sendLog(Spec->GetName() + "插件运行失败", LOGLEVEL::Error);
      return false;
    }
  }
  return true;
}

bool PLUGIN_MANAGE::Resolve() noexcept {
  for (auto iter : PluginSpecs) {
    if (STATE::Resolved == iter->GetState()) {
      iter->SetState(STATE::Read);
    }
    if (STATE::Read != iter->GetState()) {
      return false;
    }
    iter->DependencySpecsClear();
    auto Dependencies = iter->GetDependency();
    for (DEPENDENCY Dependency : Dependencies) {
      // 根据依赖项的名字找到依赖项实例对应的指针
      std::shared_ptr<SPEC> Spec = PLUGIN_MANAGE::Find(Dependency);
      if (!Spec) {
        if (TYPE::Required == std::get<2>(Dependency)) {
          return false;
        } else {
          continue;
        }
      }
      iter->DependencySpecsInsert(Dependency, Spec);
    }
    iter->SetState(STATE::Resolved);
  }
  return true;
};

std::shared_ptr<SPEC> PLUGIN_MANAGE::Find(
    const DEPENDENCY& Dependency) noexcept {
  for (std::shared_ptr<SPEC> Spec : PluginSpecs) {
    if (Spec->GetName() == std::get<0>(Dependency) &&
        Spec->GetVersion() == std::get<1>(Dependency) &&
        Spec->GetType() == std::get<2>(Dependency)) {
      return Spec;
    }
  }
  return nullptr;
}

bool PLUGIN_MANAGE::CircularityCheck(
    std::shared_ptr<SPEC> Spec, std::vector<std::shared_ptr<SPEC>>& Queue,
    std::unordered_set<std::shared_ptr<SPEC>>& CircularityCheckQueue) noexcept {
  if (Queue.end() != std::find(begin(Queue), end(Queue), Spec)) {
    // 已经在队列中就直接返回
    return true;
  }
  if (CircularityCheckQueue.end() != CircularityCheckQueue.find(Spec)) {
    // 检测到循环依赖
    return false;
  }
  CircularityCheckQueue.insert(Spec);
  if (Spec->GetState() == STATE::Read || Spec->GetState() == STATE::Invalid) {
    return false;
  }
  auto DependencySpecs = Spec->GetDependencySpecs();
  for (const auto iter : DependencySpecs) {
    if (std::get<2>(iter.first) == TYPE::Test) {
      continue;
    }
    if (!CircularityCheck(iter.second, Queue, CircularityCheckQueue)) {
      return false;
    }
  }
  Queue.push_back(Spec);
  return true;
};

bool PLUGIN_MANAGE::EnterTargetState(const std::shared_ptr<SPEC> Spec,
                                     STATE TargetState) noexcept {
  switch (TargetState) {
    case STATE::Loaded: {
      if (Spec->GetState() != STATE::Resolved) {
        emit sendLog(Spec->GetName() + "插件未处于Resolved状态，请检查插件状态",
                     LOGLEVEL::Warn);
        return false;
      }
      // 检查依赖项是否已经加载
      auto DependencySpecs = Spec->GetDependencySpecs();

      for (const auto iter : DependencySpecs) {
        if (std::get<2>(iter.first) != TYPE::Required) {
          continue;
        }
        if (iter.second->GetState() != TargetState) {
          emit sendLog(Spec->GetName() + "插件加载失败。依赖项" +
                           iter.second->GetName() +
                           "未处于加载状态，请检查插件状态",
                       LOGLEVEL::Warn);
          return false;
        }
      }
      if (!Spec->Load()) {
        emit sendLog(Spec->GetName() + "插件实例化失败", LOGLEVEL::Warn);
        return false;
      };
      // 内存池中已经有插件对象实例，直接返回true
      if (std::end(Objects) != Objects.find(Spec->GetPlugin())) {
        // 建立Spec与插件对象实例的双向指针
        Spec->GetPlugin()->SetSpec(Spec);
        emit sendLog(Spec->GetName() + "插件已经有实例", LOGLEVEL::Info);
        return true;
      }

      if (!AddObject(Spec->GetPlugin())) {
        emit sendLog(Spec->GetName() + "插件加载失败", LOGLEVEL::Warn);
        return false;
      }
      // 第一次加载插件时，插件sendSignal与PM的receive绑定
      QObject::connect(Spec->GetPlugin().get(), &IPlugin::sendSignal, this,
                       &PLUGIN_MANAGE::Receive);
      // 第一次加载插件时，插件停止信号与PM的停止槽绑定
      QObject::connect(Spec->GetPlugin().get(),
                       &IPlugin::asynchronousStopFinished, this,
                       &PLUGIN_MANAGE::AsyncStopFinished);
      // 将插件的sendlog与日志对象的ReceiveLog绑定
      QObject::connect(Spec->GetPlugin().get(), &IPlugin::sendLog, Logger.get(),
                       &LOGGER::ReceiveLog);
      return true;
    }
    case STATE::Initialized: {
      if (Spec->GetState() != STATE::Loaded) {
        return false;
      }
      // 检查依赖项是否已经加载
      auto DependencySpecs = Spec->GetDependencySpecs();
      for (const auto iter : DependencySpecs) {
        if (std::get<2>(iter.first) != TYPE::Required) {
          continue;
        }
        if (iter.second->GetState() != TargetState) {
          return false;
        }
      }
      return Spec->Initialize();
    }
    case STATE::Running: {
      if (Spec->GetState() != STATE::Initialized) {
        return false;
      }
      return Spec->Run();
    }
    // 停止插件运行
    case STATE::Stopped: {
      auto mState = Spec->GetState();
      if (STATE::Loaded != mState && STATE::Initialized != mState &&
          STATE::Running != mState) {
        emit sendLog(Spec->GetName() +
                         "插件未处于加载、初始化或者运行状态，请检查插件状态",
                     LOGLEVEL::Warn);
        return false;
      }
      if (Spec->Stop() == ExtensionSystem::StopFlag::AsynchronousStop) {
        AsyncPlugins.insert(Spec);
      }
      return true;
    }
    // 卸载插件
    case STATE::Deleted: {
      if (STATE::Stopped != Spec->GetState()) {
        emit sendLog(Spec->GetName() + "插件未处于停止状态，请检查插件状态",
                     LOGLEVEL::Warn);
        return false;
      }
      if (!Spec->Kill()) {
        emit sendLog(Spec->GetName() + "插件卸载失败", LOGLEVEL::Warn);
        return false;
      }
      emit sendLog(Spec->GetName() + "插件已经卸载", LOGLEVEL::Warn);
      return true;
    }
    default:
      break;
  }
  return true;
};

void PLUGIN_MANAGE::Receive(MSG Msg) noexcept {
  QString receiver = std::get<1>(Msg);
  QString log;

  if ("Window" == receiver) {
    log = "测试结束";
    emit sendLog(log, LOGLEVEL::Info);
    return;
  }
  for (auto Spec : PluginSpecs) {
    if (Spec->GetName() == receiver) {
      auto mPlugin = Spec->GetPlugin();
      if (nullptr == mPlugin) {
        log = "转发消息时目标插件" + receiver + "没有实例";
        emit sendLog(log, LOGLEVEL::Error);
        return;
      }
      log = "从" + std::get<0>(Msg) + "向" + receiver + "转发消息成功";
      emit sendLog(log, LOGLEVEL::Info);
      mPlugin->Receive(Msg);
      return;
    }
  }
  log = "转发消息时没有找到目标插件" + receiver;
  emit sendLog(log, LOGLEVEL::Error);
  return;
}

void PLUGIN_MANAGE::AsyncStopFinished(std::shared_ptr<SPEC> mSpec) noexcept {
  AsyncPlugins.erase(mSpec);
  emit sendLog("插件" + mSpec->GetName() + "已经停止", LOGLEVEL::Warn);
}

std::vector<QJsonObject> PLUGIN_MANAGE::GetInfo() noexcept {
  std::vector<QJsonObject> Info;
  for (auto Spec : PluginSpecs) {
    QJsonObject TempInfo = Spec->GetMetaData();
    TempInfo.insert(SPEC_STATE, mMap.find(Spec->GetState())->second);
    Info.push_back(TempInfo);
  }

  return std::move(Info);
}

bool PLUGIN_MANAGE::UnLoad(const QString& Path) noexcept { return true; }
bool PLUGIN_MANAGE::UnLoad() noexcept { return true; }
};  // namespace PluginManager
};  // namespace ExtensionSystem
